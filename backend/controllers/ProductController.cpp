#include "ProductController.h"
#include "database/DatabaseManager.h"
#include "utils/ResponseUtil.h"
#include <sstream>
#include <regex>
#include <iostream>

namespace PricillaMart {

namespace {
    std::string extractJsonValue(const std::string& json, const std::string& key) {
        std::string pattern = "\"" + key + "\"\\s*:\\s*\"([^\"]*)\"";
        std::regex r(pattern);
        std::smatch m;
        if (std::regex_search(json, m, r) && m.size() > 1) {
            return m[1].str();
        }
        std::string numPattern = "\"" + key + "\"\\s*:\\s*([0-9a-zA-Z._-]+)";
        std::regex rNum(numPattern);
        if (std::regex_search(json, m, rNum) && m.size() > 1) {
            return m[1].str();
        }
        return "";
    }

    std::string serializeProduct(const Product& p) {
        std::ostringstream ss;
        ss << "{"
           << "\"id\":" << p.id << ","
           << "\"name\":\"" << ResponseUtil::escapeJson(p.name) << "\","
           << "\"description\":\"" << ResponseUtil::escapeJson(p.description) << "\","
           << "\"categoryId\":" << p.categoryId << ","
           << "\"categoryName\":\"" << ResponseUtil::escapeJson(p.categoryName) << "\","
           << "\"price\":" << p.price << ","
           << "\"stockQuantity\":" << p.stockQuantity << ","
           << "\"imageUrl\":\"" << ResponseUtil::escapeJson(p.imageUrl) << "\","
           << "\"sellerId\":" << p.sellerId << ","
           << "\"sellerName\":\"" << ResponseUtil::escapeJson(p.sellerName) << "\","
           << "\"averageRating\":" << p.averageRating << ","
           << "\"reviewCount\":" << p.reviewCount << ","
           << "\"createdAt\":\"" << ResponseUtil::escapeJson(p.createdAt) << "\""
           << "}";
        return ss.str();
    }
}

HttpResponseData ProductController::handleGetProducts(const std::map<std::string, std::string>& params) {
    HttpResponseData resp;
    std::string q = "";
    int categoryId = 0;
    int sellerId = 0;
    std::string sort = "";
    double minPrice = 0.0;
    double maxPrice = 0.0;

    auto itQ = params.find("q");
    if (itQ != params.end()) q = itQ->second;

    auto itCat = params.find("category_id");
    if (itCat != params.end() && !itCat->second.empty()) {
        try { categoryId = std::stoi(itCat->second); } catch (...) {}
    }

    auto itSeller = params.find("seller_id");
    if (itSeller != params.end() && !itSeller->second.empty()) {
        try { sellerId = std::stoi(itSeller->second); } catch (...) {}
    }

    auto itSort = params.find("sort");
    if (itSort != params.end()) sort = itSort->second;

    auto itMin = params.find("min_price");
    if (itMin != params.end() && !itMin->second.empty()) {
        try { minPrice = std::stod(itMin->second); } catch (...) {}
    }

    auto itMax = params.find("max_price");
    if (itMax != params.end() && !itMax->second.empty()) {
        try { maxPrice = std::stod(itMax->second); } catch (...) {}
    }

    auto& db = DatabaseManager::getInstance();
    auto products = db.getProducts(q, categoryId, sellerId, sort, minPrice, maxPrice);

    std::ostringstream ss;
    ss << "[";
    for (size_t i = 0; i < products.size(); ++i) {
        if (i > 0) ss << ",";
        ss << serializeProduct(products[i]);
    }
    ss << "]";

    resp.status = 200;
    resp.body = ResponseUtil::success("Products fetched successfully.", ss.str());
    return resp;
}

HttpResponseData ProductController::handleGetProductById(int id) {
    HttpResponseData resp;
    auto& db = DatabaseManager::getInstance();
    auto pOpt = db.getProductById(id);

    if (!pOpt.has_value()) {
        resp.status = 404;
        resp.body = ResponseUtil::error("Product not found.", 404);
        return resp;
    }

    resp.status = 200;
    resp.body = ResponseUtil::success("Product details fetched.", serializeProduct(pOpt.value()));
    return resp;
}

HttpResponseData ProductController::handleCreateProduct(const std::string& body, const TokenPayload& user) {
    HttpResponseData resp;
    if (!user.isValid) {
        resp.status = 401;
        resp.body = ResponseUtil::error("Unauthorized. Please log in.", 401);
        return resp;
    }
    if (user.role != "seller" && user.role != "admin") {
        resp.status = 403;
        resp.body = ResponseUtil::error("Only registered sellers or administrators can add products.", 403);
        return resp;
    }

    std::string name = extractJsonValue(body, "name");
    std::string description = extractJsonValue(body, "description");
    std::string catIdStr = extractJsonValue(body, "categoryId");
    std::string priceStr = extractJsonValue(body, "price");
    std::string stockStr = extractJsonValue(body, "stockQuantity");
    std::string imageUrl = extractJsonValue(body, "imageUrl");

    if (name.empty() || description.empty() || catIdStr.empty() || priceStr.empty() || stockStr.empty()) {
        resp.status = 400;
        resp.body = ResponseUtil::error("All product fields (name, description, category, price, stock) are required.", 400);
        return resp;
    }

    if (imageUrl.empty()) {
        imageUrl = "https://images.unsplash.com/photo-1526170375885-4d8ecf77b99f?w=600&auto=format&fit=crop&q=80";
    }

    int categoryId = 1;
    double price = 0.0;
    int stockQuantity = 0;
    try {
        categoryId = std::stoi(catIdStr);
        price = std::stod(priceStr);
        stockQuantity = std::stoi(stockStr);
    } catch (...) {
        resp.status = 400;
        resp.body = ResponseUtil::error("Invalid numeric format for price, stock, or category.", 400);
        return resp;
    }

    if (price <= 0) {
        resp.status = 400;
        resp.body = ResponseUtil::error("Price must be greater than zero.", 400);
        return resp;
    }
    if (stockQuantity < 0) {
        resp.status = 400;
        resp.body = ResponseUtil::error("Stock quantity cannot be negative.", 400);
        return resp;
    }

    Product prod;
    prod.name = name;
    prod.description = description;
    prod.categoryId = categoryId;
    prod.price = price;
    prod.stockQuantity = stockQuantity;
    prod.imageUrl = imageUrl;
    prod.sellerId = user.userId;

    auto& db = DatabaseManager::getInstance();
    int newId = 0;
    if (!db.createProduct(prod, newId)) {
        resp.status = 500;
        resp.body = ResponseUtil::error("Failed to insert product into database.", 500);
        return resp;
    }

    auto created = db.getProductById(newId);
    resp.status = 201;
    resp.body = ResponseUtil::success("Product created successfully.", serializeProduct(created.value()));
    return resp;
}

HttpResponseData ProductController::handleUpdateProduct(int id, const std::string& body, const TokenPayload& user) {
    HttpResponseData resp;
    if (!user.isValid) {
        resp.status = 401;
        resp.body = ResponseUtil::error("Unauthorized. Please log in.", 401);
        return resp;
    }

    auto& db = DatabaseManager::getInstance();
    auto existingOpt = db.getProductById(id);
    if (!existingOpt.has_value()) {
        resp.status = 404;
        resp.body = ResponseUtil::error("Product not found.", 404);
        return resp;
    }

    const auto& existing = existingOpt.value();
    if (user.role != "admin" && existing.sellerId != user.userId) {
        resp.status = 403;
        resp.body = ResponseUtil::error("Forbidden: You are only allowed to manage your own products.", 403);
        return resp;
    }

    std::string name = extractJsonValue(body, "name");
    std::string description = extractJsonValue(body, "description");
    std::string catIdStr = extractJsonValue(body, "categoryId");
    std::string priceStr = extractJsonValue(body, "price");
    std::string stockStr = extractJsonValue(body, "stockQuantity");
    std::string imageUrl = extractJsonValue(body, "imageUrl");

    Product prod = existing;
    if (!name.empty()) prod.name = name;
    if (!description.empty()) prod.description = description;
    if (!imageUrl.empty()) prod.imageUrl = imageUrl;

    if (!catIdStr.empty()) {
        try { prod.categoryId = std::stoi(catIdStr); } catch (...) {}
    }
    if (!priceStr.empty()) {
        try {
            double p = std::stod(priceStr);
            if (p > 0) prod.price = p;
        } catch (...) {}
    }
    if (!stockStr.empty()) {
        try {
            int s = std::stoi(stockStr);
            if (s >= 0) prod.stockQuantity = s;
        } catch (...) {}
    }

    // For admin, preserve original seller
    if (user.role == "admin") {
        prod.sellerId = 0; // ignores seller check in updateProduct
    } else {
        prod.sellerId = user.userId;
    }

    if (!db.updateProduct(prod)) {
        resp.status = 500;
        resp.body = ResponseUtil::error("Failed to update product details.", 500);
        return resp;
    }

    auto updated = db.getProductById(id);
    resp.status = 200;
    resp.body = ResponseUtil::success("Product updated successfully.", serializeProduct(updated.value()));
    return resp;
}

HttpResponseData ProductController::handleDeleteProduct(int id, const TokenPayload& user) {
    HttpResponseData resp;
    if (!user.isValid) {
        resp.status = 401;
        resp.body = ResponseUtil::error("Unauthorized. Please log in.", 401);
        return resp;
    }

    auto& db = DatabaseManager::getInstance();
    auto existingOpt = db.getProductById(id);
    if (!existingOpt.has_value()) {
        resp.status = 404;
        resp.body = ResponseUtil::error("Product not found.", 404);
        return resp;
    }

    const auto& existing = existingOpt.value();
    bool isAdmin = (user.role == "admin");
    if (!isAdmin && existing.sellerId != user.userId) {
        resp.status = 403;
        resp.body = ResponseUtil::error("Forbidden: You are only allowed to delete your own products.", 403);
        return resp;
    }

    if (!db.deleteProduct(id, user.userId, isAdmin)) {
        resp.status = 500;
        resp.body = ResponseUtil::error("Failed to delete product from database.", 500);
        return resp;
    }

    resp.status = 200;
    resp.body = ResponseUtil::success("Product deleted successfully.");
    return resp;
}

} // namespace PricillaMart
