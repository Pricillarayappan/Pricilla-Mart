# PRICILLA MART
### Multi-Seller Online Shopping E-Commerce Platform
**Final Year College Capstone Project — Java Full-Stack**

---

## 📌 Project Overview

**PRICILLA MART** is a complete, fully functional, multi-seller online shopping e-commerce web platform engineered for a **College Computer Science & Engineering Capstone Project**.

The platform is built using a modern **Java Full-Stack architecture (Java 17+ / Spring Boot 3 & Maven)** coupled with an **SQLite3 relational database** and a responsive **HTML5/CSS3/JavaScript frontend** adhering to an elegant **purple and lilac reference color palette**.

---

## 🎨 Color Palette & Branding

All 17 pages, dashboards, modals, and elements strictly adhere to the designated purple/lilac brand palette:

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
- **Checkout & Multi-Mode Payments:** Delivery address input, payment method selection (**Cash on Delivery**, **Demo UPI / QR Code**, **Demo Credit/Debit Card** with 1-click quick-fill helpers), atomic inventory deduction, and unique Order ID generation (e.g., `PRC10001`).
- **Order History & Tracking:** Interactive shipment lifecycle timeline (`Pending` → `Confirmed` → `Shipped` → `Delivered` or `Cancelled`) with payment method badge.
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

- **Backend:** Java 17+, Spring Boot 3 (Spring Web, Embedded Tomcat, Spring MVC REST)
- **Database:** SQLite3 (`database/pricillamart.db`) via `sqlite-jdbc` with foreign keys, indexes, and transactions
- **Build System:** Maven (`pom.xml`)
- **Frontend:** HTML5, CSS3, Modern JavaScript (Fetch API, Async/Await)
- **Security:** SHA-256 cryptographic password hashing, Base64URL-encoded HMAC token validation

---

## 📁 Project Directory Structure

```
Pricilla/
├── pom.xml                                   # Maven build configuration
├── start_server.bat                          # 1-click Windows server launcher
├── PricillaMartServer.java                   # Standalone Java HTTP server runner
├── dev_server.py                             # Python local preview runner
├── README.md                                 # Complete documentation
│
├── database/
│   ├── schema.sql                            # Relational SQLite database schema
│   ├── seed.sql                              # Pre-populated demonstration data
│   ├── init_db.py                            # Database initialization script
│   └── pricillamart.db                       # Pre-populated SQLite3 database
│
├── src/main/java/com/pricillamart/
│   ├── PricillaMartApplication.java          # Spring Boot main entry point
│   ├── config/
│   │   └── WebConfig.java                    # Static files & CORS configuration
│   ├── models/
│   │   ├── User.java, Product.java, Category.java
│   │   ├── CartItem.java, Order.java, OrderItem.java, Review.java
│   │   └── Review.java
│   ├── services/
│   │   ├── DatabaseService.java              # Thread-safe SQLite CRUD & transactions
│   │   ├── AuthService.java                  # User registration & login logic
│   │   └── ChatbotService.java              # Contextual AI assistant logic
│   ├── controllers/
│   │   ├── AuthController.java               # /api/register, /api/login, /api/profile
│   │   ├── ProductController.java            # /api/products, search & CRUD
│   │   ├── CategoryController.java           # /api/categories
│   │   ├── CartController.java               # /api/cart operations
│   │   ├── OrderController.java              # /api/orders & checkout
│   │   ├── ReviewController.java             # /api/reviews & ratings
│   │   ├── AdminController.java              # /api/admin/stats, /api/admin/users
│   │   └── ChatbotController.java            # /api/chatbot
│   └── utils/
│       ├── ApiResponse.java                  # Standard JSON envelope
│       ├── PasswordUtil.java                 # SHA-256 hashing
│       └── JwtUtil.java                      # JWT token generator & validator
│
├── src/main/resources/
│   └── application.properties                # Spring Boot configuration
│
├── frontend/
│   ├── index.html                            # Homepage (Hero, categories, products)
│   ├── login.html                            # Login page with 1-click viva demo buttons
│   ├── register.html                         # Registration (Buyer / Seller)
│   ├── products.html                         # Catalog with search & filters
│   ├── product-details.html                  # Product details & reviews
│   ├── cart.html                             # Shopping cart
│   ├── checkout.html                         # Checkout form & order success modal
│   ├── orders.html                           # Order history & tracking
│   ├── profile.html                          # User account profile
│   ├── 404.html                              # Not found fallback
│   ├── seller/                               # Seller Hub (Dashboard, Products, Orders)
│   ├── admin/                                # Admin Panel (Dashboard, Users, Products, Orders)
│   ├── css/style.css                         # Official purple/lilac stylesheet
│   └── js/                                   # Frontend modules (api, auth, products, cart, etc.)
│
└── docs/
    └── project-documentation.md              # Detailed Capstone Project Report & viva guide
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

## 🚀 How to Run the Server & Open Localhost

### Method 1: Instant 1-Click Launcher (Windows)
Double-click **`start_server.bat`** or run:
```cmd
start_server.bat
```
Then open your web browser at:
👉 **`http://localhost:8080`**

### Method 2: Standalone Java Server
If Java is installed on your machine:
```cmd
java PricillaMartServer.java
```
Open **`http://localhost:8080`** in your browser.

### Method 3: Spring Boot with Maven
```cmd
mvn spring-boot:run
```
Or package into an executable JAR:
```cmd
mvn clean package
java -jar target/pricillamart-1.0.0.jar
```
Open **`http://localhost:8080`** in your browser.

### Method 4: Python Dev Server
```cmd
python dev_server.py
```
Open **`http://localhost:8080`** in your browser.

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
