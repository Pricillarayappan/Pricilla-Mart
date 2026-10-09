import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.net.URLEncoder;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.security.MessageDigest;
import java.sql.*;
import java.util.*;
import java.util.concurrent.Executors;

/**
 * PRICILLA MART - STANDALONE JAVA FULL-STACK SERVER
 * Pure Java HTTP Server & SQLite REST API Runner
 * Run instantly with: java PricillaMartServer.java
 */
public class PricillaMartServer {

    private static final int PORT = getPort();

    private static int getPort() {
        String envPort = System.getenv("PORT");
        if (envPort != null && !envPort.trim().isEmpty()) {
            try {
                return Integer.parseInt(envPort.trim());
            } catch (NumberFormatException ignored) {}
        }
        return 8080;
    }
    private static final String BASE_DIR = System.getProperty("user.dir");
    private static final File FRONTEND_DIR = new File(BASE_DIR, "frontend");
    private static final File DB_FILE = new File(BASE_DIR, "database/pricillamart.db");
    private static final String JWT_SECRET = "pricillamart_secret_key_college_capstone_2026";
    private static String connectionUrl;

    public static void main(String[] args) throws Exception {
        System.out.println("============================================================");
        System.out.println("                     PRICILLA MART                          ");
        System.out.println("          FULL-STACK JAVA REST SERVER RUNNER                ");
        System.out.println("============================================================");

        connectionUrl = "jdbc:sqlite:" + DB_FILE.getAbsolutePath();
        System.out.println("[*] SQLite Database: " + DB_FILE.getAbsolutePath());
        System.out.println("[*] Frontend Root:   " + FRONTEND_DIR.getAbsolutePath());

        // Verify SQLite JDBC driver or database
        try {
            Class.forName("org.xerial.sqlite.jdbc");
            try (Connection conn = DriverManager.getConnection(connectionUrl);
                 Statement st = conn.createStatement()) {
                st.execute("PRAGMA foreign_keys = ON;");
            }
            System.out.println("[+] SQLite JDBC connection established successfully.");
        } catch (ClassNotFoundException e) {
            System.out.println("[!] Notice: SQLite JDBC not in classpath, checking database file...");
        } catch (SQLException e) {
            System.out.println("[!] Database note: " + e.getMessage());
        }

        HttpServer server = HttpServer.create(new InetSocketAddress("0.0.0.0", PORT), 0);
        server.setExecutor(Executors.newFixedThreadPool(16));

        // Route Contexts
        server.createContext("/", new StaticAndApiHandler());

        server.start();

        System.out.println("[+] Server is LIVE and listening at: http://localhost:" + PORT);
        System.out.println("------------------------------------------------------------");
        System.out.println("DEFAULT DEMO ACCOUNTS:");
        System.out.println("  Admin:  admin@pricillamart.com     | Password: Admin@123");
        System.out.println("  Seller: techworld@pricillamart.com | Password: Seller@123");
        System.out.println("  Buyer:  rahul@gmail.com            | Password: Buyer@123");
        System.out.println("============================================================");
    }

    static class StaticAndApiHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String path = exchange.getRequestURI().getPath();
            String method = exchange.getRequestMethod();

            // Handle CORS Preflight
            if ("OPTIONS".equalsIgnoreCase(method)) {
                exchange.getResponseHeaders().add("Access-Control-Allow-Origin", "*");
                exchange.getResponseHeaders().add("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
                exchange.getResponseHeaders().add("Access-Control-Allow-Headers", "Content-Type, Authorization");
                exchange.sendResponseHeaders(200, -1);
                exchange.close();
                return;
            }

            // Handle API routes
            if (path.startsWith("/api/")) {
                handleApi(exchange, path, method);
                return;
            }

            // Serve Static Files
            serveStatic(exchange, path);
        }

        private void serveStatic(HttpExchange exchange, String path) throws IOException {
            if ("/".equals(path) || path.isEmpty()) {
                path = "/index.html";
            }

            File file = new File(FRONTEND_DIR, path);
            if (!file.exists() || file.isDirectory()) {
                file = new File(FRONTEND_DIR, "/404.html");
                if (!file.exists()) {
                    byte[] notFound = "404 Not Found".getBytes(StandardCharsets.UTF_8);
                    exchange.sendResponseHeaders(404, notFound.length);
                    exchange.getResponseBody().write(notFound);
                    exchange.close();
                    return;
                }
            }

            String mime = getMimeType(file.getName());
            byte[] bytes = Files.readAllBytes(file.toPath());

            exchange.getResponseHeaders().add("Content-Type", mime);
            exchange.getResponseHeaders().add("Access-Control-Allow-Origin", "*");
            exchange.sendResponseHeaders(200, bytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(bytes);
            }
        }

        private void handleApi(HttpExchange exchange, String path, String method) throws IOException {
            exchange.getResponseHeaders().add("Content-Type", "application/json; charset=utf-8");
            exchange.getResponseHeaders().add("Access-Control-Allow-Origin", "*");
            exchange.getResponseHeaders().add("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            exchange.getResponseHeaders().add("Access-Control-Allow-Headers", "Content-Type, Authorization");

            String body = readBody(exchange);
            String authHeader = exchange.getRequestHeaders().getFirst("Authorization");
            String token = (authHeader != null && authHeader.startsWith("Bearer ")) ? authHeader.substring(7) : null;
            Map<String, String> user = verifyToken(token);

            try (Connection conn = getConnection()) {
                if (conn == null) {
                    sendJson(exchange, 500, "{\"success\":false,\"error\":\"Database driver not available.\"}");
                    return;
                }

                if ("/api/categories".equals(path) && "GET".equalsIgnoreCase(method)) {
                    String json = queryCategories(conn);
                    sendJson(exchange, 200, "{\"success\":true,\"data\":" + json + "}");
                } else if ("/api/products".equals(path) && "GET".equalsIgnoreCase(method)) {
                    Map<String, String> qp = parseQuery(exchange.getRequestURI().getQuery());
                    String json = queryProducts(conn, qp.get("q"), qp.get("category_id"), qp.get("seller_id"), qp.get("sort"));
                    sendJson(exchange, 200, "{\"success\":true,\"data\":" + json + "}");
                } else if (path.startsWith("/api/products/") && "GET".equalsIgnoreCase(method)) {
                    String sub = path.substring(14);
                    if (sub.endsWith("/reviews")) {
                        int pId = Integer.parseInt(sub.split("/")[0]);
                        String json = queryReviews(conn, pId);
                        sendJson(exchange, 200, "{\"success\":true,\"data\":" + json + "}");
                    } else {
                        int pId = Integer.parseInt(sub);
                        String json = queryProductById(conn, pId);
                        if (json == null) sendJson(exchange, 404, "{\"success\":false,\"error\":\"Product not found.\"}");
                        else sendJson(exchange, 200, "{\"success\":true,\"data\":" + json + "}");
                    }
                } else if ("/api/login".equals(path) && "POST".equalsIgnoreCase(method)) {
                    Map<String, String> creds = parseJsonSimple(body);
                    String email = creds.get("email");
                    String pass = creds.get("password");
                    String res = handleLogin(conn, email, pass);
                    sendJson(exchange, res.contains("\"token\"") ? 200 : 401, res);
                } else if ("/api/register".equals(path) && "POST".equalsIgnoreCase(method)) {
                    Map<String, String> fields = parseJsonSimple(body);
                    String res = handleRegister(conn, fields);
                    sendJson(exchange, res.contains("\"token\"") ? 201 : 400, res);
                } else if ("/api/cart".equals(path) && "GET".equalsIgnoreCase(method)) {
                    if (user == null) { sendJson(exchange, 401, "{\"success\":false,\"error\":\"Unauthorized\"}"); return; }
                    String json = queryCart(conn, Integer.parseInt(user.get("userId")));
                    sendJson(exchange, 200, "{\"success\":true,\"data\":" + json + "}");
                } else if ("/api/cart".equals(path) && "POST".equalsIgnoreCase(method)) {
                    if (user == null) { sendJson(exchange, 401, "{\"success\":false,\"error\":\"Unauthorized\"}"); return; }
                    Map<String, String> b = parseJsonSimple(body);
                    int pId = Integer.parseInt(b.getOrDefault("productId", "0"));
                    int qty = Integer.parseInt(b.getOrDefault("quantity", "1"));
                    addToCart(conn, Integer.parseInt(user.get("userId")), pId, qty);
                    sendJson(exchange, 200, "{\"success\":true,\"message\":\"Added to cart\"}");
                } else if ("/api/orders".equals(path) && "GET".equalsIgnoreCase(method)) {
                    if (user == null) { sendJson(exchange, 401, "{\"success\":false,\"error\":\"Unauthorized\"}"); return; }
                    String json = queryOrders(conn, user);
                    sendJson(exchange, 200, "{\"success\":true,\"data\":" + json + "}");
                } else if ("/api/orders".equals(path) && "POST".equalsIgnoreCase(method)) {
                    if (user == null) { sendJson(exchange, 401, "{\"success\":false,\"error\":\"Unauthorized\"}"); return; }
                    Map<String, String> b = parseJsonSimple(body);
                    String res = createOrder(conn, Integer.parseInt(user.get("userId")), b);
                    sendJson(exchange, 201, "{\"success\":true,\"message\":\"Order placed successfully.\",\"data\":" + res + "}");
                } else if ("/api/chatbot".equals(path) && "POST".equalsIgnoreCase(method)) {
                    Map<String, String> b = parseJsonSimple(body);
                    String res = handleChatbot(conn, b.getOrDefault("message", ""), user);
                    sendJson(exchange, 200, "{\"success\":true,\"data\":" + res + "}");
                } else if ("/api/admin/stats".equals(path) && "GET".equalsIgnoreCase(method)) {
                    String json = queryAdminStats(conn);
                    sendJson(exchange, 200, "{\"success\":true,\"data\":" + json + "}");
                } else if ("/api/admin/users".equals(path) && "GET".equalsIgnoreCase(method)) {
                    String json = queryAdminUsers(conn);
                    sendJson(exchange, 200, "{\"success\":true,\"data\":" + json + "}");
                } else if ("/api/logout".equals(path)) {
                    sendJson(exchange, 200, "{\"success\":true,\"message\":\"Logged out\"}");
                } else {
                    sendJson(exchange, 200, "{\"success\":true,\"message\":\"API request received.\"}");
                }
            } catch (Exception e) {
                sendJson(exchange, 400, "{\"success\":false,\"error\":\"" + escapeJson(e.getMessage()) + "\"}");
            }
        }
    }

    private static Connection getConnection() {
        try {
            return DriverManager.getConnection(connectionUrl);
        } catch (Exception e) {
            return null;
        }
    }

    private static void sendJson(HttpExchange exchange, int status, String json) throws IOException {
        byte[] bytes = json.getBytes(StandardCharsets.UTF_8);
        exchange.sendResponseHeaders(status, bytes.length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(bytes);
        }
    }

    private static String readBody(HttpExchange exchange) throws IOException {
        try (InputStream is = exchange.getRequestBody();
             ByteArrayOutputStream bos = new ByteArrayOutputStream()) {
            byte[] buf = new byte[4096];
            int n;
            while ((n = is.read(buf)) != -1) bos.write(buf, 0, n);
            return bos.toString(StandardCharsets.UTF_8);
        }
    }

    private static String getMimeType(String name) {
        if (name.endsWith(".html") || name.endsWith(".htm")) return "text/html; charset=utf-8";
        if (name.endsWith(".css")) return "text/css; charset=utf-8";
        if (name.endsWith(".js")) return "application/javascript; charset=utf-8";
        if (name.endsWith(".json")) return "application/json; charset=utf-8";
        if (name.endsWith(".png")) return "image/png";
        if (name.endsWith(".jpg") || name.endsWith(".jpeg")) return "image/jpeg";
        if (name.endsWith(".svg")) return "image/svg+xml";
        return "application/octet-stream";
    }

    private static String escapeJson(String s) {
        if (s == null) return "";
        return s.replace("\\", "\\\\").replace("\"", "\\\"").replace("\n", "\\n").replace("\r", "");
    }

    private static Map<String, String> parseQuery(String query) {
        Map<String, String> map = new HashMap<>();
        if (query == null) return map;
        for (String pair : query.split("&")) {
            int eq = pair.indexOf('=');
            if (eq > 0) {
                map.put(URLDecoder.decode(pair.substring(0, eq), StandardCharsets.UTF_8),
                        URLDecoder.decode(pair.substring(eq + 1), StandardCharsets.UTF_8));
            }
        }
        return map;
    }

    private static Map<String, String> parseJsonSimple(String body) {
        Map<String, String> map = new HashMap<>();
        if (body == null) return map;
        for (String line : body.split(",")) {
            int c = line.indexOf(':');
            if (c > 0) {
                String k = line.substring(0, c).replaceAll("[\"{}\\s]", "");
                String v = line.substring(c + 1).replaceAll("[\"{}\\s]", "");
                map.put(k, v);
            }
        }
        return map;
    }

    private static String hashPassword(String s) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] h = md.digest(s.getBytes(StandardCharsets.UTF_8));
            StringBuilder sb = new StringBuilder();
            for (byte b : h) sb.append(String.format("%02x", b));
            return sb.toString();
        } catch (Exception e) {
            return s;
        }
    }

    private static String createToken(int id, String email, String role) {
        long exp = (System.currentTimeMillis() / 1000L) + (86400L * 7L);
        String payload = id + ":" + email + ":" + role + ":" + exp;
        String b64 = Base64.getUrlEncoder().withoutPadding().encodeToString(payload.getBytes(StandardCharsets.UTF_8));
        String sig = hashPassword(b64 + "." + JWT_SECRET);
        return b64 + "." + sig;
    }

    private static Map<String, String> verifyToken(String token) {
        if (token == null || !token.contains(".")) return null;
        try {
            int dot = token.lastIndexOf('.');
            String b64 = token.substring(0, dot);
            String sig = token.substring(dot + 1);
            if (!sig.equalsIgnoreCase(hashPassword(b64 + "." + JWT_SECRET))) return null;

            String decoded;
            try {
                decoded = new String(Base64.getUrlDecoder().decode(b64), StandardCharsets.UTF_8);
            } catch (Exception ex) {
                decoded = URLDecoder.decode(b64, StandardCharsets.UTF_8);
            }
            String[] parts = decoded.split(":");
            Map<String, String> m = new HashMap<>();
            m.put("userId", parts[0]);
            m.put("email", parts[1]);
            m.put("role", parts[2]);
            return m;
        } catch (Exception e) {
            return null;
        }
    }

    // ---------------- SQL HELPERS ----------------
    private static String queryCategories(Connection conn) throws SQLException {
        try (Statement st = conn.createStatement();
             ResultSet rs = st.executeQuery("SELECT c.id, c.name, c.slug, c.icon, c.description, COUNT(p.id) as pcount FROM categories c LEFT JOIN products p ON c.id = p.category_id GROUP BY c.id ORDER BY c.id ASC")) {
            StringBuilder sb = new StringBuilder("[");
            boolean first = true;
            while (rs.next()) {
                if (!first) sb.append(",");
                sb.append(String.format("{\"id\":%d,\"name\":\"%s\",\"slug\":\"%s\",\"icon\":\"%s\",\"description\":\"%s\",\"productCount\":%d}",
                        rs.getInt("id"), escapeJson(rs.getString("name")), escapeJson(rs.getString("slug")),
                        escapeJson(rs.getString("icon")), escapeJson(rs.getString("description")), rs.getInt("pcount")));
                first = false;
            }
            sb.append("]");
            return sb.toString();
        }
    }

    private static String queryProducts(Connection conn, String q, String catId, String sellerId, String sort) throws SQLException {
        StringBuilder sql = new StringBuilder("SELECT p.id, p.name, p.description, p.category_id, c.name as category_name, p.price, p.stock_quantity, p.image_url, p.seller_id, u.name as seller_name, p.created_at, IFNULL(AVG(r.rating), 0.0) as avg_rating, COUNT(r.id) as rcount FROM products p JOIN categories c ON p.category_id = c.id JOIN users u ON p.seller_id = u.id LEFT JOIN reviews r ON p.id = r.product_id WHERE 1=1 ");
        if (catId != null && !catId.isEmpty() && !"0".equals(catId)) sql.append(" AND p.category_id = ").append(Integer.parseInt(catId));
        if (sellerId != null && !sellerId.isEmpty() && !"0".equals(sellerId)) sql.append(" AND p.seller_id = ").append(Integer.parseInt(sellerId));
        if (q != null && !q.isEmpty()) sql.append(" AND (p.name LIKE '%").append(escapeJson(q)).append("%' OR p.description LIKE '%").append(escapeJson(q)).append("%') ");
        sql.append(" GROUP BY p.id ");
        if ("price_asc".equals(sort)) sql.append(" ORDER BY p.price ASC");
        else if ("price_desc".equals(sort)) sql.append(" ORDER BY p.price DESC");
        else if ("rating".equals(sort)) sql.append(" ORDER BY avg_rating DESC");
        else sql.append(" ORDER BY p.id DESC");

        try (Statement st = conn.createStatement();
             ResultSet rs = st.executeQuery(sql.toString())) {
            StringBuilder sb = new StringBuilder("[");
            boolean first = true;
            while (rs.next()) {
                if (!first) sb.append(",");
                sb.append(String.format("{\"id\":%d,\"name\":\"%s\",\"description\":\"%s\",\"categoryId\":%d,\"categoryName\":\"%s\",\"price\":%.2f,\"stockQuantity\":%d,\"imageUrl\":\"%s\",\"sellerId\":%d,\"sellerName\":\"%s\",\"averageRating\":%.1f,\"reviewCount\":%d}",
                        rs.getInt("id"), escapeJson(rs.getString("name")), escapeJson(rs.getString("description")),
                        rs.getInt("category_id"), escapeJson(rs.getString("category_name")), rs.getDouble("price"),
                        rs.getInt("stock_quantity"), escapeJson(rs.getString("image_url")), rs.getInt("seller_id"),
                        escapeJson(rs.getString("seller_name")), rs.getDouble("avg_rating"), rs.getInt("rcount")));
                first = false;
            }
            sb.append("]");
            return sb.toString();
        }
    }

    private static String queryProductById(Connection conn, int id) throws SQLException {
        String sql = "SELECT p.id, p.name, p.description, p.category_id, c.name as category_name, p.price, p.stock_quantity, p.image_url, p.seller_id, u.name as seller_name, p.created_at, IFNULL(AVG(r.rating), 0.0) as avg_rating, COUNT(r.id) as rcount FROM products p JOIN categories c ON p.category_id = c.id JOIN users u ON p.seller_id = u.id LEFT JOIN reviews r ON p.id = r.product_id WHERE p.id = " + id + " GROUP BY p.id;";
        try (Statement st = conn.createStatement();
             ResultSet rs = st.executeQuery(sql)) {
            if (rs.next()) {
                return String.format("{\"id\":%d,\"name\":\"%s\",\"description\":\"%s\",\"categoryId\":%d,\"categoryName\":\"%s\",\"price\":%.2f,\"stockQuantity\":%d,\"imageUrl\":\"%s\",\"sellerId\":%d,\"sellerName\":\"%s\",\"averageRating\":%.1f,\"reviewCount\":%d}",
                        rs.getInt("id"), escapeJson(rs.getString("name")), escapeJson(rs.getString("description")),
                        rs.getInt("category_id"), escapeJson(rs.getString("category_name")), rs.getDouble("price"),
                        rs.getInt("stock_quantity"), escapeJson(rs.getString("image_url")), rs.getInt("seller_id"),
                        escapeJson(rs.getString("seller_name")), rs.getDouble("avg_rating"), rs.getInt("rcount"));
            }
        }
        return null;
    }

    private static String queryReviews(Connection conn, int pId) throws SQLException {
        String sql = "SELECT r.id, r.product_id, p.name as product_name, r.buyer_id, u.name as buyer_name, r.rating, r.comment, r.created_at FROM reviews r JOIN users u ON r.buyer_id = u.id JOIN products p ON r.product_id = p.id WHERE r.product_id = " + pId + " ORDER BY r.id DESC;";
        try (Statement st = conn.createStatement();
             ResultSet rs = st.executeQuery(sql)) {
            StringBuilder sb = new StringBuilder("{\"reviews\":[");
            boolean first = true;
            double sum = 0; int cnt = 0;
            while (rs.next()) {
                if (!first) sb.append(",");
                int r = rs.getInt("rating");
                sum += r; cnt++;
                sb.append(String.format("{\"id\":%d,\"productId\":%d,\"buyerId\":%d,\"buyerName\":\"%s\",\"rating\":%d,\"comment\":\"%s\",\"createdAt\":\"%s\"}",
                        rs.getInt("id"), rs.getInt("product_id"), rs.getInt("buyer_id"), escapeJson(rs.getString("buyer_name")),
                        r, escapeJson(rs.getString("comment")), escapeJson(rs.getString("created_at"))));
                first = false;
            }
            double avg = cnt > 0 ? (sum / cnt) : 0.0;
            sb.append("],\"averageRating\":").append(String.format("%.1f", avg)).append(",\"reviewCount\":").append(cnt).append("}");
            return sb.toString();
        }
    }

    private static String handleLogin(Connection conn, String email, String pass) throws SQLException {
        String sql = "SELECT * FROM users WHERE email = ?;";
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setString(1, email);
            try (ResultSet rs = ps.executeQuery()) {
                if (rs.next()) {
                    String hash = rs.getString("password_hash");
                    if (hash.equals(pass) || hash.equalsIgnoreCase(hashPassword(pass))) {
                        String tok = createToken(rs.getInt("id"), rs.getString("email"), rs.getString("role"));
                        return String.format("{\"success\":true,\"message\":\"Login successful.\",\"data\":{\"token\":\"%s\",\"user\":{\"id\":%d,\"name\":\"%s\",\"email\":\"%s\",\"role\":\"%s\"}}}",
                                tok, rs.getInt("id"), escapeJson(rs.getString("name")), escapeJson(rs.getString("email")), rs.getString("role"));
                    }
                }
            }
        }
        return "{\"success\":false,\"error\":\"Invalid email or password.\"}";
    }

    private static String handleRegister(Connection conn, Map<String, String> f) throws SQLException {
        String name = f.get("name"), email = f.get("email"), phone = f.get("phone"), pass = f.get("password"), role = f.get("role");
        if (name == null || email == null || pass == null || role == null) return "{\"success\":false,\"error\":\"All fields required\"}";

        String sql = "INSERT INTO users (name, email, phone, password_hash, role) VALUES (?, ?, ?, ?, ?);";
        try (PreparedStatement ps = conn.prepareStatement(sql, Statement.RETURN_GENERATED_KEYS)) {
            ps.setString(1, name);
            ps.setString(2, email);
            ps.setString(3, phone != null ? phone : "");
            ps.setString(4, hashPassword(pass));
            ps.setString(5, role.toLowerCase());
            ps.executeUpdate();
            try (ResultSet rs = ps.getGeneratedKeys()) {
                if (rs.next()) {
                    int id = rs.getInt(1);
                    String tok = createToken(id, email, role.toLowerCase());
                    return String.format("{\"success\":true,\"message\":\"Registered successfully.\",\"data\":{\"token\":\"%s\",\"user\":{\"id\":%d,\"name\":\"%s\",\"email\":\"%s\",\"role\":\"%s\"}}}",
                            tok, id, escapeJson(name), escapeJson(email), role.toLowerCase());
                }
            }
        }
        return "{\"success\":false,\"error\":\"Failed to register user.\"}";
    }

    private static String queryCart(Connection conn, int userId) throws SQLException {
        String sql = "SELECT ci.id, ci.user_id, ci.product_id, p.name, p.price, p.image_url, p.stock_quantity, ci.quantity, (ci.quantity * p.price) as subtotal FROM cart_items ci JOIN products p ON ci.product_id = p.id WHERE ci.user_id = " + userId + ";";
        try (Statement st = conn.createStatement();
             ResultSet rs = st.executeQuery(sql)) {
            StringBuilder sb = new StringBuilder("{\"items\":[");
            boolean first = true;
            double tot = 0; int cnt = 0;
            while (rs.next()) {
                if (!first) sb.append(",");
                double sub = rs.getDouble("subtotal");
                tot += sub; cnt++;
                sb.append(String.format("{\"id\":%d,\"productId\":%d,\"productName\":\"%s\",\"price\":%.2f,\"imageUrl\":\"%s\",\"availableStock\":%d,\"quantity\":%d,\"subtotal\":%.2f}",
                        rs.getInt("id"), rs.getInt("product_id"), escapeJson(rs.getString("name")),
                        rs.getDouble("price"), escapeJson(rs.getString("image_url")), rs.getInt("stock_quantity"),
                        rs.getInt("quantity"), sub));
                first = false;
            }
            sb.append("],\"subtotal\":").append(tot).append(",\"total\":").append(tot).append(",\"itemCount\":").append(cnt).append("}");
            return sb.toString();
        }
    }

    private static void addToCart(Connection conn, int uId, int pId, int qty) throws SQLException {
        String sql = "INSERT INTO cart_items (user_id, product_id, quantity) VALUES (?, ?, ?) ON CONFLICT(user_id, product_id) DO UPDATE SET quantity = quantity + ?;";
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setInt(1, uId);
            ps.setInt(2, pId);
            ps.setInt(3, qty);
            ps.setInt(4, qty);
            ps.executeUpdate();
        }
    }

    private static String queryOrders(Connection conn, Map<String, String> user) throws SQLException {
        String role = user.get("role");
        String sql = "admin".equalsIgnoreCase(role) ? "SELECT * FROM orders ORDER BY id DESC;" :
                     "seller".equalsIgnoreCase(role) ? "SELECT DISTINCT o.* FROM orders o JOIN order_items oi ON o.id = oi.order_id WHERE oi.seller_id = " + user.get("userId") + " ORDER BY o.id DESC;" :
                     "SELECT * FROM orders WHERE buyer_id = " + user.get("userId") + " ORDER BY id DESC;";

        try (Statement st = conn.createStatement();
             ResultSet rs = st.executeQuery(sql)) {
            StringBuilder sb = new StringBuilder("[");
            boolean first = true;
            while (rs.next()) {
                if (!first) sb.append(",");
                int oId = rs.getInt("id");
                String payMethod = "Cash on Delivery";
                try { payMethod = rs.getString("payment_method"); } catch (Exception ignored) {}
                if (payMethod == null || payMethod.isEmpty()) payMethod = "Cash on Delivery";
                sb.append(String.format("{\"id\":%d,\"orderCode\":\"%s\",\"buyerName\":\"%s\",\"totalAmount\":%.2f,\"paymentMethod\":\"%s\",\"status\":\"%s\",\"createdAt\":\"%s\",\"deliveryAddress\":\"%s\",\"items\":[",
                        oId, rs.getString("order_code"), escapeJson(rs.getString("buyer_name")),
                        rs.getDouble("total_amount"), escapeJson(payMethod), rs.getString("status"), rs.getString("created_at"),
                        escapeJson(rs.getString("delivery_address"))));

                // load items
                try (Statement st2 = conn.createStatement();
                     ResultSet irs = st2.executeQuery("SELECT oi.*, p.name, p.image_url, u.name as sname FROM order_items oi JOIN products p ON oi.product_id = p.id JOIN users u ON oi.seller_id = u.id WHERE oi.order_id = " + oId)) {
                    boolean firstItem = true;
                    while (irs.next()) {
                        if (!firstItem) sb.append(",");
                        sb.append(String.format("{\"id\":%d,\"productName\":\"%s\",\"imageUrl\":\"%s\",\"sellerName\":\"%s\",\"quantity\":%d,\"unitPrice\":%.2f,\"subtotal\":%.2f}",
                                irs.getInt("id"), escapeJson(irs.getString("name")), escapeJson(irs.getString("image_url")),
                                escapeJson(irs.getString("sname")), irs.getInt("quantity"), irs.getDouble("unit_price"), irs.getDouble("subtotal")));
                        firstItem = false;
                    }
                }
                sb.append("]}");
                first = false;
            }
            sb.append("]");
            return sb.toString();
        }
    }

    private static String createOrder(Connection conn, int uId, Map<String, String> b) throws SQLException {
        conn.setAutoCommit(false);
        try {
            int max = 10001;
            try (Statement st = conn.createStatement(); ResultSet rs = st.executeQuery("SELECT IFNULL(MAX(id), 0) + 10001 FROM orders")) {
                if (rs.next()) max = rs.getInt(1);
            }
            String code = "PRC" + max;

            double total = 0;
            List<int[]> items = new ArrayList<>();
            try (Statement st = conn.createStatement();
                 ResultSet rs = st.executeQuery("SELECT ci.product_id, ci.quantity, p.price, p.seller_id FROM cart_items ci JOIN products p ON ci.product_id = p.id WHERE ci.user_id = " + uId)) {
                while (rs.next()) {
                    int pId = rs.getInt("product_id"), q = rs.getInt("quantity"), sId = rs.getInt("seller_id");
                    double p = rs.getDouble("price");
                    total += (q * p);
                    items.add(new int[]{pId, sId, q, (int) p});
                }
            }

            int oId = 0;
            String payMethod = b.getOrDefault("paymentMethod", "Cash on Delivery");
            try (PreparedStatement ps = conn.prepareStatement("INSERT INTO orders (order_code, buyer_id, total_amount, buyer_name, email, phone, delivery_address, payment_method, status) VALUES (?, ?, ?, ?, ?, ?, ?, ?, 'Confirmed')", Statement.RETURN_GENERATED_KEYS)) {
                ps.setString(1, code);
                ps.setInt(2, uId);
                ps.setDouble(3, total);
                ps.setString(4, b.getOrDefault("buyerName", "Customer"));
                ps.setString(5, b.getOrDefault("email", ""));
                ps.setString(6, b.getOrDefault("phone", ""));
                ps.setString(7, b.getOrDefault("deliveryAddress", ""));
                ps.setString(8, payMethod);
                ps.executeUpdate();
                try (ResultSet rs = ps.getGeneratedKeys()) { if (rs.next()) oId = rs.getInt(1); }
            }

            for (int[] it : items) {
                try (PreparedStatement ps = conn.prepareStatement("INSERT INTO order_items (order_id, product_id, seller_id, quantity, unit_price, subtotal) VALUES (?, ?, ?, ?, ?, ?)")) {
                    ps.setInt(1, oId);
                    ps.setInt(2, it[0]);
                    ps.setInt(3, it[1]);
                    ps.setInt(4, it[2]);
                    ps.setDouble(5, it[3]);
                    ps.setDouble(6, it[2] * it[3]);
                    ps.executeUpdate();
                }
                try (Statement st = conn.createStatement()) {
                    st.execute("UPDATE products SET stock_quantity = stock_quantity - " + it[2] + " WHERE id = " + it[0]);
                }
            }

            try (Statement st = conn.createStatement()) {
                st.execute("DELETE FROM cart_items WHERE user_id = " + uId);
            }

            conn.commit();
            return "{\"orderCode\":\"" + code + "\",\"totalAmount\":" + total + "}";
        } catch (Exception e) {
            conn.rollback();
            throw e;
        } finally {
            conn.setAutoCommit(true);
        }
    }

    private static String handleChatbot(Connection conn, String msg, Map<String, String> user) throws SQLException {
        String m = msg.toLowerCase();
        String reply;
        String recs = "[]";
        String quick = "[\"Find Laptops\",\"Show Electronics\",\"Track Orders\"]";

        if (m.contains("laptop") || m.contains("computer")) {
            reply = "Sure! You can explore top-rated laptops in our Electronics category:";
            recs = "[{\"id\":1,\"name\":\"UltraBook Pro 15.6 Laptop\",\"price\":45000.0,\"imageUrl\":\"https://images.unsplash.com/photo-1496181133206-80ce9b88a853?w=600\"}]";
        } else if (m.contains("order") || m.contains("track")) {
            reply = "You can view and track your orders in real-time under the Orders tab in the top navigation bar.";
        } else {
            reply = "Hello! I am your Pricilla Mart AI Assistant. How can I help you find products today?";
        }

        return "{\"reply\":\"" + escapeJson(reply) + "\",\"recommendations\":" + recs + ",\"quickReplies\":" + quick + "}";
    }

    private static String queryAdminStats(Connection conn) throws SQLException {
        try (Statement st = conn.createStatement();
             ResultSet rs = st.executeQuery("SELECT (SELECT COUNT(*) FROM users) as u, (SELECT COUNT(*) FROM users WHERE role='buyer') as b, (SELECT COUNT(*) FROM users WHERE role='seller') as s, (SELECT COUNT(*) FROM products) as p, (SELECT COUNT(*) FROM orders) as o, (SELECT IFNULL(SUM(total_amount), 0.0) FROM orders) as sa")) {
            if (rs.next()) {
                return String.format("{\"totalUsers\":%d,\"totalBuyers\":%d,\"totalSellers\":%d,\"totalProducts\":%d,\"totalOrders\":%d,\"totalSales\":%.2f}",
                        rs.getInt("u"), rs.getInt("b"), rs.getInt("s"), rs.getInt("p"), rs.getInt("o"), rs.getDouble("sa"));
            }
        }
        return "{}";
    }

    private static String queryAdminUsers(Connection conn) throws SQLException {
        try (Statement st = conn.createStatement();
             ResultSet rs = st.executeQuery("SELECT id, name, email, phone, role, created_at FROM users ORDER BY id DESC")) {
            StringBuilder sb = new StringBuilder("[");
            boolean first = true;
            while (rs.next()) {
                if (!first) sb.append(",");
                sb.append(String.format("{\"id\":%d,\"name\":\"%s\",\"email\":\"%s\",\"phone\":\"%s\",\"role\":\"%s\",\"createdAt\":\"%s\"}",
                        rs.getInt("id"), escapeJson(rs.getString("name")), escapeJson(rs.getString("email")),
                        escapeJson(rs.getString("phone")), rs.getString("role"), rs.getString("created_at")));
                first = false;
            }
            sb.append("]");
            return sb.toString();
        }
    }
}
