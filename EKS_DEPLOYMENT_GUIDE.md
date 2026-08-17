# EKS Deployment Guide

## Prerequisites

- EKS cluster created
- `kubectl` configured to connect to EKS
- `helm` installed locally
- AWS CLI configured with credentials

---

## Step 1: Create AWS Load Balancer Controller IAM Policy

```bash
# Download the policy document
curl -o iam_policy.json https://raw.githubusercontent.com/kubernetes-sigs/aws-load-balancer-controller/v2.6.0/docs/install/iam_policy.json

# Create IAM policy
aws iam create-policy \
  --policy-name AWSLoadBalancerControllerIAMPolicy \
  --policy-document file://iam_policy.json
```

---

## Step 2: Create IAM Role and Service Account

```bash
# Get your AWS account ID
AWS_ACCOUNT_ID=$(aws sts get-caller-identity --query Account --output text)
CLUSTER_NAME=your-eks-cluster-name  # ← UPDATE THIS

# Create IAM role for the controller
aws iam create-role \
  --role-name AmazonEKSLoadBalancerControllerRole \
  --assume-role-policy-document '{
    "Version": "2012-10-17",
    "Statement": [
      {
        "Effect": "Allow",
        "Principal": {
          "Federated": "arn:aws:iam::'"$AWS_ACCOUNT_ID"':oidc-provider/oidc.eks.'"$(aws eks describe-cluster --name $CLUSTER_NAME --query 'cluster.identity.oidc.issuer' --output text | sed -e 's|^https://||')"'"
        },
        "Action": "sts:AssumeRoleWithWebIdentity",
        "Condition": {
          "StringEquals": {
            "oidc.eks.'"$(aws eks describe-cluster --name $CLUSTER_NAME --query 'cluster.identity.oidc.issuer' --output text | sed -e 's|^https://||')"':sub": "system:serviceaccount:kube-system:aws-load-balancer-controller"
          }
        }
      }
    ]
  }'

# Attach policy to role
aws iam attach-role-policy \
  --role-name AmazonEKSLoadBalancerControllerRole \
  --policy-arn arn:aws:iam::$AWS_ACCOUNT_ID:policy/AWSLoadBalancerControllerIAMPolicy
```

---

## Step 3: Create Service Account and Kubernetes Role

```bash
kubectl apply -f - <<EOF
apiVersion: v1
kind: ServiceAccount
metadata:
  name: aws-load-balancer-controller
  namespace: kube-system
  annotations:
    eks.amazonaws.com/role-arn: arn:aws:iam::$(aws sts get-caller-identity --query Account --output text):role/AmazonEKSLoadBalancerControllerRole

---
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRole
metadata:
  name: aws-load-balancer-controller
rules:
  - apiGroups:
      - ""
      - extensions
    resources:
      - configmaps
      - endpoints
      - events
      - ingresses
      - ingresses/status
      - services
      - services/status
    verbs:
      - create
      - get
      - list
      - update
      - watch
      - patch
  - apiGroups:
      - ""
      - extensions
    resources:
      - nodes
      - pods
      - secrets
      - services
      - namespaces
    verbs:
      - get
      - list
      - watch
  - apiGroups:
      - networking.k8s.io
    resources:
      - ingresses
      - ingressclasses
    verbs:
      - get
      - list
      - watch
  - apiGroups:
      - elbv2.k8s.aws
    resources:
      - targetgroupbindings
      - targetgroupbindings/status
    verbs:
      - create
      - delete
      - get
      - list
      - patch
      - update
      - watch
  - apiGroups:
      - ec2.k8s.aws
    resources:
      - securitygroupingresses
      - securitygroupingresses/status
    verbs:
      - create
      - delete
      - get
      - list
      - patch
      - update
      - watch
  - apiGroups:
      - ec2.k8s.aws
    resources:
      - securitygroups
    verbs:
      - get
      - list
      - watch
  - apiGroups:
      - ""
    resources:
      - namespaces
    verbs:
      - get
      - list
      - watch

---
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRoleBinding
metadata:
  name: aws-load-balancer-controller
roleRef:
  apiGroup: rbac.authorization.k8s.io
  kind: ClusterRole
  name: aws-load-balancer-controller
subjects:
  - kind: ServiceAccount
    name: aws-load-balancer-controller
    namespace: kube-system
EOF
```

---

## Step 4: Install AWS Load Balancer Controller via Helm

```bash
# Add Helm repo
helm repo add eks https://aws.github.io/eks-charts
helm repo update

# Install the controller
helm install aws-load-balancer-controller eks/aws-load-balancer-controller \
  -n kube-system \
  --set serviceAccount.create=false \
  --set serviceAccount.name=aws-load-balancer-controller \
  --set clusterName=your-eks-cluster-name  # ← UPDATE THIS
```

Verify installation:

```bash
kubectl get deployment -n kube-system aws-load-balancer-controller
kubectl get pods -n kube-system -l app.kubernetes.io/name=aws-load-balancer-controller
```

---

## Step 5: Create Kubernetes Secret for Database Password

```bash
kubectl create namespace invoice-app

kubectl -n invoice-app create secret generic invoice-secrets \
  --from-literal=database-url='postgresql://postgres:your-password@invoice-postgres:5432/invoice_db' \
  --from-literal=postgres-password='your-password' \
  --from-literal=secret-key='your-random-secret-key'
```

Or use kubectl to generate:

```bash
python -c "import secrets; print(secrets.token_hex(32))"
```

---

## Step 6: Update my-values.yaml for EKS

```yaml
namespace: invoice-app

image:
  username: your-dockerhub-username  # ← UPDATE
  backendTag: latest
  frontendTag: latest

backend:
  replicaCount: 1
  secretKey: "your-random-secret-key"  # ← UPDATE
  corsOrigins: "https://invoice.vihan.online"  # ← UPDATE
  debug: "False"

frontend:
  replicaCount: 1

postgres:
  user: postgres
  password: your-password  # ← UPDATE
  database: invoice_db
  storage: 5Gi
  storageClassName: gp2  # AWS EBS

ingress:
  enabled: true
  frontendHost: invoice.vihan.online  # ← UPDATE
  backendHost: backend.vihan.online   # ← UPDATE
  annotations:
    kubernetes.io/ingress.class: alb
    alb.ingress.kubernetes.io/scheme: internet-facing
    alb.ingress.kubernetes.io/target-type: ip
```

---

## Step 7: Deploy Invoice App with Helm

```bash
helm upgrade --install invoice-app ./invoice-app \
  --namespace invoice-app \
  -f my-values.yaml
```

---

## Step 8: Verify Deployment

```bash
# Check pods
kubectl get pods -n invoice-app

# Check services
kubectl get svc -n invoice-app

# Check Ingress (wait for ALB to provision - may take 2-3 minutes)
kubectl get ingress -n invoice-app
kubectl describe ingress -n invoice-app

# Get ALB DNS name
ALB_HOSTNAME=$(kubectl get ingress -n invoice-app -o jsonpath='{.items[0].status.loadBalancer.ingress[0].hostname}')
echo "Frontend URL: http://$ALB_HOSTNAME"
```

---

## Step 9: Point Domain to ALB

Once Ingress shows the ALB hostname, update your domain's DNS:

- Go to your domain registrar (Route53, GoDaddy, Namecheap, etc.)
- Create CNAME record:
  ```
  invoice.vihan.online  → ALB_HOSTNAME
  backend.vihan.online  → ALB_HOSTNAME
  ```

---

## Step 10: Verify the App

Wait 2-3 minutes for DNS propagation, then:

```bash
curl http://invoice.vihan.online
```

You should see the frontend.

---

## Troubleshooting

### ALB not provisioning
```bash
kubectl describe ingress -n invoice-app
kubectl logs -n kube-system -l app.kubernetes.io/name=aws-load-balancer-controller
```

### Backend pod pending
```bash
kubectl describe pod -n invoice-app -l app=invoice-backend
kubectl logs -n invoice-app -l app=invoice-backend
```

### Database pod failing
```bash
kubectl describe statefulset -n invoice-app invoice-postgres
kubectl logs -n invoice-app invoice-postgres-0
```

### Check PVC binding
```bash
kubectl get pvc -n invoice-app
```

---

## Clean up

```bash
helm uninstall invoice-app -n invoice-app
kubectl delete namespace invoice-app
```
