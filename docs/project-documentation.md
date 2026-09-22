# PRICILLA MART - COMPREHENSIVE CAPSTONE PROJECT REPORT & TECHNICAL SPECIFICATION

**Course:** B.Tech / B.E. Computer Science and Engineering  
**Project Title:** PRICILLA MART - Multi-Seller Online Shopping E-Commerce Platform  
**Architecture:** Modern C++20 Full-Stack with Relational SQLite3 Database & Responsive Vanilla Web Architecture  

---

## 1. Abstract

**Pricilla Mart** is an individual, end-to-end multi-seller e-commerce web platform engineered using **C++20**, **Drogon Framework RESTful architectural principles**, an **SQLite3 relational database engine**, and a responsive **HTML5/CSS3/JavaScript frontend**. The system empowers three distinct user roles—**Buyers**, **Multi-Store Sellers**, and **Administrators**—facilitating independent vendor storefront management, dynamic keyword search, category-based faceted filtering, real-time transactional cart calculations, inventory decrement on checkout, persistent order fulfillment workflows, verified customer product reviews, and an interactive **AI Shopping Assistant Chatbot**.

---

## 2. Technology Stack & Design Decisions

### 2.1 Backend Layer (C++20)
- **Language Standard:** C++20 (`-std=c++20` or `/std:c++20`).
- **REST Engine:** High-performance asynchronous RESTful HTTP server architecture adhering to Drogon MVC controller paradigms.
- **Data Serialization:** Standardized JSON protocol with rigorous validation, sanitization, and automated escaping for security.
- **Cryptographic Security:** Custom self-contained SHA-256 password hashing and Base64URL-encoded HMAC token validation for stateless session protection.

### 2.2 Database Layer (SQLite3)
- **Engine:** SQLite3 with `PRAGMA foreign_keys = ON;` and index acceleration.
- **Concurrency & Integrity:** Atomic multi-statement transactions (`BEGIN TRANSACTION; ... COMMIT;`) preventing race conditions during simultaneous stock reduction during checkout.

### 2.3 Frontend Presentation Layer
- **Standard:** Semantic HTML5, Vanilla JavaScript (ES6+ async/await Fetch API), and pure custom CSS3.
- **Brand Palette:** Strict adherence to the designated purple/lilac luxury identity:
  - **Midnight Orchid (`#312A44`):** Primary branding, navigation, headers, and major actions.
  - **Dusky Lilac (`#88708E`):** Secondary buttons, borders, and interactive highlights.
  - **Iris Mist (`#BAB0C8`):** Input borders, soft accents, and badges.
  - **Plum Blossom (`#D7C5D2`):** Cards, modals, and container backgrounds.
  - **Silver Wisteria (`#DAD4DF`):** Primary body and table canvas background.

---

## 3. Database Schema & Entity Relationship

```
users (id, name, email [UQ], phone, password_hash, role, created_at)
  │
  ├── products (id, name, description, category_id, price, stock_quantity, image_url, seller_id, created_at)
  │     │
  │     ├── cart_items (id, user_id, product_id, quantity, created_at)
  │     │
  │     ├── order_items (id, order_id, product_id, seller_id, quantity, unit_price, subtotal)
  │     │
  │     └── reviews (id, product_id, buyer_id, rating [1-5], comment, created_at)
  │
  └── orders (id, order_code [UQ], buyer_id, total_amount, buyer_name, email, phone, delivery_address, status, created_at)
```

---

## 4. REST API Endpoint Directory

### Authentication
- `POST /api/register` – Role-based buyer/seller registration.
- `POST /api/login` – Authentication and session token issuance.
- `POST /api/logout` – Session termination.
- `GET /api/profile` – Current user profile data.

### Products & Categories
- `GET /api/categories` – Category list with product counters.
- `GET /api/products` – Search by query `q`, `category_id`, `sort`, `min_price`, `max_price`.
- `GET /api/products/{id}` – Product details with seller store name and ratings.
- `POST /api/products` – Seller product creation.
- `PUT /api/products/{id}` – Seller/Admin product modification.
- `DELETE /api/products/{id}` – Seller/Admin product removal.

### Shopping Cart & Checkout
- `GET /api/cart` – View active cart with computed item subtotals and grand total.
- `POST /api/cart` – Add product with available stock verification.
- `PUT /api/cart/{id}` – Modify item quantity.
- `DELETE /api/cart/{id}` – Remove specific item.
- `POST /api/orders` – Execute checkout, generate Order ID (e.g. `PRC10001`), decrement inventory, and clear cart.

### Orders & Tracking
- `GET /api/orders` – Role-protected order history (Buyer sees own; Seller sees items sold; Admin sees all).
- `GET /api/orders/{id}` – Detailed order breakdown.
- `PUT /api/orders/{id}/status` – Status update (`Pending` → `Confirmed` → `Shipped` → `Delivered` or `Cancelled`).

### Reviews & Ratings
- `GET /api/products/{id}/reviews` – Fetch reviews and average star rating.
- `POST /api/reviews` – Submit review and 1-5 star rating.

### Admin & Analytics
- `GET /api/admin/stats` – Total users, buyers, sellers, products, orders, and gross revenue.
- `GET /api/admin/users` – Audit all user records.
- `GET /api/seller/stats` – Seller specific revenue and order metrics.

### AI Chatbot
- `POST /api/chatbot` – Offline context-aware conversational assistant querying catalog and providing recommendations.

---

## 5. College Viva / Demonstration Walkthrough Guide

When presenting **Pricilla Mart** to professors, external examiners, or evaluation panels, execute the following 6 test flows:

### Test Flow 1: Buyer Journey
1. Navigate to `/register.html`, select role **Buyer**, fill form and register.
2. Log in at `/login.html` (or click the demo **Buyer** button).
3. Search for `"Laptop"` in the top navigation bar.
4. Filter by the **Electronics** category chip.
5. Click the product card to open `/product-details.html`.
6. Click **Add to Cart**, then navigate to `/cart.html`.
7. Adjust the quantity and verify subtotal changes.
8. Click **Proceed to Checkout**, review the address, and click **Confirm Order**.
9. Observe the **ORDER SUCCESSFUL** modal and generated Order ID (e.g. `PRC10001`).
10. Navigate to `/orders.html` to confirm the order status.

### Test Flow 2: Multi-Seller Hub
1. Log in with Seller credentials (click demo **Seller** button).
2. Observe the Seller Dashboard KPI cards: **My Products**, **Total Orders**, and **Gross Revenue**.
3. Click **Add New Product**, fill in details with initial stock, and save.
4. Verify the newly created product immediately appears in the catalog.
5. Click **Received Orders** to view buyer orders containing seller items, and update the status from `Confirmed` to `Shipped`.

### Test Flow 3: Admin Control Panel
1. Log in with Admin credentials (`admin@pricillamart.com` / `Admin@123`).
2. Review system-wide metrics: Total Users, Total Buyers, Total Sellers, Total Products, Total Orders, Total Sales.
3. Open `/admin/users.html` to inspect accounts.
4. Open `/admin/products.html` and click **Remove** on a test product; confirm the safety confirmation dialog prompts before deletion.

### Test Flow 4: Reviews & Ratings
1. As a logged-in buyer, open an ordered product.
2. Rate the product using the 5-star picker and submit written feedback.
3. Observe the calculated average star rating update in real-time.

### Test Flow 5: AI Chatbot Demonstration
1. Click the floating robot trigger in the bottom-right corner.
2. Ask: *"I need a laptop"* → Observe the bot returns recommendations with pricing and direct cart buttons.
3. Ask: *"How do I track my order?"* → Observe real-time order tracking details fetched directly from the database.

---

## 6. Conclusion

Pricilla Mart satisfies all mandatory Capstone specifications for a multi-seller online shopping marketplace. By uniting C++20 backend performance, atomic relational integrity in SQLite3, and a modern responsive user interface, the project provides a benchmark architecture for academic evaluation and real-world software engineering portfolios.
