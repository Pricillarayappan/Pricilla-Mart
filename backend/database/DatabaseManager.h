#pragma once
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <mutex>
#include "models/User.h"
#include "models/Product.h"
#include "models/Category.h"
#include "models/CartItem.h"
#include "models/Order.h"
#include "models/Review.h"

struct sqlite3;

namespace PricillaMart {

struct AdminStats {
    int totalUsers = 0;
    int totalBuyers = 0;
    int totalSellers = 0;
    int totalProducts = 0;
    int totalOrders = 0;
    double totalSales = 0.0;
};

struct SellerStats {
    int totalProducts = 0;
    int totalOrders = 0;
    double totalSales = 0.0;
};

class DatabaseManager {
public:
    static DatabaseManager& getInstance();

    bool initialize(const std::string& dbPath, const std::string& schemaPath, const std::string& seedPath);
    void close();

    // User Operations
    bool createUser(const User& user, int& newId);
    std::optional<User> findUserByEmail(const std::string& email);
    std::optional<User> findUserById(int id);
    std::vector<User> getAllUsers(const std::string& roleFilter = "");

    // Category Operations
    std::vector<Category> getAllCategories();
    std::optional<Category> getCategoryById(int id);

    // Product Operations
    std::vector<Product> getProducts(const std::string& query = "", int categoryId = 0, int sellerId = 0, const std::string& sortBy = "", double minPrice = 0.0, double maxPrice = 0.0);
    std::optional<Product> getProductById(int id);
    bool createProduct(const Product& product, int& newId);
    bool updateProduct(const Product& product);
    bool deleteProduct(int id, int sellerId = 0, bool isAdmin = false);
    bool updateStock(int productId, int deltaQuantity);

    // Cart Operations
    std::vector<CartItem> getCartItems(int userId, double& subtotal, double& total);
    bool addToCart(int userId, int productId, int quantity, std::string& errorMessage);
    bool updateCartQuantity(int userId, int cartItemId, int newQuantity, std::string& errorMessage);
    bool removeFromCart(int userId, int cartItemId);
    bool clearCart(int userId);

    // Order Operations
    bool createOrder(int buyerId, const std::string& buyerName, const std::string& email, const std::string& phone, const std::string& address, std::string& outOrderCode, std::string& errorMessage);
    std::vector<Order> getOrdersByBuyer(int buyerId);
    std::vector<Order> getOrdersBySeller(int sellerId);
    std::vector<Order> getAllOrders();
    std::optional<Order> getOrderById(int orderId);
    std::optional<Order> getOrderByCode(const std::string& orderCode);
    bool updateOrderStatus(int orderId, const std::string& status, int sellerId = 0, bool isAdmin = false);

    // Review Operations
    bool addReview(const Review& review, std::string& errorMessage);
    std::vector<Review> getReviewsByProduct(int productId, double& avgRating, int& count);
    bool hasPurchasedProduct(int buyerId, int productId);

    // Admin & Seller Analytics
    AdminStats getAdminStats();
    SellerStats getSellerStats(int sellerId);

    // Chatbot Helper
    std::vector<Product> searchProductsForChatbot(const std::string& query, int limit = 5);

private:
    DatabaseManager() = default;
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool executeSqlScript(const std::string& filePath);
    bool executeSql(const std::string& sql);

    sqlite3* db_ = nullptr;
    std::mutex dbMutex_;
};

} // namespace PricillaMart
