# 🧾 Jagannath Enterprises - Invoice Management System

A modern, full-stack three-tier application for generating invoices and quotations for rice mill machine parts.

## 🏢 Company Information
- **Owner**: K. Krishna
- **Company**: Jagannath Enterprises
- **Phone**: 8919575870
- **Domain**: Rice Mill Parts Supply & Distribution

---

## 🏗️ Architecture

**Three-Tier Application:**
```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│   Frontend      │────▶│    Backend      │────▶│   Database      │
│  React + TS     │     │  Python FastAPI │     │  PostgreSQL     │
│  Port: 3000     │     │  Port: 8000     │     │  Port: 5432     │
└─────────────────┘     └─────────────────┘     └─────────────────┘
```

### Technology Stack

**Frontend:**
- React 18 with TypeScript
- Vite for build tooling
- Tailwind CSS for styling
- React Query for data fetching
- React Router for navigation

**Backend:**
- Python 3.11+ with FastAPI
- SQLAlchemy ORM
- PostgreSQL database
- ReportLab for PDF generation
- Pydantic for validation

**Deployment:**
- Docker containers
- Nginx for frontend serving
- Ready for Kubernetes/EKS

---

## 📁 Project Structure

```
invoice-generate-app/
├── backend/                    # Python FastAPI Backend
│   ├── app/
│   │   ├── api/               # API route handlers
│   │   ├── models/            # Database models
│   │   ├── schemas/           # Pydantic schemas
│   │   ├── services/          # Business logic
│   │   ├── utils/             # PDF generation
│   │   ├── main.py           # FastAPI application
│   │   ├── config.py         # Configuration
│   │   └── seed_data.py      # Sample data
│   ├── Dockerfile
│   ├── requirements.txt
│   └── README.md
│
├── frontend/                   # React TypeScript Frontend
│   ├── src/
│   │   ├── components/       # UI components
│   │   ├── pages/            # Application pages
│   │   ├── lib/              # API client & utilities
│   │   ├── types/            # TypeScript types
│   │   └── App.tsx           # Main application
│   ├── Dockerfile
│   ├── nginx.conf
│   ├── package.json
│   └── README.md
│
├── SETUP_GUIDE.md             # Detailed setup instructions
├── QUICK_START.md             # Quick start guide
└── README.md                  # This file
```

---

## ⚡ Quick Start

### Option 1: Quick Start (Recommended for Testing)

```bash
# See QUICK_START.md for fastest setup
cat QUICK_START.md
```

### Option 2: Detailed Setup

```bash
# See SETUP_GUIDE.md for complete instructions
cat SETUP_GUIDE.md
```

### Option 3: One-Command Start

```bash
# 1. Start PostgreSQL
brew services start postgresql@15  # macOS
# OR
sudo systemctl start postgresql    # Linux

# 2. Create database
psql postgres -c "CREATE DATABASE invoice_db;"

# 3. Start Backend (Terminal 1)
cd backend && python3 -m venv venv && source venv/bin/activate && \
pip install -r requirements.txt && \
echo "DATABASE_URL=postgresql://postgres:postgres@localhost:5432/invoice_db" > .env && \
python3 -c "from app.database import init_db; init_db()" && \
python3 -m app.seed_data && \
uvicorn app.main:app --reload

# 4. Start Frontend (Terminal 2)
cd frontend && npm install && \
echo "VITE_API_URL=http://localhost:8000" > .env && \
npm run dev
```

**Access:** http://localhost:3000

---

## 🎯 Key Features

### ✅ Implemented Features
- **Customer Management** - Add, edit, search customers with full contact details
- **Parts Catalog** - 20+ pre-loaded rice mill parts with categories
- **Invoice Creation** - Generate invoices or quotations with multiple line items
- **Tax Calculation** - Automatic CGST/SGST calculation
- **PDF Generation** - Professional invoice PDFs with company branding
- **Status Tracking** - Track invoice status (Draft, Sent, Paid, Cancelled)
- **Search & Filter** - Search across customers, parts, and invoices
- **Responsive UI** - Modern, mobile-friendly interface

### Rice Mill Parts Categories
- 🔩 Shafts (Main, Secondary)
- ⚙️ Bearings (Ball, Roller)
- 🔗 Belts & Pulleys
- ⚡ Electric Motors (5HP, 10HP)
- 🎯 Rubber Rollers (Husking, Polishing)
- 🌾 Wire Mesh Screens & Sieves
- 💨 Blowers & Exhaust Fans
- ⚙️ Gears & Gear Boxes
- 🪨 Emery Stones & Abrasives
- 🔧 Accessories & Spares

---

## 🐳 Docker Deployment

### Build Backend
```bash
cd backend
docker build -t invoice-backend .
docker run -d -p 8000:8000 \
  -e DATABASE_URL="postgresql://postgres:postgres@host.docker.internal:5432/invoice_db" \
  --name invoice-backend invoice-backend
```

### Build Frontend
```bash
cd frontend
docker build -t invoice-frontend .
docker run -d -p 80:80 --name invoice-frontend invoice-frontend
```

---

## 📊 Database Schema

**Main Tables:**
- `customers` - Customer information with GST details
- `parts` - Rice mill parts catalog
- `invoices` - Invoice/Quotation headers
- `invoice_items` - Line items for each invoice

**Relationships:**
- One customer → Many invoices
- One invoice → Many invoice items
- Parts reference in invoice items

---

## 🔐 Security Features

- CORS protection
- Environment-based configuration
- SQL injection prevention (SQLAlchemy ORM)
- Input validation (Pydantic)
- Secure database connections

---

## 📱 API Endpoints

**Base URL:** http://localhost:8000

### Customers
- `GET /api/customers` - List customers
- `POST /api/customers` - Create customer
- `GET /api/customers/{id}` - Get customer details
- `PUT /api/customers/{id}` - Update customer
- `DELETE /api/customers/{id}` - Delete customer

### Parts
- `GET /api/parts` - List parts (with filters)
- `GET /api/parts/categories` - Get categories
- `POST /api/parts` - Create part
- `GET /api/parts/{id}` - Get part details
- `PUT /api/parts/{id}` - Update part

### Invoices
- `GET /api/invoices` - List invoices (with filters)
- `POST /api/invoices` - Create invoice
- `GET /api/invoices/{id}` - Get invoice details
- `GET /api/invoices/{id}/pdf` - Download PDF
- `PUT /api/invoices/{id}` - Update invoice
- `POST /api/invoices/{id}/status` - Update status

**Full API Documentation:** http://localhost:8000/docs

---

## 🧪 Sample Data

After running `python3 -m app.seed_data`, you'll have:

**3 Sample Customers:**
- Sri Lakshmi Rice Mills (Guntur)
- Vijaya Rice Industries (Vijayawada)
- Modern Rice Mill (Nellore)

**20 Rice Mill Parts:**
- Shafts, Bearings, Motors, Belts, etc.
- Prices range from ₹180 to ₹22,000
- All with HSN codes for GST

---

## 🚀 Next Steps

### For Production Deployment:
1. **Kubernetes/EKS Setup** (coming soon)
2. **SSL/TLS Configuration**
3. **Environment-specific configs**
4. **Database backups**
5. **Monitoring & logging**

### Future Enhancements:
- [ ] User authentication & authorization
- [ ] Email notifications for invoices
- [ ] WhatsApp integration
- [ ] Payment tracking
- [ ] Inventory management
- [ ] Multi-user support with roles
- [ ] Analytics dashboard
- [ ] Automated backups

---

## 📞 Support & Contact

**Company:** Jagannath Enterprises  
**Owner:** K. Krishna  
**Phone:** 8919575870  
**Location:** Andhra Pradesh, India

---

## 📄 Documentation

- [QUICK_START.md](./QUICK_START.md) - Get started in 5 minutes
- [SETUP_GUIDE.md](./SETUP_GUIDE.md) - Complete setup instructions
- [backend/README.md](./backend/README.md) - Backend documentation
- API Docs: http://localhost:8000/docs

---

## 📄 License

Proprietary - Jagannath Enterprises © 2024

---

## 🙏 Acknowledgments

Built with modern technologies to streamline invoice management for rice mill parts businesses.
