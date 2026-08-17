# AWS Load Balancer Controller Setup for EKS

## Prerequisites
- EKS cluster created
- AWS CLI configured
- kubectl connected to EKS cluster
- helm installed

---

## Step 1: Create IAM Policy

```bash
AWS_ACCOUNT_ID=$(aws sts get-caller-identity --query Account --output text)

aws iam create-policy \
  --policy-name AWSLoadBalancerControllerIAMPolicy \
  --policy-document file://aws-load-balancer-controller-policy.json
```

---

## Step 2: Create IAM Role

Replace `YOUR_AWS_ACCOUNT_ID` with your actual account ID:

```bash
AWS_ACCOUNT_ID=$(aws sts get-caller-identity --query Account --output text)
CLUSTER_NAME=your-eks-cluster-name  # ← UPDATE THIS
OIDC_ID=$(aws eks describe-cluster --name $CLUSTER_NAME --query 'cluster.identity.oidc.issuer' --output text | sed -e 's|^https://||')

aws iam create-role \
  --role-name AmazonEKSLoadBalancerControllerRole \
  --assume-role-policy-document '{
    "Version": "2012-10-17",
    "Statement": [
      {
        "Effect": "Allow",
        "Principal": {
          "Federated": "arn:aws:iam::'"$AWS_ACCOUNT_ID"':oidc-provider/'"$OIDC_ID"'"
        },
        "Action": "sts:AssumeRoleWithWebIdentity",
        "Condition": {
          "StringEquals": {
            "'"$OIDC_ID"':sub": "system:serviceaccount:kube-system:aws-load-balancer-controller"
          }
        }
      }
    ]
  }'

aws iam attach-role-policy \
  --role-name AmazonEKSLoadBalancerControllerRole \
  --policy-arn arn:aws:iam::$AWS_ACCOUNT_ID:policy/AWSLoadBalancerControllerIAMPolicy
```

---

## Step 3: Apply RBAC (ServiceAccount + ClusterRole)

Edit `aws-load-balancer-controller-rbac.yaml` and replace `YOUR_AWS_ACCOUNT_ID` with your actual account ID.

```bash
kubectl apply -f aws-load-balancer-controller-rbac.yaml
```

---

## Step 4: Install AWS Load Balancer Controller via Helm

```bash
CLUSTER_NAME=your-eks-cluster-name  # ← UPDATE THIS

helm repo add eks https://aws.github.io/eks-charts
helm repo update

helm install aws-load-balancer-controller eks/aws-load-balancer-controller \
  -n kube-system \
  --set serviceAccount.create=false \
  --set serviceAccount.name=aws-load-balancer-controller \
  --set clusterName=$CLUSTER_NAME
```

Verify:

```bash
kubectl get deployment -n kube-system aws-load-balancer-controller
kubectl get pods -n kube-system -l app.kubernetes.io/name=aws-load-balancer-controller
```

---

## Step 5: Create Kubernetes Secret for Invoice App

```bash
kubectl create namespace invoice-app

kubectl -n invoice-app create secret generic invoice-secrets \
  --from-literal=database-url='postgresql://postgres:YOUR_DB_PASSWORD@invoice-postgres:5432/invoice_db' \
  --from-literal=postgres-password='YOUR_DB_PASSWORD' \
  --from-literal=secret-key='YOUR_RANDOM_SECRET_KEY'
```

Generate a random secret key:

```bash
python -c "import secrets; print(secrets.token_hex(32))"
```

---

## Step 6: Deploy Invoice App (RBAC included in Helm chart)

Edit `my-values.yaml` with your AWS account ID and other values, then:

```bash
helm upgrade --install invoice-app ./invoice-app \
  --namespace invoice-app \
  -f my-values.yaml
```

This Helm deployment will:
- Create the AWS Load Balancer Controller RBAC (ServiceAccount, ClusterRole, ClusterRoleBinding)
- Deploy PostgreSQL, backend, and frontend
- Create Ingress for ALB provisioning

---

## Step 7: Verify Deployment

```bash
# Check pods
kubectl get pods -n invoice-app

# Check services
kubectl get svc -n invoice-app

# Check Ingress (wait 2-3 minutes for ALB to provision)
kubectl get ingress -n invoice-app
kubectl describe ingress -n invoice-app

# Get ALB hostname
kubectl get ingress -n invoice-app -o jsonpath='{.items[0].status.loadBalancer.ingress[0].hostname}'
```

---

## Step 8: Point Domain to ALB

Add CNAME records in your DNS:

```
invoice.vihan.online  → ALB_HOSTNAME
backend.vihan.online  → ALB_HOSTNAME
```

---

## Step 9: Test

```bash
curl http://invoice.vihan.online
```

---

## Cleanup

```bash
helm uninstall invoice-app -n invoice-app
kubectl delete namespace invoice-app
helm uninstall aws-load-balancer-controller -n kube-system
```
