aws eks update-kubeconfig --region ap-south-1 --name vihan-cluster

oidc_id=$(aws eks describe-cluster --name vihan-cluster --query "cluster.identity.oidc.issuer" --output text | cut -d '/' -f 5)
aws iam attach-role-policy \
  --policy-arn arn:aws:iam::175864702507:policy/AWSLoadBalancerControllerIAMPolicy \
  --role-name AmazonEKSLoadBalancerControllerRole
helm install aws-load-balancer-controller eks/aws-load-balancer-controller \
  -n kube-system \
  --set clusterName=vihan-cluster \
  --set serviceAccount.create=false \
  --set serviceAccount.name=aws-load-balancer-controller \
  --version 1.14.0

kubectl create namespace invoice-app

kubectl -n invoice-app create secret generic invoice-secrets \
  --from-literal=database-url='postgresql://postgres:postgres@invoice-postgres:5432/invoice_db' \
  --from-literal=postgres-password='postgres' \
  --from-literal=secret-key='local-dev-secret'

kubectl delete namespace invoice-app ; \
helm upgrade --install invoice-app ./invoice-app \
  --namespace invoice-app \
  --create-namespace \
  -f my-values.yaml


kubectl get pods -n invoice-app

kubectl get svc -n invoice-app







Runninng commands below -

# Set variables
AWS_ACCOUNT_ID=$(aws sts get-caller-identity --query Account --output text)
CLUSTER_NAME=your-eks-cluster-name  # ← UPDATE THIS
OIDC_ID=$(aws eks describe-cluster --name $CLUSTER_NAME --query 'cluster.identity.oidc.issuer' --output text | sed -e 's|^https://||')

# Step 1: Create IAM Policy
aws iam create-policy \
  --policy-name AWSLoadBalancerControllerIAMPolicy \
  --policy-document file://aws-load-balancer-controller-policy.json

# Step 2: Create IAM Role
aws iam create-role \
  --role-name AmazonEKSLoadBalancerControllerRole \
  --assume-role-policy-document '{"Version":"2012-10-17","Statement":[{"Effect":"Allow","Principal":{"Federated":"arn:aws:iam::'"$AWS_ACCOUNT_ID"':oidc-provider/'"$OIDC_ID"'"},"Action":"sts:AssumeRoleWithWebIdentity","Condition":{"StringEquals":{"'"$OIDC_ID"':sub":"system:serviceaccount:kube-system:aws-load-balancer-controller"}}}]}'

# Step 3: Attach Policy to Role
aws iam attach-role-policy \
  --role-name AmazonEKSLoadBalancerControllerRole \
  --policy-arn arn:aws:iam::$AWS_ACCOUNT_ID:policy/AWSLoadBalancerControllerIAMPolicy

# Step 4: Create namespace
kubectl create namespace invoice-app

# Step 5: Create secret with DB password
kubectl -n invoice-app create secret generic invoice-secrets \
  --from-literal=database-url='postgresql://postgres:postgres@invoice-postgres:5432/invoice_db' \
  --from-literal=postgres-password='postgres' \
  --from-literal=secret-key='local-dev-secret-key'

# Step 6: Update my-values.yaml with your AWS account ID (edit manually)
# Replace "YOUR_AWS_ACCOUNT_ID" with the actual account ID in my-values.yaml
sed -i "s/YOUR_AWS_ACCOUNT_ID/$AWS_ACCOUNT_ID/g" my-values.yaml

helm repo add eks https://aws.github.io/eks-charts
helm repo update

helm install aws-load-balancer-controller eks/aws-load-balancer-controller \
  -n kube-system \
  --set serviceAccount.create=false \
  --set serviceAccount.name=aws-load-balancer-controller \
  --set clusterName=$CLUSTER_NAME

helm upgrade --install invoice-app ./invoice-app \
  --namespace invoice-app \
  --create-namespace \
  -f my-values.yaml


kubectl label serviceaccount aws-load-balancer-controller \
  -n kube-system \
  app.kubernetes.io/managed-by=Helm \
  --overwrite

kubectl annotate serviceaccount aws-load-balancer-controller \
  -n kube-system \
  meta.helm.sh/release-name=invoice-app \
  meta.helm.sh/release-namespace=invoice-app \
  --overwrite















# Step 7: Deploy with Helm
helm upgrade --install invoice-app ./invoice-app \
  --namespace invoice-app \
  -f my-values.yaml

# Step 8: Wait for ALB to provision (2-3 minutes)
kubectl get ingress -n invoice-app -w

# Step 9: Get ALB hostname
kubectl get ingress -n invoice-app -o jsonpath='{.items[0].status.loadBalancer.ingress[0].hostname}'