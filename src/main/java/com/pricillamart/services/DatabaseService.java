package com.pricillamart.services;

import com.pricillamart.models.*;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;

import jakarta.annotation.PostConstruct;
import java.io.File;
import java.sql.*;
import java.util.*;

@Service
public class DatabaseService {

    @Value("${app.db.path:database/pricillamart.db}")
    private String dbPath;

    private String connectionUrl;

    @PostConstruct
    public void init() {
        File dbFile = new File(dbPath);
        if (!dbFile.isAbsolute()) {
            dbFile = new File(System.getProperty("user.dir"), dbPath);
        }
        if (!dbFile.getParentFile().exists()) {
            dbFile.getParentFile().mkdirs();
        }
        connectionUrl = "jdbc:sqlite:" + dbFile.getAbsolutePath();
        System.out.println("[DatabaseService] Connected to SQLite database at: " + connectionUrl);

        try (Connection conn = getConnection()) {
            try (Statement stmt = conn.createStatement()) {
                stmt.execute("PRAGMA foreign_keys = ON;");
            }
        } catch (SQLException e) {
            System.err.println("[DatabaseService] Error verifying database: " + e.getMessage());
        }
    }

    public Connection getConnection() throws SQLException {
        Connection conn = DriverManager.getConnection(connectionUrl);
        try (Statement stmt = conn.createStatement()) {
            stmt.execute("PRAGMA foreign_keys = ON;");
        }
        return conn;
    }

    // ---------------- USER OPERATIONS ----------------
    public Optional<User> findUserByEmail(String email) {
        String sql = "SELECT id, name, email, phone, password_hash, role, created_at FROM users WHERE email = ?;";
        try (Connection conn = getConnection();
             PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setString(1, email);
            try (ResultSet rs = ps.executeQuery()) {
                if (rs.next()) {
                    return Optional.of(new User(
                        rs.getInt("id"),
                        rs.getString("name"),
                        rs.getString("email"),
                        rs.getString("phone"),
                        rs.getString("password_hash"),
                        rs.getString("role"),
                        rs.getString("created_at")
                    ));
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return Optional.empty();
    }

    public Optional<User> findUserById(int id) {
        String sql = "SELECT id, name, email, phone, password_hash, role, created_at FROM users WHERE id = ?;";
        try (Connection conn = getConnection();
             PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setInt(1, id);
            try (ResultSet rs = ps.executeQuery()) {
                if (rs.next()) {
                    return Optional.of(new User(
                        rs.getInt("id"),
                        rs.getString("name"),
                        rs.getString("email"),
                        rs.getString("phone"),
                        rs.getString("password_hash"),
                        rs.getString("role"),
                        rs.getString("created_at")
                    ));
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return Optional.empty();
    }

    public int createUser(User user) throws SQLException {
        String sql = "INSERT INTO users (name, email, phone, password_hash, role) VALUES (?, ?, ?, ?, ?);";
        try (Connection conn = getConnection();
             PreparedStatement ps = conn.prepareStatement(sql, Statement.RETURN_GENERATED_KEYS)) {
            ps.setString(1, user.getName());
            ps.setString(2, user.getEmail());
            ps.setString(3, user.getPhone());
            ps.setString(4, user.getPasswordHash());
            ps.setString(5, user.getRole());
            ps.executeUpdate();
            try (ResultSet rs = ps.getGeneratedKeys()) {
                if (rs.next()) {
                    return rs.getInt(1);
                }
            }
        }
        return 0;
    }

    public List<User> getAllUsers(String roleFilter) {
        List<User> list = new ArrayList<>();
        String sql = "SELECT id, name, email, phone, password_hash, role, created_at FROM users";
        if (roleFilter != null && !roleFilter.isEmpty() && !roleFilter.equalsIgnoreCase("all")) {
            sql += " WHERE role = ?";
        }
        sql += " ORDER BY id DESC;";

        try (Connection conn = getConnection();
             PreparedStatement ps = conn.prepareStatement(sql)) {
            if (roleFilter != null && !roleFilter.isEmpty() && !roleFilter.equalsIgnoreCase("all")) {
                ps.setString(1, roleFilter);
            }
            try (ResultSet rs = ps.executeQuery()) {
                while (rs.next()) {
                    list.add(new User(
                        rs.getInt("id"),
                        rs.getString("name"),
                        rs.getString("email"),
                        rs.getString("phone"),
                        rs.getString("password_hash"),
                        rs.getString("role"),
                        rs.getString("created_at")
                    ));
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return list;
    }

    // ---------------- CATEGORY OPERATIONS ----------------
    public List<Category> getAllCategories() {
        List<Category> list = new ArrayList<>();
        String sql = "SELECT c.id, c.name, c.slug, c.icon, c.description, COUNT(p.id) as product_count " +
                     "FROM categories c LEFT JOIN products p ON c.id = p.category_id " +
                     "GROUP BY c.id ORDER BY c.id ASC;";
        try (Connection conn = getConnection();
             Statement stmt = conn.createStatement();
             ResultSet rs = stmt.executeQuery(sql)) {
            while (rs.next()) {
                list.add(new Category(
                    rs.getInt("id"),
                    rs.getString("name"),
                    rs.getString("slug"),
                    rs.getString("icon"),
                    rs.getString("description"),
                    rs.getInt("product_count")
                ));
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return list;
    }

    // ---------------- PRODUCT OPERATIONS ----------------
    public List<Product> getProducts(String query, int categoryId, int sellerId, String sortBy, double minPrice, double maxPrice) {
        List<Product> list = new ArrayList<>();
        StringBuilder sql = new StringBuilder();
        sql.append("SELECT p.id, p.name, p.description, p.category_id, c.name as category_name, ")
           .append("p.price, p.stock_quantity, p.image_url, p.seller_id, u.name as seller_name, ")
           .append("p.created_at, IFNULL(AVG(r.rating), 0.0) as avg_rating, COUNT(r.id) as rcount ")
           .append("FROM products p ")
           .append("JOIN categories c ON p.category_id = c.id ")
           .append("JOIN users u ON p.seller_id = u.id ")
           .append("LEFT JOIN reviews r ON p.id = r.product_id ")
           .append("WHERE 1=1 ");

        List<Object> params = new ArrayList<>();
        if (categoryId > 0) {
            sql.append(" AND p.category_id = ? ");
            params.add(categoryId);
        }
        if (sellerId > 0) {
            sql.append(" AND p.seller_id = ? ");
            params.add(sellerId);
        }
        if (minPrice > 0.0) {
            sql.append(" AND p.price >= ? ");
            params.add(minPrice);
        }
        if (maxPrice > 0.0) {
            sql.append(" AND p.price <= ? ");
            params.add(maxPrice);
        }
        if (query != null && !query.trim().isEmpty()) {
            sql.append(" AND (p.name LIKE ? OR p.description LIKE ? OR c.name LIKE ?) ");
            String like = "%" + query.trim() + "%";
            params.add(like);
            params.add(like);
            params.add(like);
        }

        sql.append(" GROUP BY p.id ");

        if ("price_asc".equalsIgnoreCase(sortBy)) {
            sql.append(" ORDER BY p.price ASC");
        } else if ("price_desc".equalsIgnoreCase(sortBy)) {
            sql.append(" ORDER BY p.price DESC");
        } else if ("rating".equalsIgnoreCase(sortBy)) {
            sql.append(" ORDER BY avg_rating DESC");
        } else {
            sql.append(" ORDER BY p.id DESC");
        }

        try (Connection conn = getConnection();
             PreparedStatement ps = conn.prepareStatement(sql.toString())) {
            for (int i = 0; i < params.size(); i++) {
                ps.setObject(i + 1, params.get(i));
            }
            try (ResultSet rs = ps.executeQuery()) {
                while (rs.next()) {
                    Product p = new Product();
                    p.setId(rs.getInt("id"));
                    p.setName(rs.getString("name"));
                    p.setDescription(rs.getString("description"));
                    p.setCategoryId(rs.getInt("category_id"));
                    p.setCategoryName(rs.getString("category_name"));
                    p.setPrice(rs.getDouble("price"));
                    p.setStockQuantity(rs.getInt("stock_quantity"));
                    p.setImageUrl(rs.getString("image_url"));
                    p.setSellerId(rs.getInt("seller_id"));
                    p.setSellerName(rs.getString("seller_name"));
                    p.setCreatedAt(rs.getString("created_at"));
                    p.setAverageRating(rs.getDouble("avg_rating"));
                    p.setReviewCount(rs.getInt("rcount"));
                    list.add(p);
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return list;
    }

    public Optional<Product> getProductById(int id) {
        String sql = "SELECT p.id, p.name, p.description, p.category_id, c.name as category_name, " +
                     "p.price, p.stock_quantity, p.image_url, p.seller_id, u.name as seller_name, " +
                     "p.created_at, IFNULL(AVG(r.rating), 0.0) as avg_rating, COUNT(r.id) as rcount " +
                     "FROM products p " +
                     "JOIN categories c ON p.category_id = c.id " +
                     "JOIN users u ON p.seller_id = u.id " +
                     "LEFT JOIN reviews r ON p.id = r.product_id " +
                     "WHERE p.id = ? GROUP BY p.id;";
        try (Connection conn = getConnection();
             PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setInt(1, id);
            try (ResultSet rs = ps.executeQuery()) {
                if (rs.next()) {
                    Product p = new Product();
                    p.setId(rs.getInt("id"));
                    p.setName(rs.getString("name"));
                    p.setDescription(rs.getString("description"));
                    p.setCategoryId(rs.getInt("category_id"));
                    p.setCategoryName(rs.getString("category_name"));
                    p.setPrice(rs.getDouble("price"));
                    p.setStockQuantity(rs.getInt("stock_quantity"));
                    p.setImageUrl(rs.getString("image_url"));
                    p.setSellerId(rs.getInt("seller_id"));
                    p.setSellerName(rs.getString("seller_name"));
                    p.setCreatedAt(rs.getString("created_at"));
                    p.setAverageRating(rs.getDouble("avg_rating"));
                    p.setReviewCount(rs.getInt("rcount"));
                    return Optional.of(p);
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return Optional.empty();
    }

    public int createProduct(Product product) throws SQLException {
        String sql = "INSERT INTO products (name, description, category_id, price, stock_quantity, image_url, seller_id) " +
                     "VALUES (?, ?, ?, ?, ?, ?, ?);";
        try (Connection conn = getConnection();
             PreparedStatement ps = conn.prepareStatement(sql, Statement.RETURN_GENERATED_KEYS)) {
            ps.setString(1, product.getName());
            ps.setString(2, product.getDescription());
            ps.setInt(3, product.getCategoryId());
            ps.setDouble(4, product.getPrice());
            ps.setInt(5, product.getStockQuantity());
            ps.setString(6, product.getImageUrl());
            ps.setInt(7, product.getSellerId());
            ps.executeUpdate();
            try (ResultSet rs = ps.getGeneratedKeys()) {
                if (rs.next()) {
                    return rs.getInt(1);
                }
            }
        }
        return 0;
    }

    public boolean updateProduct(Product product) throws SQLException {
        StringBuilder sql = new StringBuilder("UPDATE products SET name = ?, description = ?, category_id = ?, price = ?, stock_quantity = ?, image_url = ? WHERE id = ?");
        if (product.getSellerId() > 0) {
            sql.append(" AND seller_id = ?");
        }
        sql.append(";");

        try (Connection conn = getConnection();
             PreparedStatement ps = conn.prepareStatement(sql.toString())) {
            ps.setString(1, product.getName());
            ps.setString(2, product.getDescription());
            ps.setInt(3, product.getCategoryId());
            ps.setDouble(4, product.getPrice());
            ps.setInt(5, product.getStockQuantity());
            ps.setString(6, product.getImageUrl());
            ps.setInt(7, product.getId());
            if (product.getSellerId() > 0) {
                ps.setInt(8, product.getSellerId());
            }
            return ps.executeUpdate() > 0;
        }
    }

    public boolean deleteProduct(int id, int sellerId, boolean isAdmin) throws SQLException {
        StringBuilder sql = new StringBuilder("DELETE FROM products WHERE id = ?");
        if (!isAdmin && sellerId > 0) {
            sql.append(" AND seller_id = ?");
        }
        sql.append(";");

        try (Connection conn = getConnection();
             PreparedStatement ps = conn.prepareStatement(sql.toString())) {
            ps.setInt(1, id);
            if (!isAdmin && sellerId > 0) {
                ps.setInt(2, sellerId);
            }
            return ps.executeUpdate() > 0;
        }
    }

    // ---------------- CART OPERATIONS ----------------
    public List<CartItem> getCartItems(int userId) {
        List<CartItem> items = new ArrayList<>();
        String sql = "SELECT ci.id, ci.user_id, ci.product_id, p.name, p.price, p.image_url, " +
                     "p.stock_quantity, ci.quantity, (ci.quantity * p.price) as item_subtotal, ci.created_at " +
                     "FROM cart_items ci " +
                     "JOIN products p ON ci.product_id = p.id " +
                     "WHERE ci.user_id = ? ORDER BY ci.id DESC;";
        try (Connection conn = getConnection();
             PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setInt(1, userId);
            try (ResultSet rs = ps.executeQuery()) {
                while (rs.next()) {
                    CartItem it = new CartItem();
                    it.setId(rs.getInt("id"));
                    it.setUserId(rs.getInt("user_id"));
                    it.setProductId(rs.getInt("product_id"));
                    it.setProductName(rs.getString("name"));
                    it.setPrice(rs.getDouble("price"));
                    it.setImageUrl(rs.getString("image_url"));
                    it.setAvailableStock(rs.getInt("stock_quantity"));
                    it.setQuantity(rs.getInt("quantity"));
                    it.setSubtotal(rs.getDouble("item_subtotal"));
                    it.setCreatedAt(rs.getString("created_at"));
                    items.add(it);
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return items;
    }

    public void addToCart(int userId, int productId, int quantity) throws SQLException {
        try (Connection conn = getConnection()) {
            // Check stock
            int stock = 0;
            String checkSql = "SELECT stock_quantity FROM products WHERE id = ?;";
            try (PreparedStatement ps = conn.prepareStatement(checkSql)) {
                ps.setInt(1, productId);
                try (ResultSet rs = ps.executeQuery()) {
                    if (rs.next()) {
                        stock = rs.getInt("stock_quantity");
                    } else {
                        throw new SQLException("Product not found.");
                    }
                }
            }

            int existingQty = 0;
            String exSql = "SELECT quantity FROM cart_items WHERE user_id = ? AND product_id = ?;";
            try (PreparedStatement ps = conn.prepareStatement(exSql)) {
                ps.setInt(1, userId);
                ps.setInt(2, productId);
                try (ResultSet rs = ps.executeQuery()) {
                    if (rs.next()) {
                        existingQty = rs.getInt("quantity");
                    }
                }
            }

            if (existingQty + quantity > stock) {
                throw new SQLException("Insufficient stock available.");
            }

            String upsertSql = "INSERT INTO cart_items (user_id, product_id, quantity) VALUES (?, ?, ?) " +
                               "ON CONFLICT(user_id, product_id) DO UPDATE SET quantity = quantity + ?;";
            try (PreparedStatement ps = conn.prepareStatement(upsertSql)) {
                ps.setInt(1, userId);
                ps.setInt(2, productId);
                ps.setInt(3, quantity);
                ps.setInt(4, quantity);
                ps.executeUpdate();
            }
        }
    }

    public void updateCartQuantity(int userId, int cartItemId, int newQuantity) throws SQLException {
        try (Connection conn = getConnection()) {
            if (newQuantity <= 0) {
                String delSql = "DELETE FROM cart_items WHERE id = ? AND user_id = ?;";
                try (PreparedStatement ps = conn.prepareStatement(delSql)) {
                    ps.setInt(1, cartItemId);
                    ps.setInt(2, userId);
                    ps.executeUpdate();
                }
                return;
            }

            int stock = 0;
            String stockSql = "SELECT p.stock_quantity FROM cart_items ci JOIN products p ON ci.product_id = p.id WHERE ci.id = ? AND ci.user_id = ?;";
            try (PreparedStatement ps = conn.prepareStatement(stockSql)) {
                ps.setInt(1, cartItemId);
                ps.setInt(2, userId);
                try (ResultSet rs = ps.executeQuery()) {
                    if (rs.next()) {
                        stock = rs.getInt("stock_quantity");
                    } else {
                        throw new SQLException("Cart item not found.");
                    }
                }
            }

            if (newQuantity > stock) {
                throw new SQLException("Insufficient stock available.");
            }

            String upSql = "UPDATE cart_items SET quantity = ? WHERE id = ? AND user_id = ?;";
            try (PreparedStatement ps = conn.prepareStatement(upSql)) {
                ps.setInt(1, newQuantity);
                ps.setInt(2, cartItemId);
                ps.setInt(3, userId);
                ps.executeUpdate();
            }
        }
    }

    public boolean removeFromCart(int userId, int cartItemId) throws SQLException {
        String sql = "DELETE FROM cart_items WHERE id = ? AND user_id = ?;";
        try (Connection conn = getConnection();
             PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setInt(1, cartItemId);
            ps.setInt(2, userId);
            return ps.executeUpdate() > 0;
        }
    }

    public void clearCart(int userId) throws SQLException {
        String sql = "DELETE FROM cart_items WHERE user_id = ?;";
        try (Connection conn = getConnection();
             PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setInt(1, userId);
            ps.executeUpdate();
        }
    }

    // ---------------- ORDER OPERATIONS ----------------
    public Order createOrder(int buyerId, String buyerName, String email, String phone, String address) throws SQLException {
        try (Connection conn = getConnection()) {
            conn.setAutoCommit(false);
            try {
                // Fetch cart items
                String cartSql = "SELECT ci.product_id, p.seller_id, ci.quantity, p.price, p.stock_quantity " +
                                 "FROM cart_items ci JOIN products p ON ci.product_id = p.id WHERE ci.user_id = ?;";
                List<Map<String, Object>> items = new ArrayList<>();
                double totalAmount = 0.0;

                try (PreparedStatement ps = conn.prepareStatement(cartSql)) {
                    ps.setInt(1, buyerId);
                    try (ResultSet rs = ps.executeQuery()) {
                        while (rs.next()) {
                            int qty = rs.getInt("quantity");
                            int stock = rs.getInt("stock_quantity");
                            if (qty > stock) {
                                throw new SQLException("Insufficient stock available for one or more items.");
                            }
                            double price = rs.getDouble("price");
                            totalAmount += (qty * price);

                            Map<String, Object> it = new HashMap<>();
                            it.put("productId", rs.getInt("product_id"));
                            it.put("sellerId", rs.getInt("seller_id"));
                            it.put("quantity", qty);
                            it.put("unitPrice", price);
                            items.add(it);
                        }
                    }
                }

                if (items.isEmpty()) {
                    throw new SQLException("Your shopping cart is empty.");
                }

                // Generate Order Code (e.g. PRC10001)
                int nextId = 10001;
                try (Statement stmt = conn.createStatement();
                     ResultSet rs = stmt.executeQuery("SELECT IFNULL(MAX(id), 0) + 10001 FROM orders;")) {
                    if (rs.next()) {
                        nextId = rs.getInt(1);
                    }
                }
                String orderCode = "PRC" + nextId;

                // Insert Order
                String oSql = "INSERT INTO orders (order_code, buyer_id, total_amount, buyer_name, email, phone, delivery_address, status) " +
                              "VALUES (?, ?, ?, ?, ?, ?, ?, 'Confirmed');";
                int orderId = 0;
                try (PreparedStatement ps = conn.prepareStatement(oSql, Statement.RETURN_GENERATED_KEYS)) {
                    ps.setString(1, orderCode);
                    ps.setInt(2, buyerId);
                    ps.setDouble(3, totalAmount);
                    ps.setString(4, buyerName);
                    ps.setString(5, email);
                    ps.setString(6, phone);
                    ps.setString(7, address);
                    ps.executeUpdate();
                    try (ResultSet rs = ps.getGeneratedKeys()) {
                        if (rs.next()) {
                            orderId = rs.getInt(1);
                        }
                    }
                }

                // Insert items & decrement stock
                String itemSql = "INSERT INTO order_items (order_id, product_id, seller_id, quantity, unit_price, subtotal) VALUES (?, ?, ?, ?, ?, ?);";
                String decSql = "UPDATE products SET stock_quantity = stock_quantity - ? WHERE id = ?;";

                try (PreparedStatement psItem = conn.prepareStatement(itemSql);
                     PreparedStatement psDec = conn.prepareStatement(decSql)) {
                    for (Map<String, Object> it : items) {
                        int pId = (int) it.get("productId");
                        int sId = (int) it.get("sellerId");
                        int qty = (int) it.get("quantity");
                        double price = (double) it.get("unitPrice");

                        psItem.setInt(1, orderId);
                        psItem.setInt(2, pId);
                        psItem.setInt(3, sId);
                        psItem.setInt(4, qty);
                        psItem.setDouble(5, price);
                        psItem.setDouble(6, qty * price);
                        psItem.executeUpdate();

                        psDec.setInt(1, qty);
                        psDec.setInt(2, pId);
                        psDec.executeUpdate();
                    }
                }

                // Clear cart
                try (PreparedStatement ps = conn.prepareStatement("DELETE FROM cart_items WHERE user_id = ?;")) {
                    ps.setInt(1, buyerId);
                    ps.executeUpdate();
                }

                conn.commit();

                Order order = new Order();
                order.setId(orderId);
                order.setOrderCode(orderCode);
                order.setTotalAmount(totalAmount);
                order.setStatus("Confirmed");
                return order;
            } catch (Exception e) {
                conn.rollback();
                throw e;
            } finally {
                conn.setAutoCommit(true);
            }
        }
    }

    public List<Order> getOrdersByBuyer(int buyerId) {
        String sql = "SELECT * FROM orders WHERE buyer_id = ? ORDER BY id DESC;";
        return fetchOrdersWithQuery(sql, buyerId, 0, false);
    }

    public List<Order> getOrdersBySeller(int sellerId) {
        String sql = "SELECT DISTINCT o.* FROM orders o JOIN order_items oi ON o.id = oi.order_id WHERE oi.seller_id = ? ORDER BY o.id DESC;";
        return fetchOrdersWithQuery(sql, 0, sellerId, true);
    }

    public List<Order> getAllOrders() {
        String sql = "SELECT * FROM orders ORDER BY id DESC;";
        return fetchOrdersWithQuery(sql, 0, 0, false);
    }

    private List<Order> fetchOrdersWithQuery(String sql, int buyerId, int sellerId, boolean filterSellerItems) {
        List<Order> orders = new ArrayList<>();
        try (Connection conn = getConnection();
             PreparedStatement ps = conn.prepareStatement(sql)) {
            if (buyerId > 0) ps.setInt(1, buyerId);
            else if (sellerId > 0) ps.setInt(1, sellerId);

            try (ResultSet rs = ps.executeQuery()) {
                while (rs.next()) {
                    Order o = new Order();
                    o.setId(rs.getInt("id"));
                    o.setOrderCode(rs.getString("order_code"));
                    o.setBuyerId(rs.getInt("buyer_id"));
                    o.setBuyerName(rs.getString("buyer_name"));
                    o.setEmail(rs.getString("email"));
                    o.setPhone(rs.getString("phone"));
                    o.setDeliveryAddress(rs.getString("delivery_address"));
                    o.setTotalAmount(rs.getDouble("total_amount"));
                    o.setStatus(rs.getString("status"));
                    o.setCreatedAt(rs.getString("created_at"));
                    orders.add(o);
                }
            }

            // Populate items
            String itemSql = "SELECT oi.id, oi.order_id, oi.product_id, p.name, p.image_url, oi.seller_id, " +
                             "u.name as seller_name, oi.quantity, oi.unit_price, oi.subtotal " +
                             "FROM order_items oi " +
                             "JOIN products p ON oi.product_id = p.id " +
                             "JOIN users u ON oi.seller_id = u.id " +
                             "WHERE oi.order_id = ?";
            if (filterSellerItems && sellerId > 0) {
                itemSql += " AND oi.seller_id = " + sellerId;
            }

            for (Order o : orders) {
                try (PreparedStatement ips = conn.prepareStatement(itemSql)) {
                    ips.setInt(1, o.getId());
                    try (ResultSet irs = ips.executeQuery()) {
                        while (irs.next()) {
                            OrderItem it = new OrderItem();
                            it.setId(irs.getInt("id"));
                            it.setOrderId(irs.getInt("order_id"));
                            it.setProductId(irs.getInt("product_id"));
                            it.setProductName(irs.getString("name"));
                            it.setImageUrl(irs.getString("image_url"));
                            it.setSellerId(irs.getInt("seller_id"));
                            it.setSellerName(irs.getString("seller_name"));
                            it.setQuantity(irs.getInt("quantity"));
                            it.setUnitPrice(irs.getDouble("unit_price"));
                            it.setSubtotal(irs.getDouble("subtotal"));
                            o.getItems().add(it);
                        }
                    }
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return orders;
    }

    public boolean updateOrderStatus(int orderId, String status) throws SQLException {
        String sql = "UPDATE orders SET status = ? WHERE id = ?;";
        try (Connection conn = getConnection();
             PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setString(1, status);
            ps.setInt(2, orderId);
            return ps.executeUpdate() > 0;
        }
    }

    // ---------------- REVIEWS ----------------
    public void addReview(Review review) throws SQLException {
        String sql = "INSERT INTO reviews (product_id, buyer_id, rating, comment) VALUES (?, ?, ?, ?);";
        try (Connection conn = getConnection();
             PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setInt(1, review.getProductId());
            ps.setInt(2, review.getBuyerId());
            ps.setInt(3, review.getRating());
            ps.setString(4, review.getComment());
            ps.executeUpdate();
        }
    }

    public List<Review> getReviewsByProduct(int productId) {
        List<Review> list = new ArrayList<>();
        String sql = "SELECT r.id, r.product_id, p.name as product_name, r.buyer_id, u.name as buyer_name, " +
                     "r.rating, r.comment, r.created_at " +
                     "FROM reviews r " +
                     "JOIN users u ON r.buyer_id = u.id " +
                     "JOIN products p ON r.product_id = p.id " +
                     "WHERE r.product_id = ? ORDER BY r.id DESC;";
        try (Connection conn = getConnection();
             PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setInt(1, productId);
            try (ResultSet rs = ps.executeQuery()) {
                while (rs.next()) {
                    list.add(new Review(
                        rs.getInt("id"),
                        rs.getInt("product_id"),
                        rs.getString("product_name"),
                        rs.getInt("buyer_id"),
                        rs.getString("buyer_name"),
                        rs.getInt("rating"),
                        rs.getString("comment"),
                        rs.getString("created_at")
                    ));
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return list;
    }

    // ---------------- ANALYTICS ----------------
    public Map<String, Object> getAdminStats() {
        Map<String, Object> map = new HashMap<>();
        try (Connection conn = getConnection();
             Statement stmt = conn.createStatement()) {
            ResultSet rs = stmt.executeQuery(
                "SELECT (SELECT COUNT(*) FROM users) as total_users, " +
                "(SELECT COUNT(*) FROM users WHERE role = 'buyer') as total_buyers, " +
                "(SELECT COUNT(*) FROM users WHERE role = 'seller') as total_sellers, " +
                "(SELECT COUNT(*) FROM products) as total_products, " +
                "(SELECT COUNT(*) FROM orders) as total_orders, " +
                "(SELECT IFNULL(SUM(total_amount), 0.0) FROM orders WHERE status != 'Cancelled') as total_sales;"
            );
            if (rs.next()) {
                map.put("totalUsers", rs.getInt("total_users"));
                map.put("totalBuyers", rs.getInt("total_buyers"));
                map.put("totalSellers", rs.getInt("total_sellers"));
                map.put("totalProducts", rs.getInt("total_products"));
                map.put("totalOrders", rs.getInt("total_orders"));
                map.put("totalSales", rs.getDouble("total_sales"));
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return map;
    }

    public Map<String, Object> getSellerStats(int sellerId) {
        Map<String, Object> map = new HashMap<>();
        String sql = "SELECT " +
                     "(SELECT COUNT(*) FROM products WHERE seller_id = ?) as total_products, " +
                     "(SELECT COUNT(DISTINCT order_id) FROM order_items WHERE seller_id = ?) as total_orders, " +
                     "(SELECT IFNULL(SUM(subtotal), 0.0) FROM order_items WHERE seller_id = ?) as total_sales;";
        try (Connection conn = getConnection();
             PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setInt(1, sellerId);
            ps.setInt(2, sellerId);
            ps.setInt(3, sellerId);
            try (ResultSet rs = ps.executeQuery()) {
                if (rs.next()) {
                    map.put("totalProducts", rs.getInt("total_products"));
                    map.put("totalOrders", rs.getInt("total_orders"));
                    map.put("totalSales", rs.getDouble("total_sales"));
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return map;
    }

    // ---------------- CHATBOT SEARCH HELPER ----------------
    public List<Product> searchProductsForChatbot(String query, int limit) {
        List<Product> matches = new ArrayList<>();
        String sql = "SELECT p.id, p.name, p.price, p.image_url, c.name as category_name FROM products p " +
                     "JOIN categories c ON p.category_id = c.id " +
                     "WHERE p.name LIKE ? OR p.description LIKE ? OR c.name LIKE ? LIMIT ?;";
        try (Connection conn = getConnection();
             PreparedStatement ps = conn.prepareStatement(sql)) {
            String like = "%" + query.trim() + "%";
            ps.setString(1, like);
            ps.setString(2, like);
            ps.setString(3, like);
            ps.setInt(4, limit);
            try (ResultSet rs = ps.executeQuery()) {
                while (rs.next()) {
                    Product p = new Product();
                    p.setId(rs.getInt("id"));
                    p.setName(rs.getString("name"));
                    p.setPrice(rs.getDouble("price"));
                    p.setImageUrl(rs.getString("image_url"));
                    p.setCategoryName(rs.getString("category_name"));
                    matches.add(p);
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return matches;
    }
}
