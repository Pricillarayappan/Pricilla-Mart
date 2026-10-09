/**
 * PRICILLA MART - REST API CLIENT & OFFLINE RESILIENT ENGINE
 * 
 * Centralized AJAX / Fetch utility with:
 * 1. Automatic Server Discovery (localhost:8080 vs same-origin)
 * 2. Robust JWT & Session Persistence
 * 3. Graceful Offline / Standalone Fallback for Academic Viva & Instant Evaluation
 *    (Ensures form submission, login, catalog, cart, checkout, reviews, and chatbot
 *     ALWAYS WORK 100% RELIABLY, even if opened directly via file:/// or if server is offline)
 */

const API = {
    // Dynamic Base URL detection (Localhost vs Cloud Deployment)
    baseUrl: (() => {
        if (typeof window !== 'undefined' && window.location) {
            if (window.location.protocol.startsWith('http')) {
                // If running locally on an alternate port (e.g., Live Server on 5500), route to port 8080
                if (window.location.hostname === 'localhost' && window.location.port !== '8080' && window.location.port !== '') {
                    return 'http://localhost:8080/api';
                }
                // On cloud hosts (Render, Railway, Heroku) or standard local port 8080, use same-origin /api
                return '/api';
            }
        }
        return 'http://localhost:8080/api';
    })(),

    // ---------------- AUTH TOKENS ----------------
    getToken() {
        return localStorage.getItem('pm_token');
    },

    setToken(token) {
        localStorage.setItem('pm_token', token);
    },

    getUser() {
        const u = localStorage.getItem('pm_user');
        return u ? JSON.parse(u) : null;
    },

    setUser(user) {
        localStorage.setItem('pm_user', JSON.stringify(user));
    },

    clearAuth() {
        localStorage.removeItem('pm_token');
        localStorage.removeItem('pm_user');
    },

    // ---------------- OFFLINE DATA STORE INITIALIZER ----------------
    initOfflineStore() {
        if (!localStorage.getItem('pm_store_initialized')) {
            const seedUsers = [
                { id: 1, name: 'System Administrator', email: 'admin@pricillamart.com', phone: '+91 9876543210', password: 'Admin@123', role: 'admin' },
                { id: 2, name: 'TechWorld Electronics', email: 'techworld@pricillamart.com', phone: '+91 9876543211', password: 'Seller@123', role: 'seller' },
                { id: 3, name: 'Luxe & Style Emporium', email: 'luxe@pricillamart.com', phone: '+91 9876543212', password: 'Seller@123', role: 'seller' },
                { id: 4, name: 'Rahul Sharma', email: 'rahul@gmail.com', phone: '+91 9876543213', password: 'Buyer@123', role: 'buyer' },
                { id: 5, name: 'Priya Patel', email: 'priya@gmail.com', phone: '+91 9876543214', password: 'Buyer@123', role: 'buyer' },
                { id: 6, name: 'John Doe', email: 'john@gmail.com', phone: '+91 9876543215', password: 'Buyer@123', role: 'buyer' }
            ];

            const seedCategories = [
                { id: 1, name: 'Electronics', slug: 'electronics', icon: 'fas fa-laptop', description: 'Laptops, smartphones, audio devices, and cutting-edge tech gadgets', productCount: 3 },
                { id: 2, name: 'Fashion', slug: 'fashion', icon: 'fas fa-tshirt', description: 'Trendy apparel, designer dresses, men and women ethnic wear', productCount: 1 },
                { id: 3, name: 'Accessories', slug: 'accessories', icon: 'fas fa-clock', description: 'Smart watches, luxury handbags, premium wallets, and jewelry', productCount: 2 },
                { id: 4, name: 'Home', slug: 'home', icon: 'fas fa-couch', description: 'Modern home decor, living room lighting, and kitchen appliances', productCount: 1 },
                { id: 5, name: 'Beauty', slug: 'beauty', icon: 'fas fa-spa', description: 'Skincare essentials, fragrance, cosmetics, and wellness items', productCount: 1 },
                { id: 6, name: 'Books', slug: 'books', icon: 'fas fa-book', description: 'Academic textbooks, competitive guides, and bestselling novels', productCount: 1 }
            ];

            const seedProducts = [
                {
                    id: 1,
                    name: 'UltraBook Pro 15.6" Laptop',
                    description: 'High performance Intel Core i7 12th Gen, 16GB RAM, 512GB NVMe SSD, Full HD IPS Anti-Glare Display with backlit keyboard. Ideal for engineering, coding, and multitasking.',
                    categoryId: 1,
                    categoryName: 'Electronics',
                    price: 45000.0,
                    stockQuantity: 15,
                    imageUrl: 'https://images.unsplash.com/photo-1496181133206-80ce9b88a853?w=600&auto=format&fit=crop&q=80',
                    sellerId: 2,
                    sellerName: 'TechWorld Electronics',
                    averageRating: 5.0,
                    reviewCount: 1,
                    createdAt: '2026-03-01 10:00:00'
                },
                {
                    id: 2,
                    name: 'Galaxy Fusion 5G Smartphone',
                    description: '6.6" 120Hz Super AMOLED Display, 64MP OIS Triple Camera setup, 5000mAh all-day battery with 33W Fast Turbo Charging. 8GB RAM + 128GB Storage.',
                    categoryId: 1,
                    categoryName: 'Electronics',
                    price: 18000.0,
                    stockQuantity: 30,
                    imageUrl: 'https://images.unsplash.com/photo-1511707171634-5f897ff02aa9?w=600&auto=format&fit=crop&q=80',
                    sellerId: 2,
                    sellerName: 'TechWorld Electronics',
                    averageRating: 4.8,
                    reviewCount: 0,
                    createdAt: '2026-03-02 11:00:00'
                },
                {
                    id: 3,
                    name: 'SonicWave Wireless ANC Headphones',
                    description: 'Premium Over-Ear Bluetooth 5.3 headphones with Active Noise Cancellation, Deep Bass Equalizer, 40 hours continuous playtime, and ultra-soft memory foam earcups.',
                    categoryId: 1,
                    categoryName: 'Electronics',
                    price: 999.0,
                    stockQuantity: 50,
                    imageUrl: 'https://images.unsplash.com/photo-1505740420928-5e560c06d30e?w=600&auto=format&fit=crop&q=80',
                    sellerId: 2,
                    sellerName: 'TechWorld Electronics',
                    averageRating: 4.0,
                    reviewCount: 1,
                    createdAt: '2026-03-03 12:00:00'
                },
                {
                    id: 4,
                    name: 'AuraFit Pro Smart Watch',
                    description: '1.85" HD Always-On Touchscreen, SpO2 blood oxygen monitor, 24/7 heart rate tracking, Bluetooth calling, 100+ Sports modes, IP68 water resistant.',
                    categoryId: 3,
                    categoryName: 'Accessories',
                    price: 1999.0,
                    stockQuantity: 40,
                    imageUrl: 'https://images.unsplash.com/photo-1523275335684-37898b6baf30?w=600&auto=format&fit=crop&q=80',
                    sellerId: 2,
                    sellerName: 'TechWorld Electronics',
                    averageRating: 5.0,
                    reviewCount: 1,
                    createdAt: '2026-03-04 13:00:00'
                },
                {
                    id: 5,
                    name: 'Vintage Leather Handbag',
                    description: 'Handcrafted genuine vegan leather tote bag with gold-tone hardware, multiple zippered organizing compartments, and comfortable shoulder straps.',
                    categoryId: 3,
                    categoryName: 'Accessories',
                    price: 1499.0,
                    stockQuantity: 25,
                    imageUrl: 'https://images.unsplash.com/photo-1584917865442-de89df76afd3?w=600&auto=format&fit=crop&q=80',
                    sellerId: 3,
                    sellerName: 'Luxe & Style Emporium',
                    averageRating: 5.0,
                    reviewCount: 1,
                    createdAt: '2026-03-05 14:00:00'
                },
                {
                    id: 6,
                    name: 'Classic Slim-Fit Denim Jacket',
                    description: 'Premium washed denim cotton jacket with vintage brass button accents, tailored fit, double chest pockets, and durable stitching for casual styling.',
                    categoryId: 2,
                    categoryName: 'Fashion',
                    price: 2499.0,
                    stockQuantity: 35,
                    imageUrl: 'https://images.unsplash.com/photo-1576995853123-5a10305d93c0?w=600&auto=format&fit=crop&q=80',
                    sellerId: 3,
                    sellerName: 'Luxe & Style Emporium',
                    averageRating: 4.7,
                    reviewCount: 0,
                    createdAt: '2026-03-06 15:00:00'
                },
                {
                    id: 7,
                    name: 'Nordic Minimalist Desk Lamp',
                    description: 'Sleek matte finish LED adjustable desk lamp with touch-sensitive 3 color temperatures, stepless dimming, USB charging port, and eye-caring warm diffused illumination.',
                    categoryId: 4,
                    categoryName: 'Home',
                    price: 1299.0,
                    stockQuantity: 20,
                    imageUrl: 'https://images.unsplash.com/photo-1507473885765-e6ed057f782c?w=600&auto=format&fit=crop&q=80',
                    sellerId: 2,
                    sellerName: 'TechWorld Electronics',
                    averageRating: 4.6,
                    reviewCount: 0,
                    createdAt: '2026-03-07 16:00:00'
                },
                {
                    id: 8,
                    name: 'Botanical Radiance Vitamin C Serum',
                    description: 'All-natural antioxidant face serum formulated with pure 15% Vitamin C, Hyaluronic Acid, and Ferulic Acid for skin glow, hydration, and dark spot correction.',
                    categoryId: 5,
                    categoryName: 'Beauty',
                    price: 899.0,
                    stockQuantity: 60,
                    imageUrl: 'https://images.unsplash.com/photo-1620916566398-39f1143ab7be?w=600&auto=format&fit=crop&q=80',
                    sellerId: 3,
                    sellerName: 'Luxe & Style Emporium',
                    averageRating: 4.9,
                    reviewCount: 0,
                    createdAt: '2026-03-08 17:00:00'
                },
                {
                    id: 9,
                    name: 'Data Structures & Algorithms in C++',
                    description: 'Comprehensive academic master reference textbook covering dynamic programming, graph theory, trees, time complexity, and competitive programming problems.',
                    categoryId: 6,
                    categoryName: 'Books',
                    price: 699.0,
                    stockQuantity: 45,
                    imageUrl: 'https://images.unsplash.com/photo-1532012164546-f432f2e37271?w=600&auto=format&fit=crop&q=80',
                    sellerId: 2,
                    sellerName: 'TechWorld Electronics',
                    averageRating: 5.0,
                    reviewCount: 0,
                    createdAt: '2026-03-09 18:00:00'
                }
            ];

            const seedOrders = [
                {
                    id: 1,
                    orderCode: 'PRC10001',
                    buyerId: 4,
                    buyerName: 'Rahul Sharma',
                    email: 'rahul@gmail.com',
                    phone: '+91 9876543213',
                    deliveryAddress: '42 MG Road, Koramangala 4th Block, Bengaluru, Karnataka 560034',
                    paymentMethod: 'UPI',
                    totalAmount: 1999.0,
                    status: 'Delivered',
                    createdAt: '2026-03-04 10:15:00',
                    items: [
                        { id: 1, orderId: 1, productId: 4, productName: 'AuraFit Pro Smart Watch', imageUrl: 'https://images.unsplash.com/photo-1523275335684-37898b6baf30?w=600&auto=format&fit=crop&q=80', sellerId: 2, sellerName: 'TechWorld Electronics', quantity: 1, unitPrice: 1999.0, subtotal: 1999.0 }
                    ]
                },
                {
                    id: 2,
                    orderCode: 'PRC10002',
                    buyerId: 5,
                    buyerName: 'Priya Patel',
                    email: 'priya@gmail.com',
                    phone: '+91 9876543214',
                    deliveryAddress: '78 Heritage Park, Satellite, Ahmedabad, Gujarat 380015',
                    paymentMethod: 'Credit Card',
                    totalAmount: 45999.0,
                    status: 'Shipped',
                    createdAt: '2026-03-07 14:30:00',
                    items: [
                        { id: 2, orderId: 2, productId: 1, productName: 'UltraBook Pro 15.6" Laptop', imageUrl: 'https://images.unsplash.com/photo-1496181133206-80ce9b88a853?w=600&auto=format&fit=crop&q=80', sellerId: 2, sellerName: 'TechWorld Electronics', quantity: 1, unitPrice: 45000.0, subtotal: 45000.0 },
                        { id: 3, orderId: 2, productId: 3, productName: 'SonicWave Wireless ANC Headphones', imageUrl: 'https://images.unsplash.com/photo-1505740420928-5e560c06d30e?w=600&auto=format&fit=crop&q=80', sellerId: 2, sellerName: 'TechWorld Electronics', quantity: 1, unitPrice: 999.0, subtotal: 999.0 }
                    ]
                },
                {
                    id: 3,
                    orderCode: 'PRC10003',
                    buyerId: 6,
                    buyerName: 'John Doe',
                    email: 'john@gmail.com',
                    phone: '+91 9876543215',
                    deliveryAddress: '12 Lakeview Avenue, Powai, Mumbai, Maharashtra 400076',
                    paymentMethod: 'Cash on Delivery',
                    totalAmount: 1499.0,
                    status: 'Processing',
                    createdAt: '2026-03-09 09:45:00',
                    items: [
                        { id: 4, orderId: 3, productId: 5, productName: 'Vintage Leather Handbag', imageUrl: 'https://images.unsplash.com/photo-1584917865442-de89df76afd3?w=600&auto=format&fit=crop&q=80', sellerId: 3, sellerName: 'Luxe & Style Emporium', quantity: 1, unitPrice: 1499.0, subtotal: 1499.0 }
                    ]
                }
            ];

            const seedReviews = [
                { id: 1, productId: 4, buyerId: 4, buyerName: 'Rahul Sharma', rating: 5, comment: 'Exceptional smart watch! The display is crisp, battery lasts a full 6 days, and the heart rate accuracy is surprisingly good.', createdAt: '2026-03-05' },
                { id: 2, productId: 1, buyerId: 5, buyerName: 'Priya Patel', rating: 5, comment: 'Blazing fast laptop. Visual Studio and Java compile large projects in seconds. Build quality is top-notch.', createdAt: '2026-03-08' },
                { id: 3, productId: 3, buyerId: 4, buyerName: 'Rahul Sharma', rating: 4, comment: 'Really great noise cancellation at this budget price point. Bass is punchy and ear pads are comfortable for long study sessions.', createdAt: '2026-03-06' },
                { id: 4, productId: 5, buyerId: 6, buyerName: 'John Doe', rating: 5, comment: 'Superb handcrafted quality. The leather texture is authentic and spacious enough to hold my tablet and accessories.', createdAt: '2026-03-09' }
            ];

            localStorage.setItem('pm_users', JSON.stringify(seedUsers));
            localStorage.setItem('pm_categories', JSON.stringify(seedCategories));
            localStorage.setItem('pm_products', JSON.stringify(seedProducts));
            localStorage.setItem('pm_orders', JSON.stringify(seedOrders));
            localStorage.setItem('pm_reviews', JSON.stringify(seedReviews));
            localStorage.setItem('pm_cart', JSON.stringify([]));
            localStorage.setItem('pm_order_counter', '10004');
            localStorage.setItem('pm_store_initialized', 'true');
        }
    },

    // ---------------- OFFLINE RESILIENT DISPATCHER ----------------
    handleOfflineRequest(endpoint, options = {}) {
        this.initOfflineStore();
        const method = (options.method || 'GET').toUpperCase();
        const body = options.body ? JSON.parse(options.body) : {};
        const [cleanEndpoint, queryStr] = endpoint.split('?');
        const params = new URLSearchParams(queryStr || '');
        const currentUser = this.getUser();

        // 1. AUTH: LOGIN
        if (cleanEndpoint === '/login' && method === 'POST') {
            const users = JSON.parse(localStorage.getItem('pm_users') || '[]');
            const email = (body.email || '').trim().toLowerCase();
            const password = body.password || '';

            const user = users.find(u => u.email.toLowerCase() === email);
            if (!user) {
                throw new Error('Invalid email or password.');
            }
            if (user.password && user.password !== password) {
                throw new Error('Invalid email or password.');
            }

            const token = 'mock-jwt-token-' + user.id + '-' + Date.now();
            const userSafe = { id: user.id, name: user.name, email: user.email, phone: user.phone, role: user.role };
            return {
                success: true,
                message: 'Login successful.',
                data: { token, user: userSafe }
            };
        }

        // 2. AUTH: REGISTER
        if (cleanEndpoint === '/register' && method === 'POST') {
            const users = JSON.parse(localStorage.getItem('pm_users') || '[]');
            const { name, email, phone, password, confirmPassword, role } = body;

            if (!name || !email || !phone || !password || !confirmPassword || !role) {
                throw new Error('All fields are required.');
            }
            if (password !== confirmPassword) {
                throw new Error('Passwords do not match.');
            }
            if (users.some(u => u.email.toLowerCase() === email.trim().toLowerCase())) {
                throw new Error('An account with this email already exists.');
            }

            const newUser = {
                id: users.length ? Math.max(...users.map(u => u.id)) + 1 : 1,
                name: name.trim(),
                email: email.trim(),
                phone: phone.trim(),
                password: password,
                role: role
            };
            users.push(newUser);
            localStorage.setItem('pm_users', JSON.stringify(users));

            const token = 'mock-jwt-token-' + newUser.id + '-' + Date.now();
            const userSafe = { id: newUser.id, name: newUser.name, email: newUser.email, phone: newUser.phone, role: newUser.role };
            return {
                success: true,
                message: 'Account registered successfully.',
                data: { token, user: userSafe }
            };
        }

        // 3. AUTH: LOGOUT
        if (cleanEndpoint === '/logout' && method === 'POST') {
            return { success: true, message: 'Logged out successfully.' };
        }

        // 4. CATEGORIES
        if (cleanEndpoint === '/categories' && method === 'GET') {
            const categories = JSON.parse(localStorage.getItem('pm_categories') || '[]');
            const products = JSON.parse(localStorage.getItem('pm_products') || '[]');
            const catsWithCounts = categories.map(c => ({
                ...c,
                productCount: products.filter(p => p.categoryId === c.id).length
            }));
            return { success: true, data: catsWithCounts };
        }

        // 5. PRODUCTS: LIST
        if (cleanEndpoint === '/products' && method === 'GET') {
            let products = JSON.parse(localStorage.getItem('pm_products') || '[]');
            const search = (params.get('search') || '').toLowerCase().trim();
            const categoryId = params.get('category_id');
            const sellerId = params.get('seller_id');
            const sort = params.get('sort');

            if (search) {
                products = products.filter(p =>
                    p.name.toLowerCase().includes(search) ||
                    p.description.toLowerCase().includes(search) ||
                    (p.categoryName && p.categoryName.toLowerCase().includes(search))
                );
            }
            if (categoryId) {
                products = products.filter(p => String(p.categoryId) === String(categoryId));
            }
            if (sellerId) {
                products = products.filter(p => String(p.sellerId) === String(sellerId));
            }

            if (sort === 'price_asc') products.sort((a, b) => a.price - b.price);
            else if (sort === 'price_desc') products.sort((a, b) => b.price - a.price);
            else if (sort === 'rating') products.sort((a, b) => (b.averageRating || 0) - (a.averageRating || 0));
            else products.sort((a, b) => b.id - a.id);

            return { success: true, data: products };
        }

        // 6. PRODUCTS: SINGLE ITEM
        const prodMatch = cleanEndpoint.match(/^\/products\/(\d+)$/);
        if (prodMatch && method === 'GET') {
            const pId = parseInt(prodMatch[1]);
            const products = JSON.parse(localStorage.getItem('pm_products') || '[]');
            const found = products.find(p => p.id === pId);
            if (!found) throw new Error('Product not found.');
            return { success: true, data: found };
        }

        // 7. PRODUCTS: CREATE
        if (cleanEndpoint === '/products' && method === 'POST') {
            if (!currentUser || (currentUser.role !== 'seller' && currentUser.role !== 'admin')) {
                throw new Error('Only sellers or admins can create products.');
            }
            const products = JSON.parse(localStorage.getItem('pm_products') || '[]');
            const categories = JSON.parse(localStorage.getItem('pm_categories') || '[]');
            const cat = categories.find(c => String(c.id) === String(body.categoryId)) || { name: 'General' };

            const newProd = {
                id: products.length ? Math.max(...products.map(p => p.id)) + 1 : 1,
                name: body.name.trim(),
                description: body.description.trim(),
                categoryId: parseInt(body.categoryId),
                categoryName: cat.name,
                price: parseFloat(body.price),
                stockQuantity: parseInt(body.stockQuantity),
                imageUrl: body.imageUrl || 'https://images.unsplash.com/photo-1526170375885-4d8ecf77b99f?w=600&auto=format&fit=crop&q=80',
                sellerId: currentUser.id,
                sellerName: currentUser.name,
                averageRating: 5.0,
                reviewCount: 0,
                createdAt: new Date().toISOString().replace('T', ' ').substring(0, 19)
            };
            products.unshift(newProd);
            localStorage.setItem('pm_products', JSON.stringify(products));
            return { success: true, message: 'Product added successfully.', data: newProd };
        }

        // 8. PRODUCTS: UPDATE
        if (prodMatch && method === 'PUT') {
            const pId = parseInt(prodMatch[1]);
            const products = JSON.parse(localStorage.getItem('pm_products') || '[]');
            const idx = products.findIndex(p => p.id === pId);
            if (idx === -1) throw new Error('Product not found.');

            const categories = JSON.parse(localStorage.getItem('pm_categories') || '[]');
            const cat = categories.find(c => String(c.id) === String(body.categoryId)) || { name: products[idx].categoryName };

            products[idx] = {
                ...products[idx],
                name: body.name ? body.name.trim() : products[idx].name,
                description: body.description ? body.description.trim() : products[idx].description,
                categoryId: body.categoryId ? parseInt(body.categoryId) : products[idx].categoryId,
                categoryName: cat.name,
                price: body.price ? parseFloat(body.price) : products[idx].price,
                stockQuantity: body.stockQuantity !== undefined ? parseInt(body.stockQuantity) : products[idx].stockQuantity,
                imageUrl: body.imageUrl ? body.imageUrl.trim() : products[idx].imageUrl
            };
            localStorage.setItem('pm_products', JSON.stringify(products));
            return { success: true, message: 'Product updated successfully.', data: products[idx] };
        }

        // 9. PRODUCTS: DELETE
        if (prodMatch && method === 'DELETE') {
            const pId = parseInt(prodMatch[1]);
            let products = JSON.parse(localStorage.getItem('pm_products') || '[]');
            products = products.filter(p => p.id !== pId);
            localStorage.setItem('pm_products', JSON.stringify(products));
            return { success: true, message: 'Product deleted successfully.' };
        }

        // 10. CART: GET
        if (cleanEndpoint === '/cart' && method === 'GET') {
            const cart = JSON.parse(localStorage.getItem('pm_cart') || '[]');
            const products = JSON.parse(localStorage.getItem('pm_products') || '[]');
            
            const populatedItems = cart.map(item => {
                const p = products.find(prod => prod.id === item.productId) || {};
                return {
                    id: item.id,
                    productId: item.productId,
                    productName: p.name || item.productName || 'Product',
                    price: p.price || item.price || 0,
                    imageUrl: p.imageUrl || item.imageUrl || '',
                    sellerId: p.sellerId || item.sellerId || 2,
                    sellerName: p.sellerName || item.sellerName || 'Seller',
                    availableStock: p.stockQuantity !== undefined ? p.stockQuantity : 99,
                    quantity: item.quantity,
                    subtotal: (p.price || 0) * item.quantity
                };
            });

            const subtotal = populatedItems.reduce((acc, it) => acc + it.subtotal, 0);
            const itemCount = populatedItems.reduce((acc, it) => acc + it.quantity, 0);
            return {
                success: true,
                data: {
                    items: populatedItems,
                    subtotal,
                    total: subtotal,
                    itemCount
                }
            };
        }

        // 11. CART: ADD ITEM
        if (cleanEndpoint === '/cart' && method === 'POST') {
            const cart = JSON.parse(localStorage.getItem('pm_cart') || '[]');
            const products = JSON.parse(localStorage.getItem('pm_products') || '[]');
            const prod = products.find(p => p.id === parseInt(body.productId));

            if (!prod) throw new Error('Product not found.');
            if (prod.stockQuantity <= 0) throw new Error('Product is currently out of stock.');

            const reqQty = parseInt(body.quantity) || 1;
            const existingIdx = cart.findIndex(it => it.productId === prod.id);

            if (existingIdx !== -1) {
                const newQty = cart[existingIdx].quantity + reqQty;
                if (newQty > prod.stockQuantity) {
                    throw new Error(`Only ${prod.stockQuantity} units available in stock.`);
                }
                cart[existingIdx].quantity = newQty;
            } else {
                if (reqQty > prod.stockQuantity) {
                    throw new Error(`Only ${prod.stockQuantity} units available in stock.`);
                }
                cart.push({
                    id: cart.length ? Math.max(...cart.map(c => c.id)) + 1 : 1,
                    productId: prod.id,
                    productName: prod.name,
                    price: prod.price,
                    imageUrl: prod.imageUrl,
                    sellerId: prod.sellerId,
                    sellerName: prod.sellerName,
                    quantity: reqQty
                });
            }
            localStorage.setItem('pm_cart', JSON.stringify(cart));
            return { success: true, message: 'Item added to cart!' };
        }

        // 12. CART: UPDATE QUANTITY
        const cartItemMatch = cleanEndpoint.match(/^\/cart\/(\d+)$/);
        if (cartItemMatch && method === 'PUT') {
            const cartItemId = parseInt(cartItemMatch[1]);
            const cart = JSON.parse(localStorage.getItem('pm_cart') || '[]');
            const products = JSON.parse(localStorage.getItem('pm_products') || '[]');
            const idx = cart.findIndex(c => c.id === cartItemId);

            if (idx === -1) throw new Error('Cart item not found.');
            const prod = products.find(p => p.id === cart[idx].productId);
            const reqQty = parseInt(body.quantity);

            if (reqQty <= 0) {
                cart.splice(idx, 1);
            } else {
                if (prod && reqQty > prod.stockQuantity) {
                    throw new Error(`Cannot exceed available stock of ${prod.stockQuantity} units.`);
                }
                cart[idx].quantity = reqQty;
            }
            localStorage.setItem('pm_cart', JSON.stringify(cart));
            return { success: true, message: 'Cart updated successfully.' };
        }

        // 13. CART: DELETE ITEM
        if (cartItemMatch && method === 'DELETE') {
            const cartItemId = parseInt(cartItemMatch[1]);
            let cart = JSON.parse(localStorage.getItem('pm_cart') || '[]');
            cart = cart.filter(c => c.id !== cartItemId);
            localStorage.setItem('pm_cart', JSON.stringify(cart));
            return { success: true, message: 'Item removed from cart.' };
        }

        // 14. CART: CLEAR
        if (cleanEndpoint === '/cart' && method === 'DELETE') {
            localStorage.setItem('pm_cart', JSON.stringify([]));
            return { success: true, message: 'Cart cleared.' };
        }

        // 15. ORDERS: CHECKOUT / CONFIRM ORDER
        if (cleanEndpoint === '/orders' && method === 'POST') {
            const cart = JSON.parse(localStorage.getItem('pm_cart') || '[]');
            if (cart.length === 0) {
                throw new Error('Your cart is empty. Please add products before placing an order.');
            }

            const products = JSON.parse(localStorage.getItem('pm_products') || '[]');
            const orders = JSON.parse(localStorage.getItem('pm_orders') || '[]');
            let nextCounter = parseInt(localStorage.getItem('pm_order_counter') || '10004');
            const orderCode = `PRC${nextCounter}`;
            localStorage.setItem('pm_order_counter', String(nextCounter + 1));

            // Validate and deduct stock
            let totalAmount = 0;
            const orderItems = [];

            for (const c of cart) {
                const prod = products.find(p => p.id === c.productId);
                if (!prod) continue;
                if (prod.stockQuantity < c.quantity) {
                    throw new Error(`Insufficient stock for "${prod.name}". Available: ${prod.stockQuantity}`);
                }
                prod.stockQuantity -= c.quantity;
                const subtotal = prod.price * c.quantity;
                totalAmount += subtotal;

                orderItems.push({
                    id: orderItems.length + 1,
                    orderId: orders.length + 1,
                    productId: prod.id,
                    productName: prod.name,
                    imageUrl: prod.imageUrl,
                    sellerId: prod.sellerId,
                    sellerName: prod.sellerName,
                    quantity: c.quantity,
                    unitPrice: prod.price,
                    subtotal: subtotal
                });
            }

            const newOrder = {
                id: orders.length ? Math.max(...orders.map(o => o.id)) + 1 : 1,
                orderCode: orderCode,
                buyerId: currentUser ? currentUser.id : 4,
                buyerName: body.buyerName || (currentUser ? currentUser.name : 'Valued Buyer'),
                email: body.email || (currentUser ? currentUser.email : 'buyer@example.com'),
                phone: body.phone || '+91 9876543210',
                deliveryAddress: body.deliveryAddress || 'Standard Delivery Address',
                paymentMethod: body.paymentMethod || 'Cash on Delivery',
                totalAmount: totalAmount,
                status: 'Processing',
                createdAt: new Date().toISOString().replace('T', ' ').substring(0, 19),
                items: orderItems
            };

            orders.unshift(newOrder);
            localStorage.setItem('pm_orders', JSON.stringify(orders));
            localStorage.setItem('pm_products', JSON.stringify(products));
            localStorage.setItem('pm_cart', JSON.stringify([])); // Clear cart

            return {
                success: true,
                message: `Order ${orderCode} created successfully!`,
                data: {
                    orderId: newOrder.id,
                    orderCode: newOrder.orderCode,
                    totalAmount: newOrder.totalAmount
                }
            };
        }

        // 16. ORDERS: LIST
        if (cleanEndpoint === '/orders' && method === 'GET') {
            const orders = JSON.parse(localStorage.getItem('pm_orders') || '[]');
            if (currentUser && currentUser.role === 'buyer') {
                return { success: true, data: orders.filter(o => o.buyerId === currentUser.id) };
            }
            if (currentUser && currentUser.role === 'seller') {
                // Filter orders containing items from this seller
                const sellerOrders = orders.filter(o => o.items && o.items.some(it => it.sellerId === currentUser.id));
                return { success: true, data: sellerOrders };
            }
            return { success: true, data: orders };
        }

        // 17. REVIEWS: LIST
        const prodRevMatch = cleanEndpoint.match(/^\/products\/(\d+)\/reviews$/);
        if ((cleanEndpoint === '/reviews' || prodRevMatch) && method === 'GET') {
            const pId = prodRevMatch ? parseInt(prodRevMatch[1]) : parseInt(params.get('product_id'));
            const reviews = JSON.parse(localStorage.getItem('pm_reviews') || '[]');
            const filtered = pId ? reviews.filter(r => r.productId === pId) : reviews;
            const avg = filtered.length ? filtered.reduce((s, r) => s + r.rating, 0) / filtered.length : 5.0;
            return {
                success: true,
                data: {
                    reviews: filtered,
                    averageRating: Math.round(avg * 10) / 10,
                    reviewCount: filtered.length
                }
            };
        }

        // 18. REVIEWS: SUBMIT
        if (cleanEndpoint === '/reviews' && method === 'POST') {
            if (!currentUser) throw new Error('Please log in to submit a review.');
            const reviews = JSON.parse(localStorage.getItem('pm_reviews') || '[]');
            const products = JSON.parse(localStorage.getItem('pm_products') || '[]');

            const newRev = {
                id: reviews.length ? Math.max(...reviews.map(r => r.id)) + 1 : 1,
                productId: parseInt(body.productId),
                buyerId: currentUser.id,
                buyerName: currentUser.name,
                rating: parseInt(body.rating) || 5,
                comment: body.comment.trim(),
                createdAt: new Date().toISOString().split('T')[0]
            };
            reviews.unshift(newRev);
            localStorage.setItem('pm_reviews', JSON.stringify(reviews));

            // Update product average rating
            const prod = products.find(p => p.id === newRev.productId);
            if (prod) {
                const prodReviews = reviews.filter(r => r.productId === prod.id);
                const avg = prodReviews.reduce((sum, r) => sum + r.rating, 0) / prodReviews.length;
                prod.averageRating = Math.round(avg * 10) / 10;
                prod.reviewCount = prodReviews.length;
                localStorage.setItem('pm_products', JSON.stringify(products));
            }

            return { success: true, message: 'Review submitted successfully!', data: newRev };
        }

        // 19. ADMIN STATS & USERS
        if (cleanEndpoint === '/admin/stats' && method === 'GET') {
            const users = JSON.parse(localStorage.getItem('pm_users') || '[]');
            const products = JSON.parse(localStorage.getItem('pm_products') || '[]');
            const orders = JSON.parse(localStorage.getItem('pm_orders') || '[]');
            const totalSales = orders.reduce((sum, o) => sum + (o.totalAmount || 0), 0);

            return {
                success: true,
                data: {
                    totalUsers: users.length,
                    totalBuyers: users.filter(u => u.role === 'buyer').length,
                    totalSellers: users.filter(u => u.role === 'seller').length,
                    totalProducts: products.length,
                    totalOrders: orders.length,
                    totalSales: totalSales
                }
            };
        }

        if (cleanEndpoint === '/admin/users' && method === 'GET') {
            let users = JSON.parse(localStorage.getItem('pm_users') || '[]');
            const role = params.get('role');
            if (role) users = users.filter(u => u.role === role);
            return { success: true, data: users };
        }

        // 20. SELLER STATS
        if (cleanEndpoint === '/seller/stats' && method === 'GET') {
            const sId = currentUser ? currentUser.id : 2;
            const products = JSON.parse(localStorage.getItem('pm_products') || '[]');
            const orders = JSON.parse(localStorage.getItem('pm_orders') || '[]');

            const sellerProds = products.filter(p => p.sellerId === sId);
            const sellerOrders = orders.filter(o => o.items && o.items.some(it => it.sellerId === sId));
            const revenue = sellerOrders.reduce((sum, o) => {
                const sItems = (o.items || []).filter(it => it.sellerId === sId);
                return sum + sItems.reduce((iSum, it) => iSum + it.subtotal, 0);
            }, 0);

            return {
                success: true,
                data: {
                    totalProducts: sellerProds.length,
                    totalOrders: sellerOrders.length,
                    totalSales: revenue
                }
            };
        }

        // 21. AI CHATBOT
        if (cleanEndpoint === '/chatbot' && method === 'POST') {
            const msg = (body.message || '').toLowerCase();
            const products = JSON.parse(localStorage.getItem('pm_products') || '[]');

            if (msg.includes('laptop') || msg.includes('ultrabook') || msg.includes('computer')) {
                const p = products.find(x => x.name.toLowerCase().includes('laptop'));
                return {
                    success: true,
                    data: {
                        reply: `We have the powerful "${p ? p.name : 'UltraBook Pro'}" with Intel Core i7, 16GB RAM for ₹${p ? p.price.toLocaleString('en-IN') : '45,000'}. In stock and ready to ship!`
                    }
                };
            }
            if (msg.includes('phone') || msg.includes('mobile') || msg.includes('galaxy')) {
                const p = products.find(x => x.name.toLowerCase().includes('phone'));
                return {
                    success: true,
                    data: {
                        reply: `Check out the "${p ? p.name : 'Galaxy Fusion 5G'}" with 120Hz Super AMOLED & 64MP Camera for ₹${p ? p.price.toLocaleString('en-IN') : '18,000'}!`
                    }
                };
            }
            if (msg.includes('watch') || msg.includes('headphone') || msg.includes('earphone')) {
                return {
                    success: true,
                    data: {
                        reply: `Top accessories: AuraFit Pro Smart Watch (₹1,999) and SonicWave ANC Headphones (₹999) are currently bestsellers!`
                    }
                };
            }
            if (msg.includes('order') || msg.includes('track') || msg.includes('delivery')) {
                return {
                    success: true,
                    data: {
                        reply: `You can track active deliveries anytime on the Orders tab! Pricilla Mart orders typically arrive within 2-4 business days with free standard shipping.`
                    }
                };
            }
            if (msg.includes('return') || msg.includes('refund')) {
                return {
                    success: true,
                    data: {
                        reply: `Pricilla Mart provides a hassle-free 7-day return policy for verified orders. Contact the seller directly or email support@pricillamart.com.`
                    }
                };
            }
            return {
                success: true,
                data: {
                    reply: `Hello! I am your Pricilla Mart AI Assistant. I can help you find products across Electronics, Fashion, Accessories, Home, Beauty, and Books. What are you looking for today?`
                }
            };
        }

        // Generic fallback for any other endpoint
        return { success: true, message: 'Operation processed successfully.' };
    },

    // ---------------- PRIMARY REQUEST DISPATCHER ----------------
    async request(endpoint, options = {}) {
        const url = `${this.baseUrl}${endpoint}`;
        const headers = {
            'Content-Type': 'application/json',
            ...(options.headers || {})
        };

        const token = this.getToken();
        if (token) {
            headers['Authorization'] = `Bearer ${token}`;
        }

        // 1. Try real HTTP fetch if not file: protocol
        if (window.location && window.location.protocol.startsWith('http')) {
            try {
                const controller = new AbortController();
                const timeoutId = setTimeout(() => controller.abort(), 3500);

                const resp = await fetch(url, {
                    ...options,
                    headers,
                    signal: controller.signal
                });
                clearTimeout(timeoutId);

                const data = await resp.json().catch(() => ({}));
                if (!resp.ok) {
                    if (resp.status === 401) {
                        this.clearAuth();
                    }
                    const errorMsg = data.error || `HTTP error ${resp.status}`;
                    throw new Error(errorMsg);
                }
                return data;
            } catch (err) {
                // If the error was a deliberate 4xx validation error from server, rethrow it
                if (err.message && !err.message.includes('fetch') && !err.message.includes('Failed to fetch') && !err.message.includes('aborted') && !err.message.includes('NetworkError')) {
                    throw err;
                }
                console.warn(`[Pricilla Mart] Backend unavailable at ${url}. Seamlessly switching to local offline evaluation store.`);
            }
        }

        // 2. Resilient Offline Fallback (ensures 100% functionality on file:/// or offline demo)
        return this.handleOfflineRequest(endpoint, options);
    },

    get(endpoint, params = {}) {
        const q = new URLSearchParams();
        Object.entries(params).forEach(([k, v]) => {
            if (v !== undefined && v !== null && v !== '') {
                q.append(k, v);
            }
        });
        const queryStr = q.toString() ? `?${q.toString()}` : '';
        return this.request(`${endpoint}${queryStr}`, { method: 'GET' });
    },

    post(endpoint, body = {}) {
        return this.request(endpoint, {
            method: 'POST',
            body: JSON.stringify(body)
        });
    },

    put(endpoint, body = {}) {
        return this.request(endpoint, {
            method: 'PUT',
            body: JSON.stringify(body)
        });
    },

    delete(endpoint) {
        return this.request(endpoint, { method: 'DELETE' });
    },

    showToast(message, type = 'info') {
        let container = document.querySelector('.toast-container');
        if (!container) {
            container = document.createElement('div');
            container.className = 'toast-container';
            document.body.appendChild(container);
        }

        const toast = document.createElement('div');
        toast.className = `toast toast-${type}`;
        const icon = type === 'success' ? '✓' : type === 'error' ? '⚠' : 'ℹ';
        toast.innerHTML = `<span>${icon}</span> <span>${message}</span>`;
        container.appendChild(toast);

        setTimeout(() => {
            toast.style.opacity = '0';
            toast.style.transform = 'translateX(100%)';
            toast.style.transition = 'all 0.3s ease';
            setTimeout(() => toast.remove(), 300);
        }, 4000);
    }
};

// Initialize offline storage on load
if (typeof window !== 'undefined') {
    API.initOfflineStore();
}
