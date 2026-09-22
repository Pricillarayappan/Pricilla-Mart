#include "DatabaseManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <random>
#include <algorithm>

#if __has_include(<sqlite3.h>)
#include <sqlite3.h>
#else
#include "../third_party/sqlite3.h"
#endif

namespace PricillaMart {

DatabaseManager& DatabaseManager::getInstance() {
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::~DatabaseManager() {
    close();
}

void DatabaseManager::close() {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool DatabaseManager::initialize(const std::string& dbPath, const std::string& schemaPath, const std::string& seedPath) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }

    int rc = sqlite3_open(dbPath.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "[DatabaseManager] Cannot open database: " << (db_ ? sqlite3_errmsg(db_) : "unknown") << std::endl;
        return false;
    }

    // Enable foreign keys
    char* err = nullptr;
    sqlite3_exec(db_, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &err);
    if (err) { sqlite3_free(err); err = nullptr; }

    // Check if tables already exist
    sqlite3_stmt* stmt = nullptr;
    const char* checkSql = "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='users';";
    bool tablesExist = false;
    if (sqlite3_prepare_v2(db_, checkSql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            tablesExist = (sqlite3_column_int(stmt, 0) > 0);
        }
        sqlite3_finalize(stmt);
    }

    if (!tablesExist) {
        std::cout << "[DatabaseManager] Initializing schema from: " << schemaPath << std::endl;
        if (!executeSqlScript(schemaPath)) {
            std::cerr << "[DatabaseManager] Failed to apply schema" << std::endl;
            return false;
        }

        std::cout << "[DatabaseManager] Seeding initial data from: " << seedPath << std::endl;
        executeSqlScript(seedPath);
    }

    std::cout << "[DatabaseManager] Database initialized successfully: " << dbPath << std::endl;
    return true;
}

bool DatabaseManager::executeSqlScript(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[DatabaseManager] Could not open SQL file: " << filePath << std::endl;
        return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string sql = buffer.str();

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "[DatabaseManager] SQL error in script (" << filePath << "): " << (errMsg ? errMsg : "unknown") << std::endl;
        if (errMsg) sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool DatabaseManager::executeSql(const std::string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        if (errMsg) sqlite3_free(errMsg);
        return false;
    }
    return true;
}

// ---------------- USER OPERATIONS ----------------
bool DatabaseManager::createUser(const User& user, int& newId) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char* sql = "INSERT INTO users (name, email, phone, password_hash, role) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, user.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user.email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user.phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, user.passwordHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, user.role.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        newId = static_cast<int>(sqlite3_last_insert_rowid(db_));
        return true;
    }
    return false;
}

std::optional<User> DatabaseManager::findUserByEmail(const std::string& email) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return std::nullopt;

    const char* sql = "SELECT id, name, email, phone, password_hash, role, created_at FROM users WHERE email = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;

    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<User> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        User u;
        u.id = sqlite3_column_int(stmt, 0);
        u.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        u.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        u.phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        u.passwordHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        u.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        u.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        result = u;
    }
    sqlite3_finalize(stmt);
    return result;
}

std::optional<User> DatabaseManager::findUserById(int id) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return std::nullopt;

    const char* sql = "SELECT id, name, email, phone, password_hash, role, created_at FROM users WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;

    sqlite3_bind_int(stmt, 1, id);

    std::optional<User> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        User u;
        u.id = sqlite3_column_int(stmt, 0);
        u.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        u.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        u.phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        u.passwordHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        u.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        u.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        result = u;
    }
    sqlite3_finalize(stmt);
    return result;
}

std::vector<User> DatabaseManager::getAllUsers(const std::string& roleFilter) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    std::vector<User> users;
    if (!db_) return users;

    std::string sql = "SELECT id, name, email, phone, password_hash, role, created_at FROM users";
    if (!roleFilter.empty() && roleFilter != "all") {
        sql += " WHERE role = '" + roleFilter + "'";
    }
    sql += " ORDER BY id DESC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            User u;
            u.id = sqlite3_column_int(stmt, 0);
            u.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            u.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            u.phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            u.passwordHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            u.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            u.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            users.push_back(u);
        }
        sqlite3_finalize(stmt);
    }
    return users;
}

// ---------------- CATEGORY OPERATIONS ----------------
std::vector<Category> DatabaseManager::getAllCategories() {
    std::lock_guard<std::mutex> lock(dbMutex_);
    std::vector<Category> list;
    if (!db_) return list;

    const char* sql = "SELECT c.id, c.name, c.slug, c.icon, c.description, COUNT(p.id) as pcount "
                      "FROM categories c LEFT JOIN products p ON c.id = p.category_id "
                      "GROUP BY c.id ORDER BY c.id ASC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Category cat;
            cat.id = sqlite3_column_int(stmt, 0);
            cat.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            cat.slug = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            cat.icon = sqlite3_column_text(stmt, 3) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)) : "";
            cat.description = sqlite3_column_text(stmt, 4) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) : "";
            cat.productCount = sqlite3_column_int(stmt, 5);
            list.push_back(cat);
        }
        sqlite3_finalize(stmt);
    }
    return list;
}

std::optional<Category> DatabaseManager::getCategoryById(int id) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return std::nullopt;

    const char* sql = "SELECT id, name, slug, icon, description FROM categories WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_int(stmt, 1, id);

    std::optional<Category> res;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Category c;
        c.id = sqlite3_column_int(stmt, 0);
        c.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        c.slug = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        c.icon = sqlite3_column_text(stmt, 3) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)) : "";
        c.description = sqlite3_column_text(stmt, 4) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) : "";
        res = c;
    }
    sqlite3_finalize(stmt);
    return res;
}

// ---------------- PRODUCT OPERATIONS ----------------
std::vector<Product> DatabaseManager::getProducts(const std::string& query, int categoryId, int sellerId, const std::string& sortBy, double minPrice, double maxPrice) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    std::vector<Product> list;
    if (!db_) return list;

    std::string sql = "SELECT p.id, p.name, p.description, p.category_id, c.name as category_name, "
                      "p.price, p.stock_quantity, p.image_url, p.seller_id, u.name as seller_name, "
                      "p.created_at, IFNULL(AVG(r.rating), 0.0) as avg_rating, COUNT(r.id) as rcount "
                      "FROM products p "
                      "JOIN categories c ON p.category_id = c.id "
                      "JOIN users u ON p.seller_id = u.id "
                      "LEFT JOIN reviews r ON p.id = r.product_id "
                      "WHERE 1=1 ";

    if (categoryId > 0) {
        sql += " AND p.category_id = " + std::to_string(categoryId);
    }
    if (sellerId > 0) {
        sql += " AND p.seller_id = " + std::to_string(sellerId);
    }
    if (minPrice > 0.0) {
        sql += " AND p.price >= " + std::to_string(minPrice);
    }
    if (maxPrice > 0.0) {
        sql += " AND p.price <= " + std::to_string(maxPrice);
    }
    if (!query.empty()) {
        std::string qClean;
        for (char ch : query) {
            if (ch == '\'') qClean += "''";
            else qClean += ch;
        }
        sql += " AND (p.name LIKE '%" + qClean + "%' OR p.description LIKE '%" + qClean + "%' OR c.name LIKE '%" + qClean + "%')";
    }

    sql += " GROUP BY p.id ";

    if (sortBy == "price_asc") {
        sql += " ORDER BY p.price ASC";
    } else if (sortBy == "price_desc") {
        sql += " ORDER BY p.price DESC";
    } else if (sortBy == "rating") {
        sql += " ORDER BY avg_rating DESC";
    } else {
        sql += " ORDER BY p.id DESC";
    }

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Product p;
            p.id = sqlite3_column_int(stmt, 0);
            p.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            p.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            p.categoryId = sqlite3_column_int(stmt, 3);
            p.categoryName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            p.price = sqlite3_column_double(stmt, 5);
            p.stockQuantity = sqlite3_column_int(stmt, 6);
            p.imageUrl = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            p.sellerId = sqlite3_column_int(stmt, 8);
            p.sellerName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
            p.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
            p.averageRating = sqlite3_column_double(stmt, 11);
            p.reviewCount = sqlite3_column_int(stmt, 12);
            list.push_back(p);
        }
        sqlite3_finalize(stmt);
    }
    return list;
}

std::optional<Product> DatabaseManager::getProductById(int id) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return std::nullopt;

    const char* sql = "SELECT p.id, p.name, p.description, p.category_id, c.name as category_name, "
                      "p.price, p.stock_quantity, p.image_url, p.seller_id, u.name as seller_name, "
                      "p.created_at, IFNULL(AVG(r.rating), 0.0) as avg_rating, COUNT(r.id) as rcount "
                      "FROM products p "
                      "JOIN categories c ON p.category_id = c.id "
                      "JOIN users u ON p.seller_id = u.id "
                      "LEFT JOIN reviews r ON p.id = r.product_id "
                      "WHERE p.id = ? GROUP BY p.id;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_int(stmt, 1, id);

    std::optional<Product> res;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Product p;
        p.id = sqlite3_column_int(stmt, 0);
        p.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        p.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        p.categoryId = sqlite3_column_int(stmt, 3);
        p.categoryName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        p.price = sqlite3_column_double(stmt, 5);
        p.stockQuantity = sqlite3_column_int(stmt, 6);
        p.imageUrl = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        p.sellerId = sqlite3_column_int(stmt, 8);
        p.sellerName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        p.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        p.averageRating = sqlite3_column_double(stmt, 11);
        p.reviewCount = sqlite3_column_int(stmt, 12);
        res = p;
    }
    sqlite3_finalize(stmt);
    return res;
}

bool DatabaseManager::createProduct(const Product& product, int& newId) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char* sql = "INSERT INTO products (name, description, category_id, price, stock_quantity, image_url, seller_id) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, product.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, product.description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, product.categoryId);
    sqlite3_bind_double(stmt, 4, product.price);
    sqlite3_bind_int(stmt, 5, product.stockQuantity);
    sqlite3_bind_text(stmt, 6, product.imageUrl.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 7, product.sellerId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        newId = static_cast<int>(sqlite3_last_insert_rowid(db_));
        return true;
    }
    return false;
}

bool DatabaseManager::updateProduct(const Product& product) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    std::string sql = "UPDATE products SET name = ?, description = ?, category_id = ?, price = ?, stock_quantity = ?, image_url = ? "
                      "WHERE id = ?";
    if (product.sellerId > 0) {
        sql += " AND seller_id = ?";
    }
    sql += ";";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, product.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, product.description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, product.categoryId);
    sqlite3_bind_double(stmt, 4, product.price);
    sqlite3_bind_int(stmt, 5, product.stockQuantity);
    sqlite3_bind_text(stmt, 6, product.imageUrl.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 7, product.id);
    if (product.sellerId > 0) {
        sqlite3_bind_int(stmt, 8, product.sellerId);
    }

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

bool DatabaseManager::deleteProduct(int id, int sellerId, bool isAdmin) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    std::string sql = "DELETE FROM products WHERE id = ?";
    if (!isAdmin && sellerId > 0) {
        sql += " AND seller_id = ?";
    }
    sql += ";";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, id);
    if (!isAdmin && sellerId > 0) {
        sqlite3_bind_int(stmt, 2, sellerId);
    }

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

bool DatabaseManager::updateStock(int productId, int deltaQuantity) {
    std::string sql = "UPDATE products SET stock_quantity = stock_quantity + (" + std::to_string(deltaQuantity) + ") "
                      "WHERE id = " + std::to_string(productId) + " AND stock_quantity + (" + std::to_string(deltaQuantity) + ") >= 0;";
    return executeSql(sql);
}

// ---------------- CART OPERATIONS ----------------
std::vector<CartItem> DatabaseManager::getCartItems(int userId, double& subtotal, double& total) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    std::vector<CartItem> items;
    subtotal = 0.0;
    total = 0.0;
    if (!db_) return items;

    const char* sql = "SELECT ci.id, ci.user_id, ci.product_id, p.name, p.price, p.image_url, "
                      "p.stock_quantity, ci.quantity, (ci.quantity * p.price) as item_subtotal, ci.created_at "
                      "FROM cart_items ci "
                      "JOIN products p ON ci.product_id = p.id "
                      "WHERE ci.user_id = ? ORDER BY ci.id DESC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, userId);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            CartItem item;
            item.id = sqlite3_column_int(stmt, 0);
            item.userId = sqlite3_column_int(stmt, 1);
            item.productId = sqlite3_column_int(stmt, 2);
            item.productName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            item.price = sqlite3_column_double(stmt, 4);
            item.imageUrl = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            item.availableStock = sqlite3_column_int(stmt, 6);
            item.quantity = sqlite3_column_int(stmt, 7);
            item.subtotal = sqlite3_column_double(stmt, 8);
            item.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));

            subtotal += item.subtotal;
            items.push_back(item);
        }
        sqlite3_finalize(stmt);
    }
    total = subtotal; // Free shipping in Pricilla Mart
    return items;
}

bool DatabaseManager::addToCart(int userId, int productId, int quantity, std::string& errorMessage) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) {
        errorMessage = "Database not connected.";
        return false;
    }

    if (quantity <= 0) {
        errorMessage = "Invalid quantity.";
        return false;
    }

    // Check product stock and current cart quantity
    int currentStock = 0;
    const char* stockSql = "SELECT stock_quantity FROM products WHERE id = ?;";
    sqlite3_stmt* sStmt = nullptr;
    if (sqlite3_prepare_v2(db_, stockSql, -1, &sStmt, nullptr) != SQLITE_OK) {
        errorMessage = "Failed to query product stock.";
        return false;
    }
    sqlite3_bind_int(sStmt, 1, productId);
    if (sqlite3_step(sStmt) == SQLITE_ROW) {
        currentStock = sqlite3_column_int(sStmt, 0);
    } else {
        sqlite3_finalize(sStmt);
        errorMessage = "Product not found.";
        return false;
    }
    sqlite3_finalize(sStmt);

    int existingCartQty = 0;
    const char* cartSql = "SELECT quantity FROM cart_items WHERE user_id = ? AND product_id = ?;";
    sqlite3_stmt* cStmt = nullptr;
    if (sqlite3_prepare_v2(db_, cartSql, -1, &cStmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(cStmt, 1, userId);
        sqlite3_bind_int(cStmt, 2, productId);
        if (sqlite3_step(cStmt) == SQLITE_ROW) {
            existingCartQty = sqlite3_column_int(cStmt, 0);
        }
        sqlite3_finalize(cStmt);
    }

    if (existingCartQty + quantity > currentStock) {
        errorMessage = "Insufficient stock available.";
        return false;
    }

    const char* upsertSql = "INSERT INTO cart_items (user_id, product_id, quantity) VALUES (?, ?, ?) "
                            "ON CONFLICT(user_id, product_id) DO UPDATE SET quantity = quantity + ?;";
    sqlite3_stmt* uStmt = nullptr;
    if (sqlite3_prepare_v2(db_, upsertSql, -1, &uStmt, nullptr) != SQLITE_OK) {
        errorMessage = "Failed to add item to cart.";
        return false;
    }
    sqlite3_bind_int(uStmt, 1, userId);
    sqlite3_bind_int(uStmt, 2, productId);
    sqlite3_bind_int(uStmt, 3, quantity);
    sqlite3_bind_int(uStmt, 4, quantity);

    int rc = sqlite3_step(uStmt);
    sqlite3_finalize(uStmt);
    return rc == SQLITE_DONE;
}

bool DatabaseManager::updateCartQuantity(int userId, int cartItemId, int newQuantity, std::string& errorMessage) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) {
        errorMessage = "Database not connected.";
        return false;
    }

    if (newQuantity <= 0) {
        const char* delSql = "DELETE FROM cart_items WHERE id = ? AND user_id = ?;";
        sqlite3_stmt* dStmt = nullptr;
        if (sqlite3_prepare_v2(db_, delSql, -1, &dStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(dStmt, 1, cartItemId);
            sqlite3_bind_int(dStmt, 2, userId);
            sqlite3_step(dStmt);
            sqlite3_finalize(dStmt);
        }
        return true;
    }

    // Verify stock
    const char* checkSql = "SELECT p.stock_quantity FROM cart_items ci JOIN products p ON ci.product_id = p.id WHERE ci.id = ? AND ci.user_id = ?;";
    sqlite3_stmt* cStmt = nullptr;
    if (sqlite3_prepare_v2(db_, checkSql, -1, &cStmt, nullptr) != SQLITE_OK) {
        errorMessage = "Database error verifying stock.";
        return false;
    }
    sqlite3_bind_int(cStmt, 1, cartItemId);
    sqlite3_bind_int(cStmt, 2, userId);

    int stock = 0;
    if (sqlite3_step(cStmt) == SQLITE_ROW) {
        stock = sqlite3_column_int(cStmt, 0);
    } else {
        sqlite3_finalize(cStmt);
        errorMessage = "Cart item not found.";
        return false;
    }
    sqlite3_finalize(cStmt);

    if (newQuantity > stock) {
        errorMessage = "Insufficient stock available.";
        return false;
    }

    const char* updateSql = "UPDATE cart_items SET quantity = ? WHERE id = ? AND user_id = ?;";
    sqlite3_stmt* uStmt = nullptr;
    if (sqlite3_prepare_v2(db_, updateSql, -1, &uStmt, nullptr) != SQLITE_OK) {
        errorMessage = "Database error updating quantity.";
        return false;
    }
    sqlite3_bind_int(uStmt, 1, newQuantity);
    sqlite3_bind_int(uStmt, 2, cartItemId);
    sqlite3_bind_int(uStmt, 3, userId);

    int rc = sqlite3_step(uStmt);
    sqlite3_finalize(uStmt);
    return rc == SQLITE_DONE;
}

bool DatabaseManager::removeFromCart(int userId, int cartItemId) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char* sql = "DELETE FROM cart_items WHERE id = ? AND user_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, cartItemId);
    sqlite3_bind_int(stmt, 2, userId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

bool DatabaseManager::clearCart(int userId) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char* sql = "DELETE FROM cart_items WHERE user_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, userId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

// ---------------- ORDER OPERATIONS ----------------
bool DatabaseManager::createOrder(int buyerId, const std::string& buyerName, const std::string& email, const std::string& phone, const std::string& address, std::string& outOrderCode, std::string& errorMessage) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) {
        errorMessage = "Database not connected.";
        return false;
    }

    // Step 1: Query Cart Items
    struct TempItem {
        int productId;
        int sellerId;
        int quantity;
        double unitPrice;
        int stock;
    };
    std::vector<TempItem> cartList;
    double totalAmount = 0.0;

    const char* cartSql = "SELECT ci.product_id, p.seller_id, ci.quantity, p.price, p.stock_quantity "
                          "FROM cart_items ci JOIN products p ON ci.product_id = p.id WHERE ci.user_id = ?;";
    sqlite3_stmt* cStmt = nullptr;
    if (sqlite3_prepare_v2(db_, cartSql, -1, &cStmt, nullptr) != SQLITE_OK) {
        errorMessage = "Error reading cart items.";
        return false;
    }
    sqlite3_bind_int(cStmt, 1, buyerId);
    while (sqlite3_step(cStmt) == SQLITE_ROW) {
        TempItem item;
        item.productId = sqlite3_column_int(cStmt, 0);
        item.sellerId = sqlite3_column_int(cStmt, 1);
        item.quantity = sqlite3_column_int(cStmt, 2);
        item.unitPrice = sqlite3_column_double(cStmt, 3);
        item.stock = sqlite3_column_int(cStmt, 4);

        if (item.quantity > item.stock) {
            sqlite3_finalize(cStmt);
            errorMessage = "Insufficient stock available for one or more items.";
            return false;
        }

        totalAmount += (item.quantity * item.unitPrice);
        cartList.push_back(item);
    }
    sqlite3_finalize(cStmt);

    if (cartList.empty()) {
        errorMessage = "Your shopping cart is empty.";
        return false;
    }

    // Step 2: Begin Transaction
    executeSql("BEGIN TRANSACTION;");

    // Generate unique order code (e.g. PRC10001)
    int nextOrderNum = 10001;
    const char* maxSql = "SELECT IFNULL(MAX(id), 0) + 10001 FROM orders;";
    sqlite3_stmt* mStmt = nullptr;
    if (sqlite3_prepare_v2(db_, maxSql, -1, &mStmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(mStmt) == SQLITE_ROW) {
            nextOrderNum = sqlite3_column_int(mStmt, 0);
        }
        sqlite3_finalize(mStmt);
    }
    outOrderCode = "PRC" + std::to_string(nextOrderNum);

    // Step 3: Insert Order
    const char* oSql = "INSERT INTO orders (order_code, buyer_id, total_amount, buyer_name, email, phone, delivery_address, status) "
                       "VALUES (?, ?, ?, ?, ?, ?, ?, 'Confirmed');";
    sqlite3_stmt* oStmt = nullptr;
    if (sqlite3_prepare_v2(db_, oSql, -1, &oStmt, nullptr) != SQLITE_OK) {
        executeSql("ROLLBACK;");
        errorMessage = "Failed to prepare order insertion.";
        return false;
    }
    sqlite3_bind_text(oStmt, 1, outOrderCode.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(oStmt, 2, buyerId);
    sqlite3_bind_double(oStmt, 3, totalAmount);
    sqlite3_bind_text(oStmt, 4, buyerName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(oStmt, 5, email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(oStmt, 6, phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(oStmt, 7, address.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(oStmt) != SQLITE_DONE) {
        sqlite3_finalize(oStmt);
        executeSql("ROLLBACK;");
        errorMessage = "Failed to create order record.";
        return false;
    }
    int orderId = static_cast<int>(sqlite3_last_insert_rowid(db_));
    sqlite3_finalize(oStmt);

    // Step 4: Insert Order Items and Decrement Stock
    const char* oiSql = "INSERT INTO order_items (order_id, product_id, seller_id, quantity, unit_price, subtotal) "
                        "VALUES (?, ?, ?, ?, ?, ?);";
    const char* stSql = "UPDATE products SET stock_quantity = stock_quantity - ? WHERE id = ?;";

    for (const auto& item : cartList) {
        // Insert order item
        sqlite3_stmt* oiStmt = nullptr;
        if (sqlite3_prepare_v2(db_, oiSql, -1, &oiStmt, nullptr) != SQLITE_OK) {
            executeSql("ROLLBACK;");
            errorMessage = "Failed to insert order item.";
            return false;
        }
        sqlite3_bind_int(oiStmt, 1, orderId);
        sqlite3_bind_int(oiStmt, 2, item.productId);
        sqlite3_bind_int(oiStmt, 3, item.sellerId);
        sqlite3_bind_int(oiStmt, 4, item.quantity);
        sqlite3_bind_double(oiStmt, 5, item.unitPrice);
        sqlite3_bind_double(oiStmt, 6, item.quantity * item.unitPrice);
        sqlite3_step(oiStmt);
        sqlite3_finalize(oiStmt);

        // Decrement stock
        sqlite3_stmt* sDecStmt = nullptr;
        if (sqlite3_prepare_v2(db_, stSql, -1, &sDecStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(sDecStmt, 1, item.quantity);
            sqlite3_bind_int(sDecStmt, 2, item.productId);
            sqlite3_step(sDecStmt);
            sqlite3_finalize(sDecStmt);
        }
    }

    // Step 5: Clear purchased items from user's cart
    const char* clrSql = "DELETE FROM cart_items WHERE user_id = ?;";
    sqlite3_stmt* clrStmt = nullptr;
    if (sqlite3_prepare_v2(db_, clrSql, -1, &clrStmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(clrStmt, 1, buyerId);
        sqlite3_step(clrStmt);
        sqlite3_finalize(clrStmt);
    }

    executeSql("COMMIT;");
    return true;
}

std::vector<Order> DatabaseManager::getOrdersByBuyer(int buyerId) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    std::vector<Order> orders;
    if (!db_) return orders;

    const char* sql = "SELECT id, order_code, buyer_id, total_amount, buyer_name, email, phone, "
                      "delivery_address, status, created_at FROM orders WHERE buyer_id = ? ORDER BY id DESC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, buyerId);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Order o;
            o.id = sqlite3_column_int(stmt, 0);
            o.orderCode = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            o.buyerId = sqlite3_column_int(stmt, 2);
            o.totalAmount = sqlite3_column_double(stmt, 3);
            o.buyerName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            o.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            o.phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            o.deliveryAddress = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            o.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
            o.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
            orders.push_back(o);
        }
        sqlite3_finalize(stmt);
    }

    // Populate order items
    const char* itemSql = "SELECT oi.id, oi.order_id, oi.product_id, p.name, p.image_url, oi.seller_id, "
                          "u.name as seller_name, oi.quantity, oi.unit_price, oi.subtotal "
                          "FROM order_items oi "
                          "JOIN products p ON oi.product_id = p.id "
                          "JOIN users u ON oi.seller_id = u.id "
                          "WHERE oi.order_id = ?;";
    for (auto& o : orders) {
        sqlite3_stmt* iStmt = nullptr;
        if (sqlite3_prepare_v2(db_, itemSql, -1, &iStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(iStmt, 1, o.id);
            while (sqlite3_step(iStmt) == SQLITE_ROW) {
                OrderItem it;
                it.id = sqlite3_column_int(iStmt, 0);
                it.orderId = sqlite3_column_int(iStmt, 1);
                it.productId = sqlite3_column_int(iStmt, 2);
                it.productName = reinterpret_cast<const char*>(sqlite3_column_text(iStmt, 3));
                it.imageUrl = reinterpret_cast<const char*>(sqlite3_column_text(iStmt, 4));
                it.sellerId = sqlite3_column_int(iStmt, 5);
                it.sellerName = reinterpret_cast<const char*>(sqlite3_column_text(iStmt, 6));
                it.quantity = sqlite3_column_int(iStmt, 7);
                it.unitPrice = sqlite3_column_double(iStmt, 8);
                it.subtotal = sqlite3_column_double(iStmt, 9);
                o.items.push_back(it);
            }
            sqlite3_finalize(iStmt);
        }
    }
    return orders;
}

std::vector<Order> DatabaseManager::getOrdersBySeller(int sellerId) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    std::vector<Order> orders;
    if (!db_) return orders;

    const char* sql = "SELECT DISTINCT o.id, o.order_code, o.buyer_id, o.total_amount, o.buyer_name, o.email, o.phone, "
                      "o.delivery_address, o.status, o.created_at FROM orders o "
                      "JOIN order_items oi ON o.id = oi.order_id WHERE oi.seller_id = ? ORDER BY o.id DESC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, sellerId);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Order o;
            o.id = sqlite3_column_int(stmt, 0);
            o.orderCode = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            o.buyerId = sqlite3_column_int(stmt, 2);
            o.totalAmount = sqlite3_column_double(stmt, 3);
            o.buyerName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            o.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            o.phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            o.deliveryAddress = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            o.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
            o.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
            orders.push_back(o);
        }
        sqlite3_finalize(stmt);
    }

    // Populate seller's items for each order
    const char* itemSql = "SELECT oi.id, oi.order_id, oi.product_id, p.name, p.image_url, oi.seller_id, "
                          "u.name as seller_name, oi.quantity, oi.unit_price, oi.subtotal "
                          "FROM order_items oi "
                          "JOIN products p ON oi.product_id = p.id "
                          "JOIN users u ON oi.seller_id = u.id "
                          "WHERE oi.order_id = ? AND oi.seller_id = ?;";
    for (auto& o : orders) {
        sqlite3_stmt* iStmt = nullptr;
        if (sqlite3_prepare_v2(db_, itemSql, -1, &iStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(iStmt, 1, o.id);
            sqlite3_bind_int(iStmt, 2, sellerId);
            while (sqlite3_step(iStmt) == SQLITE_ROW) {
                OrderItem it;
                it.id = sqlite3_column_int(iStmt, 0);
                it.orderId = sqlite3_column_int(iStmt, 1);
                it.productId = sqlite3_column_int(iStmt, 2);
                it.productName = reinterpret_cast<const char*>(sqlite3_column_text(iStmt, 3));
                it.imageUrl = reinterpret_cast<const char*>(sqlite3_column_text(iStmt, 4));
                it.sellerId = sqlite3_column_int(iStmt, 5);
                it.sellerName = reinterpret_cast<const char*>(sqlite3_column_text(iStmt, 6));
                it.quantity = sqlite3_column_int(iStmt, 7);
                it.unitPrice = sqlite3_column_double(iStmt, 8);
                it.subtotal = sqlite3_column_double(iStmt, 9);
                o.items.push_back(it);
            }
            sqlite3_finalize(iStmt);
        }
    }
    return orders;
}

std::vector<Order> DatabaseManager::getAllOrders() {
    std::lock_guard<std::mutex> lock(dbMutex_);
    std::vector<Order> orders;
    if (!db_) return orders;

    const char* sql = "SELECT id, order_code, buyer_id, total_amount, buyer_name, email, phone, "
                      "delivery_address, status, created_at FROM orders ORDER BY id DESC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Order o;
            o.id = sqlite3_column_int(stmt, 0);
            o.orderCode = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            o.buyerId = sqlite3_column_int(stmt, 2);
            o.totalAmount = sqlite3_column_double(stmt, 3);
            o.buyerName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            o.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            o.phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            o.deliveryAddress = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            o.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
            o.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
            orders.push_back(o);
        }
        sqlite3_finalize(stmt);
    }

    // Populate order items
    const char* itemSql = "SELECT oi.id, oi.order_id, oi.product_id, p.name, p.image_url, oi.seller_id, "
                          "u.name as seller_name, oi.quantity, oi.unit_price, oi.subtotal "
                          "FROM order_items oi "
                          "JOIN products p ON oi.product_id = p.id "
                          "JOIN users u ON oi.seller_id = u.id "
                          "WHERE oi.order_id = ?;";
    for (auto& o : orders) {
        sqlite3_stmt* iStmt = nullptr;
        if (sqlite3_prepare_v2(db_, itemSql, -1, &iStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(iStmt, 1, o.id);
            while (sqlite3_step(iStmt) == SQLITE_ROW) {
                OrderItem it;
                it.id = sqlite3_column_int(iStmt, 0);
                it.orderId = sqlite3_column_int(iStmt, 1);
                it.productId = sqlite3_column_int(iStmt, 2);
                it.productName = reinterpret_cast<const char*>(sqlite3_column_text(iStmt, 3));
                it.imageUrl = reinterpret_cast<const char*>(sqlite3_column_text(iStmt, 4));
                it.sellerId = sqlite3_column_int(iStmt, 5);
                it.sellerName = reinterpret_cast<const char*>(sqlite3_column_text(iStmt, 6));
                it.quantity = sqlite3_column_int(iStmt, 7);
                it.unitPrice = sqlite3_column_double(iStmt, 8);
                it.subtotal = sqlite3_column_double(iStmt, 9);
                o.items.push_back(it);
            }
            sqlite3_finalize(iStmt);
        }
    }
    return orders;
}

std::optional<Order> DatabaseManager::getOrderById(int orderId) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return std::nullopt;

    const char* sql = "SELECT id, order_code, buyer_id, total_amount, buyer_name, email, phone, "
                      "delivery_address, status, created_at FROM orders WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_int(stmt, 1, orderId);

    std::optional<Order> res;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Order o;
        o.id = sqlite3_column_int(stmt, 0);
        o.orderCode = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        o.buyerId = sqlite3_column_int(stmt, 2);
        o.totalAmount = sqlite3_column_double(stmt, 3);
        o.buyerName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        o.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        o.phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        o.deliveryAddress = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        o.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        o.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        res = o;
    }
    sqlite3_finalize(stmt);

    if (res) {
        const char* itemSql = "SELECT oi.id, oi.order_id, oi.product_id, p.name, p.image_url, oi.seller_id, "
                              "u.name as seller_name, oi.quantity, oi.unit_price, oi.subtotal "
                              "FROM order_items oi "
                              "JOIN products p ON oi.product_id = p.id "
                              "JOIN users u ON oi.seller_id = u.id "
                              "WHERE oi.order_id = ?;";
        sqlite3_stmt* iStmt = nullptr;
        if (sqlite3_prepare_v2(db_, itemSql, -1, &iStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(iStmt, 1, res->id);
            while (sqlite3_step(iStmt) == SQLITE_ROW) {
                OrderItem it;
                it.id = sqlite3_column_int(iStmt, 0);
                it.orderId = sqlite3_column_int(iStmt, 1);
                it.productId = sqlite3_column_int(iStmt, 2);
                it.productName = reinterpret_cast<const char*>(sqlite3_column_text(iStmt, 3));
                it.imageUrl = reinterpret_cast<const char*>(sqlite3_column_text(iStmt, 4));
                it.sellerId = sqlite3_column_int(iStmt, 5);
                it.sellerName = reinterpret_cast<const char*>(sqlite3_column_text(iStmt, 6));
                it.quantity = sqlite3_column_int(iStmt, 7);
                it.unitPrice = sqlite3_column_double(iStmt, 8);
                it.subtotal = sqlite3_column_double(iStmt, 9);
                res->items.push_back(it);
            }
            sqlite3_finalize(iStmt);
        }
    }
    return res;
}

std::optional<Order> DatabaseManager::getOrderByCode(const std::string& orderCode) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return std::nullopt;

    const char* sql = "SELECT id, order_code, buyer_id, total_amount, buyer_name, email, phone, "
                      "delivery_address, status, created_at FROM orders WHERE order_code = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_text(stmt, 1, orderCode.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<Order> res;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Order o;
        o.id = sqlite3_column_int(stmt, 0);
        o.orderCode = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        o.buyerId = sqlite3_column_int(stmt, 2);
        o.totalAmount = sqlite3_column_double(stmt, 3);
        o.buyerName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        o.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        o.phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        o.deliveryAddress = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        o.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        o.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        res = o;
    }
    sqlite3_finalize(stmt);
    return res;
}

bool DatabaseManager::updateOrderStatus(int orderId, const std::string& status, int sellerId, bool isAdmin) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    // Validate status value
    static const std::vector<std::string> validStatuses = {"Pending", "Confirmed", "Shipped", "Delivered", "Cancelled"};
    if (std::find(validStatuses.begin(), validStatuses.end(), status) == validStatuses.end()) {
        return false;
    }

    if (!isAdmin && sellerId > 0) {
        // Verify this seller is part of this order
        const char* vSql = "SELECT COUNT(*) FROM order_items WHERE order_id = ? AND seller_id = ?;";
        sqlite3_stmt* vStmt = nullptr;
        bool authorized = false;
        if (sqlite3_prepare_v2(db_, vSql, -1, &vStmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(vStmt, 1, orderId);
            sqlite3_bind_int(vStmt, 2, sellerId);
            if (sqlite3_step(vStmt) == SQLITE_ROW) {
                authorized = (sqlite3_column_int(vStmt, 0) > 0);
            }
            sqlite3_finalize(vStmt);
        }
        if (!authorized) return false;
    }

    const char* sql = "UPDATE orders SET status = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, orderId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

// ---------------- REVIEW OPERATIONS ----------------
bool DatabaseManager::hasPurchasedProduct(int buyerId, int productId) {
    const char* sql = "SELECT COUNT(*) FROM order_items oi JOIN orders o ON oi.order_id = o.id "
                      "WHERE o.buyer_id = ? AND oi.product_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    bool purchased = false;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, buyerId);
        sqlite3_bind_int(stmt, 2, productId);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            purchased = (sqlite3_column_int(stmt, 0) > 0);
        }
        sqlite3_finalize(stmt);
    }
    return purchased;
}

bool DatabaseManager::addReview(const Review& review, std::string& errorMessage) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) {
        errorMessage = "Database not connected.";
        return false;
    }

    if (review.rating < 1 || review.rating > 5) {
        errorMessage = "Rating must be between 1 and 5 stars.";
        return false;
    }
    if (review.comment.empty()) {
        errorMessage = "Review comment cannot be empty.";
        return false;
    }

    const char* sql = "INSERT INTO reviews (product_id, buyer_id, rating, comment) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        errorMessage = "Failed to prepare review insertion.";
        return false;
    }

    sqlite3_bind_int(stmt, 1, review.productId);
    sqlite3_bind_int(stmt, 2, review.buyerId);
    sqlite3_bind_int(stmt, 3, review.rating);
    sqlite3_bind_text(stmt, 4, review.comment.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        errorMessage = "Failed to save review.";
        return false;
    }
    return true;
}

std::vector<Review> DatabaseManager::getReviewsByProduct(int productId, double& avgRating, int& count) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    std::vector<Review> reviews;
    avgRating = 0.0;
    count = 0;
    if (!db_) return reviews;

    const char* sql = "SELECT r.id, r.product_id, p.name as product_name, r.buyer_id, u.name as buyer_name, "
                      "r.rating, r.comment, r.created_at "
                      "FROM reviews r "
                      "JOIN users u ON r.buyer_id = u.id "
                      "JOIN products p ON r.product_id = p.id "
                      "WHERE r.product_id = ? ORDER BY r.id DESC;";
    sqlite3_stmt* stmt = nullptr;
    double sum = 0.0;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, productId);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Review r;
            r.id = sqlite3_column_int(stmt, 0);
            r.productId = sqlite3_column_int(stmt, 1);
            r.productName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            r.buyerId = sqlite3_column_int(stmt, 3);
            r.buyerName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            r.rating = sqlite3_column_int(stmt, 5);
            r.comment = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            r.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));

            sum += r.rating;
            reviews.push_back(r);
        }
        sqlite3_finalize(stmt);
    }
    count = static_cast<int>(reviews.size());
    if (count > 0) {
        avgRating = sum / count;
    }
    return reviews;
}

// ---------------- ADMIN & SELLER ANALYTICS ----------------
AdminStats DatabaseManager::getAdminStats() {
    std::lock_guard<std::mutex> lock(dbMutex_);
    AdminStats stats;
    if (!db_) return stats;

    const char* sql = "SELECT "
                      "(SELECT COUNT(*) FROM users), "
                      "(SELECT COUNT(*) FROM users WHERE role = 'buyer'), "
                      "(SELECT COUNT(*) FROM users WHERE role = 'seller'), "
                      "(SELECT COUNT(*) FROM products), "
                      "(SELECT COUNT(*) FROM orders), "
                      "(SELECT IFNULL(SUM(total_amount), 0.0) FROM orders WHERE status != 'Cancelled');";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            stats.totalUsers = sqlite3_column_int(stmt, 0);
            stats.totalBuyers = sqlite3_column_int(stmt, 1);
            stats.totalSellers = sqlite3_column_int(stmt, 2);
            stats.totalProducts = sqlite3_column_int(stmt, 3);
            stats.totalOrders = sqlite3_column_int(stmt, 4);
            stats.totalSales = sqlite3_column_double(stmt, 5);
        }
        sqlite3_finalize(stmt);
    }
    return stats;
}

SellerStats DatabaseManager::getSellerStats(int sellerId) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    SellerStats stats;
    if (!db_) return stats;

    const char* sql = "SELECT "
                      "(SELECT COUNT(*) FROM products WHERE seller_id = ?), "
                      "(SELECT COUNT(DISTINCT order_id) FROM order_items WHERE seller_id = ?), "
                      "(SELECT IFNULL(SUM(subtotal), 0.0) FROM order_items WHERE seller_id = ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, sellerId);
        sqlite3_bind_int(stmt, 2, sellerId);
        sqlite3_bind_int(stmt, 3, sellerId);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            stats.totalProducts = sqlite3_column_int(stmt, 0);
            stats.totalOrders = sqlite3_column_int(stmt, 1);
            stats.totalSales = sqlite3_column_double(stmt, 2);
        }
        sqlite3_finalize(stmt);
    }
    return stats;
}

// ---------------- CHATBOT SEARCH HELPER ----------------
std::vector<Product> DatabaseManager::searchProductsForChatbot(const std::string& query, int limit) {
    std::vector<Product> results;
    std::string cleanQuery;
    for (char c : query) {
        cleanQuery += std::tolower(c);
    }

    // Split words
    std::istringstream iss(cleanQuery);
    std::string token;
    std::vector<std::string> tokens;
    while (iss >> token) {
        if (token.length() > 2) {
            tokens.push_back(token);
        }
    }

    auto allProducts = getProducts();
    for (const auto& prod : allProducts) {
        std::string pName = prod.name;
        std::string pDesc = prod.description;
        std::string pCat = prod.categoryName;
        std::transform(pName.begin(), pName.end(), pName.begin(), ::tolower);
        std::transform(pDesc.begin(), pDesc.end(), pDesc.begin(), ::tolower);
        std::transform(pCat.begin(), pCat.end(), pCat.begin(), ::tolower);

        bool matched = false;
        for (const auto& t : tokens) {
            if (pName.find(t) != std::string::npos ||
                pDesc.find(t) != std::string::npos ||
                pCat.find(t) != std::string::npos) {
                matched = true;
                break;
            }
        }
        if (matched) {
            results.push_back(prod);
            if (static_cast<int>(results.size()) >= limit) break;
        }
    }

    return results;
}

} // namespace PricillaMart
