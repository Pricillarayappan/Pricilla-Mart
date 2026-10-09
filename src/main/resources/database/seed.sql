-- ============================================================
-- PRICILLA MART - SEED DATA
-- Pre-populated sample data for Capstone Viva / Demo
-- ============================================================

-- 1. INSERT CATEGORIES
INSERT OR IGNORE INTO categories (id, name, slug, icon, description) VALUES
(1, 'Electronics', 'electronics', 'fas fa-laptop', 'Laptops, smartphones, audio devices, and cutting-edge tech gadgets'),
(2, 'Fashion', 'fashion', 'fas fa-tshirt', 'Trendy apparel, designer dresses, men and women ethnic wear'),
(3, 'Accessories', 'accessories', 'fas fa-clock', 'Smart watches, luxury handbags, premium wallets, and jewelry'),
(4, 'Home', 'home', 'fas fa-couch', 'Modern home decor, living room lighting, and kitchen appliances'),
(5, 'Beauty', 'beauty', 'fas fa-spa', 'Skincare essentials, fragrance, cosmetics, and wellness items'),
(6, 'Books', 'books', 'fas fa-book', 'Academic textbooks, competitive guides, and bestselling novels');

-- 2. INSERT USERS
-- Predefined Admin (Password: Admin@123)
-- Seller 1 (Password: Seller@123)
-- Seller 2 (Password: Seller@123)
-- Buyer 1 (Password: Buyer@123)
-- Buyer 2 (Password: Buyer@123)
-- Buyer 3 (Password: Buyer@123)
-- Password hash: standard SHA-256 hex strings
INSERT OR IGNORE INTO users (id, name, email, phone, password_hash, role) VALUES
(1, 'System Administrator', 'admin@pricillamart.com', '+91 9876543210', 'a665a45920422f9d417e4867efdc4fb8a04a1f3fff1fa07e998e86f7f7a27ae3', 'admin'),
(2, 'TechWorld Electronics', 'techworld@pricillamart.com', '+91 9876543211', '8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918', 'seller'),
(3, 'Luxe & Style Emporium', 'luxe@pricillamart.com', '+91 9876543212', '8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918', 'seller'),
(4, 'Rahul Sharma', 'rahul@gmail.com', '+91 9876543213', 'b109f3bbbc244eb82441917ed06d618b9008dd09b3befd1b5e07394c706a8bb9', 'buyer'),
(5, 'Priya Patel', 'priya@gmail.com', '+91 9876543214', 'b109f3bbbc244eb82441917ed06d618b9008dd09b3befd1b5e07394c706a8bb9', 'buyer'),
(6, 'John Doe', 'john@gmail.com', '+91 9876543215', 'b109f3bbbc244eb82441917ed06d618b9008dd09b3befd1b5e07394c706a8bb9', 'buyer');

-- 3. INSERT PRODUCTS (Diverse, high-quality sample data)
INSERT OR IGNORE INTO products (id, name, description, category_id, price, stock_quantity, image_url, seller_id) VALUES
(1, 'UltraBook Pro 15.6" Laptop', 'High performance Intel Core i7 12th Gen, 16GB RAM, 512GB NVMe SSD, Full HD IPS Anti-Glare Display with backlit keyboard. Ideal for engineering, coding, and multitasking.', 1, 45000.00, 15, 'https://images.unsplash.com/photo-1496181133206-80ce9b88a853?w=600&auto=format&fit=crop&q=80', 2),
(2, 'Galaxy Fusion 5G Smartphone', '6.6" 120Hz Super AMOLED Display, 64MP OIS Triple Camera setup, 5000mAh all-day battery with 33W Fast Turbo Charging. 8GB RAM + 128GB Storage.', 1, 18000.00, 30, 'https://images.unsplash.com/photo-1511707171634-5f897ff02aa9?w=600&auto=format&fit=crop&q=80', 2),
(3, 'SonicWave Wireless ANC Headphones', 'Premium Over-Ear Bluetooth 5.3 headphones with Active Noise Cancellation, Deep Bass Equalizer, 40 hours continuous playtime, and ultra-soft memory foam earcups.', 1, 999.00, 50, 'https://images.unsplash.com/photo-1505740420928-5e560c06d30e?w=600&auto=format&fit=crop&q=80', 2),
(4, 'AuraFit Pro Smart Watch', '1.85" HD Always-On Touchscreen, SpO2 blood oxygen monitor, 24/7 heart rate tracking, Bluetooth calling, 100+ Sports modes, IP68 water resistant.', 3, 1999.00, 40, 'https://images.unsplash.com/photo-1523275335684-37898b6baf30?w=600&auto=format&fit=crop&q=80', 2),
(5, 'Vintage Leather Handbag', 'Handcrafted genuine vegan leather tote bag with gold-tone hardware, multiple zippered organizing compartments, and comfortable shoulder straps.', 3, 1499.00, 25, 'https://images.unsplash.com/photo-1584917865442-de89df76afd3?w=600&auto=format&fit=crop&q=80', 3),
(6, 'Classic Slim-Fit Denim Jacket', 'Premium washed denim cotton jacket with vintage brass button accents, tailored fit, double chest pockets, and durable stitching for casual styling.', 2, 2499.00, 35, 'https://images.unsplash.com/photo-1576995853123-5a10305d93c0?w=600&auto=format&fit=crop&q=80', 3),
(7, 'Nordic Minimalist Desk Lamp', 'Sleek matte finish LED adjustable desk lamp with touch-sensitive 3 color temperatures, stepless dimming, USB charging port, and eye-caring warm diffused illumination.', 4, 1299.00, 20, 'https://images.unsplash.com/photo-1507473885765-e6ed057f782c?w=600&auto=format&fit=crop&q=80', 2),
(8, 'Botanical Radiance Vitamin C Serum', 'All-natural antioxidant face serum formulated with pure 15% Vitamin C, Hyaluronic Acid, and Ferulic Acid for skin glow, hydration, and dark spot correction.', 5, 899.00, 60, 'https://images.unsplash.com/photo-1620916566398-39f1143ab7be?w=600&auto=format&fit=crop&q=80', 3),
(9, 'Data Structures & Algorithms in C++', 'Comprehensive academic master reference textbook covering dynamic programming, graph theory, trees, time complexity, and competitive programming problems.', 6, 699.00, 45, 'https://images.unsplash.com/photo-1532012164546-f432f2e37271?w=600&auto=format&fit=crop&q=80', 2);

-- 4. INSERT SAMPLE ORDERS
INSERT OR IGNORE INTO orders (id, order_code, buyer_id, total_amount, buyer_name, email, phone, delivery_address, payment_method, status, created_at) VALUES
(1, 'PRC10001', 4, 1999.00, 'Rahul Sharma', 'rahul@gmail.com', '+91 9876543213', '42 MG Road, Koramangala 4th Block, Bengaluru, Karnataka 560034', 'UPI', 'Delivered', datetime('now', '-5 days')),
(2, 'PRC10002', 5, 45999.00, 'Priya Patel', 'priya@gmail.com', '+91 9876543214', '78 Heritage Park, Satellite, Ahmedabad, Gujarat 380015', 'Credit Card', 'Shipped', datetime('now', '-2 days')),
(3, 'PRC10003', 6, 1499.00, 'John Doe', 'john@gmail.com', '+91 9876543215', '12 Lakeview Avenue, Powai, Mumbai, Maharashtra 400076', 'Cash on Delivery', 'Confirmed', datetime('now', '-1 hours'));

-- 5. INSERT SAMPLE ORDER ITEMS
INSERT OR IGNORE INTO order_items (id, order_id, product_id, seller_id, quantity, unit_price, subtotal) VALUES
(1, 1, 4, 2, 1, 1999.00, 1999.00),
(2, 2, 1, 2, 1, 45000.00, 45000.00),
(3, 2, 3, 2, 1, 999.00, 999.00),
(4, 3, 5, 3, 1, 1499.00, 1499.00);

-- 6. INSERT SAMPLE REVIEWS
INSERT OR IGNORE INTO reviews (id, product_id, buyer_id, rating, comment, created_at) VALUES
(1, 4, 4, 5, 'Exceptional smart watch! The display is crisp, battery lasts a full 6 days, and the heart rate accuracy is surprisingly good.', datetime('now', '-4 days')),
(2, 1, 5, 5, 'Blazing fast laptop. Visual Studio and CMake compile large C++ projects in seconds. Build quality is top-notch.', datetime('now', '-1 days')),
(3, 3, 4, 4, 'Really great noise cancellation at this budget price point. Bass is punchy and ear pads are comfortable for long study sessions.', datetime('now', '-3 days')),
(4, 5, 6, 5, 'Superb handcrafted quality. The leather texture is authentic and spacious enough to hold my tablet and accessories.', datetime('now', '-1 hours'));
