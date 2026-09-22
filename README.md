# PRICILLA MART
### Multi-Seller Online Shopping E-Commerce Platform
**Final Year College Capstone Project**

---

## 📌 Project Overview

**PRICILLA MART** is a complete, fully functional, multi-seller online shopping e-commerce web platform engineered for a **College Computer Science & Engineering Capstone Project**.

Unlike conventional prototype projects that rely on Node.js or Python, the core backend of Pricilla Mart is built in **Modern C++20**, following the high-performance **Drogon Framework RESTful architecture** coupled with an **SQLite3 relational database**. The client frontend is implemented in responsive **HTML5, CSS3, and JavaScript**, styled with an exclusive, luxurious **purple and lilac reference color palette**.

---

## 🎨 Color Palette & Branding

All pages, dashboards, modals, and elements strictly adhere to the designated purple/lilac brand palette:

| Element | Color Name | HEX Code | Purpose |
|---|---|---|---|
| **Primary** | Midnight Orchid | `#312A44` | Navbar, footer, primary buttons, admin sidebar, headings |
| **Secondary** | Dusky Lilac | `#88708E` | Secondary buttons, borders, category tabs, hover states |
| **Light** | Iris Mist | `#BAB0C8` | Input borders, soft backgrounds, secondary badges |
| **Card** | Plum Blossom | `#D7C5D2` | Product cards, section cards, summary boxes |
| **Background** | Silver Wisteria | `#DAD4DF` | Main page canvas, layout backgrounds, tables |
| **Input Surface**| Pale Lavender | `#F5F2F7` | Form inputs and search fields |
| **Pure White** | White | `#FFFFFF` | Text on dark buttons, card inner highlights |

---

## 🚀 Key Modules & Features

### 1. Multi-Role Authentication & Access Control
- **Buyer Registration & Login:** Password hashing with SHA-256 and stateless JWT token authentication.
- **Seller Registration & Login:** Dedicated seller workflow for storefront inventory management.
- **Predefined Administrator:** Full platform governance, user audit, and content moderation.
- **Role Redirection:** Automatic routing to Buyer storefront, Seller Hub, or Admin Control Panel.

### 2. Buyer Module
- **Live Search & Filter:** Search by keywords across title, description, and categories.
- **Faceted Category Filtering:** Instant category chips for Electronics, Fashion, Accessories, Home, Beauty, and Books.
- **Product Details:** High-resolution product images, seller information, available stock count, and average star ratings.
- **Dynamic Shopping Cart:** Add items, increment/decrement quantities, automatic stock-cap validation, and real-time subtotal calculation.
- **Checkout:** Delivery address input, order summary breakdown, atomic inventory deduction, and unique Order ID generation (e.g., `PRC10001`).
- **Order History & Tracking:** Interactive shipment lifecycle timeline (`Pending` → `Confirmed` → `Shipped` → `Delivered` or `Cancelled`).
- **Product Reviews & Ratings:** 1-to-5 star interactive picker and verified customer reviews.

### 3. Seller Module
- **Storefront Dashboard:** Real-time KPI cards for **My Products**, **Total Orders Received**, and **Gross Revenue**.
- **Inventory Management:** Add new products, edit price/stock/description, and remove products with instant catalog synchronization.
- **Order Fulfillment:** Inspect received customer orders with buyer contact details, and update dispatch status.

### 4. Admin Module
- **Executive Analytics:** High-level metrics: Total Users, Total Buyers, Total Sellers, Total Products, Total Orders, Total Sales Revenue.
- **User Governance:** Audit registered buyers, multi-sellers, and administrators with role-filtering.
- **Catalog Moderation:** Remove inappropriate listings with a safety confirmation dialog to prevent accidental deletion.
- **Global Order Audit:** Inspect and override shipment statuses across all vendors.

### 5. AI Shopping Assistant Chatbot
- **Interactive Floating Widget:** Persistent bottom-right conversational assistant on all pages.
- **Contextual Search & Recommendations:** Natural queries (e.g., *"I need a laptop"*, *"Show me fashion products"*, *"How do I track my order?"*).
- **Database Integrated:** Directly queries SQLite3 for in-stock products and renders clickable product recommendation cards inside the chat window.
- **Offline Rule-Based Engine:** Operates 100% reliably without requiring external cloud API keys for college evaluation. Supports optional external LLM via `CHATBOT_API_KEY` in `.env`.

---

## 🛠️ Technology Stack

- **Backend:** C++20 (Standard C++20, multithreaded HTTP REST routing, Drogon MVC controller design)
- **Database:** SQLite3 (Relational database with foreign key constraints, indexes, and transactions)
- **Build System:** CMake (Version 3.15+)
- **Frontend:** HTML5, CSS3, Modern JavaScript (Fetch API, Async/Await)
- **Security:** SHA-256 cryptographic password hashing, Base64URL-encoded HMAC token validation

---

## 📁 Project Directory Structure

```
PricillaMart/
├── CMakeLists.txt              # Root CMake build configuration
├── README.md                   # Project documentation & viva guide
├── .gitignore                  # Git ignore rules (builds, databases, envs)
├── .env.example                # Environment variable template
│
├── database/
│   ├── schema.sql              # Relational SQLite database schema
│   ├── seed.sql                # Pre-populated demonstration data
│   └── init_db.py              # Quick Python SQLite initialization script
│
├── backend/
│   ├── main.cpp                # Self-contained C++20 REST server entry point
│   ├── drogon_main.cpp         # Drogon framework C++ entry point
│   ├── config/
│   │   ├── AppConfig.h         # Configuration loader
│   │   └── config.json         # Drogon server settings
│   ├── database/
│   │   ├── DatabaseManager.h   # SQLite3 database manager header
│   │   └── DatabaseManager.cpp # Database operations and transactions
│   ├── models/
│   │   ├── User.h              # User model
│   │   ├── Product.h           # Product model
│   │   ├── Category.h          # Category model
│   │   ├── CartItem.h          # Cart item model
│   │   ├── Order.h             # Order model
│   │   └── Review.h            # Review model
│   ├── controllers/
│   │   ├── AuthController.h / .cpp       # User registration & login
│   │   ├── ProductController.h / .cpp    # Catalog & search
│   │   ├── CategoryController.h / .cpp   # Category hierarchy
│   │   ├── CartController.h / .cpp       # Shopping cart operations
│   │   ├── OrderController.h / .cpp      # Checkout & orders
│   │   ├── ReviewController.h / .cpp     # Reviews & ratings
│   │   ├── AdminController.h / .cpp      # Admin & seller analytics
│   │   └── ChatbotController.h / .cpp    # AI Chatbot engine
│   └── utils/
│       ├── PasswordHasher.h / .cpp       # SHA-256 hashing
│       ├── JwtUtil.h                     # Token generation & validation
│       └── ResponseUtil.h                # JSON formatting helpers
│
├── frontend/
│   ├── index.html              # Homepage
│   ├── login.html              # Login page with demo one-click buttons
│   ├── register.html           # User registration (Buyer / Seller)
│   ├── products.html           # Product catalog with search & filters
│   ├── product-details.html    # Product specifications & reviews
│   ├── cart.html               # Shopping cart & subtotal summary
│   ├── checkout.html           # Shipping address & order confirmation
│   ├── orders.html             # Order history & tracking timeline
│   ├── profile.html            # User account profile
│   ├── 404.html                # Not found fallback page
│   ├── seller/
│   │   ├── dashboard.html      # Seller dashboard with KPI stats
│   │   ├── products.html       # Seller inventory manager
│   │   └── orders.html         # Seller received orders manager
│   ├── admin/
│   │   ├── dashboard.html      # Admin KPI control center
│   │   ├── users.html          # Admin user directory & role filter
│   │   ├── products.html       # Admin global product moderation
│   │   └── orders.html         # Admin platform-wide order audit
│   ├── css/
│   │   └── style.css           # Official purple/lilac stylesheet
│   └── js/
│       ├── api.js              # REST client wrapper
│       ├── auth.js             # Authentication & session state
│       ├── products.js         # Product listing & search
│       ├── cart.js             # Shopping cart operations
│       ├── orders.js           # Orders & checkout operations
│       ├── reviews.js          # Product reviews & star rating
│       ├── chatbot.js          # AI Chatbot floating widget
│       └── dashboard.js        # Seller & admin dashboard operations
│
└── docs/
    └── project-documentation.md # Detailed Capstone Project Report & viva guide
```

---

## 🔑 Default Demonstration Accounts

For quick professor examination, evaluation, and live demonstration, pre-seeded accounts are provided:

| Role | Email | Password | Pre-seeded Permissions |
|---|---|---|---|
| **Admin** | `admin@pricillamart.com` | `Admin@123` | Full access to Admin Panel, user audit, catalog moderation, orders |
| **Seller** | `techworld@pricillamart.com` | `Seller@123` | Electronics seller with active products and received orders |
| **Buyer** | `rahul@gmail.com` | `Buyer@123` | Active buyer with cart items, placed orders, and reviews |

*Note: On the login page, you can also use the **Quick Demo Logins** one-click buttons to instantly pre-fill credentials.*

---

## ⚙️ Setup & Execution Instructions

### Option 1: Database Initialization
Before starting the backend, initialize the SQLite database using Python (built-in SQLite module):
```bash
python database/init_db.py
```
*Note: The C++ backend also verifies and seeds the database automatically on startup if `database/pricillamart.db` is not present.*

### Option 2: Build with CMake & C++ Compiler
```bash
# Generate build configuration
cmake -S . -B build

# Compile project executable
cmake --build build --config Release

# Run Pricilla Mart
./build/bin/pricillamart_server
```

### Option 3: Direct Compilation (GCC / Clang / MSVC)
On Windows with MinGW/GCC:
```bash
g++ -std=c++20 backend/main.cpp backend/database/DatabaseManager.cpp backend/utils/PasswordHasher.cpp backend/controllers/*.cpp -Ibackend -lsqlite3 -lws2_32 -o pricillamart_server.exe
./pricillamart_server.exe
```

On Linux / macOS:
```bash
g++ -std=c++20 backend/main.cpp backend/database/DatabaseManager.cpp backend/utils/PasswordHasher.cpp backend/controllers/*.cpp -Ibackend -lsqlite3 -lpthread -o pricillamart_server
./pricillamart_server
```

Once running, open your web browser at:
👉 **`http://localhost:8080`**

---

## 🧪 Testing Verification Scenarios

### Test 1: Buyer Flow
1. Open `http://localhost:8080` and click **Login**.
2. Click the **Buyer** quick-fill button (`rahul@gmail.com`) and sign in.
3. Browse products or use the search bar to search for `"Laptop"`.
4. Click on the product card to view specifications and reviews.
5. Click **Add to Cart**, then open the Cart.
6. Adjust the quantity and proceed to **Checkout**.
7. Confirm the order to receive your unique Order ID (`PRC10001`).
8. View the order status and leave a 5-star review.

### Test 2: Seller Flow
1. Log in with Seller credentials (`techworld@pricillamart.com`).
2. Verify the Seller KPI metrics: Products count, Orders count, and Gross Revenue.
3. Click **Add New Product**, enter details, and save.
4. Verify the product immediately appears in the inventory and on the public catalog.
5. Navigate to **Received Orders** and advance an order from `Confirmed` to `Shipped`.

### Test 3: Admin Flow
1. Log in with Admin credentials (`admin@pricillamart.com` / `Admin@123`).
2. Review system-wide platform statistics.
3. Open **Users** to view and filter accounts by role.
4. Open **Products** to review all multi-seller items and verify safe deletion confirmation.
5. Open **Orders** to inspect all transactions across the platform.

### Test 4: AI Chatbot Demonstration
1. Click the purple floating robot icon in the bottom-right corner.
2. Ask: *"I need a laptop"* or *"Show me fashion products"*.
3. Observe direct product card recommendations with prices and cart shortcuts.
4. Ask: *"How do I track my order?"* to view your live account status.

---

## 📄 License & Academic Integrity

This project is developed as an individual undergraduate Computer Science and Engineering Capstone Project. All rights reserved.
