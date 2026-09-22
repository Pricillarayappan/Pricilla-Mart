#include "CartController.h"
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

    std::string serializeCart(const std::vector<CartItem>& items, double subtotal, double total) {
        std::ostringstream ss;
        ss << "{"
           << "\"subtotal\":" << subtotal << ","
           << "\"total\":" << total << ","
           << "\"itemCount\":" << items.size() << ","
           << "\"items\":[";
        for (size_t i = 0; i < items.size(); ++i) {
            const auto& it = items[i];
            if (i > 0) ss << ",";
            ss << "{"
               << "\"id\":" << it.id << ","
               << "\"productId\":" << it.productId << ","
               << "\"productName\":\"" << ResponseUtil::escapeJson(it.productName) << "\","
               << "\"price\":" << it.price << ","
               << "\"imageUrl\":\"" << ResponseUtil::escapeJson(it.imageUrl) << "\","
               << "\"availableStock\":" << it.availableStock << ","
               << "\"quantity\":" << it.quantity << ","
               << "\"subtotal\":" << it.subtotal
               << "}";
        }
        ss << "]}";
        return ss.str();
    }
}

HttpResponseData CartController::handleGetCart(const TokenPayload& user) {
    HttpResponseData resp;
    if (!user.isValid) {
        resp.status = 401;
        resp.body = ResponseUtil::error("Unauthorized. Please log in to view cart.", 401);
        return resp;
    }

    auto& db = DatabaseManager::getInstance();
    double subtotal = 0.0, total = 0.0;
    auto items = db.getCartItems(user.userId, subtotal, total);

    resp.status = 200;
    resp.body = ResponseUtil::success("Cart fetched successfully.", serializeCart(items, subtotal, total));
    return resp;
}

HttpResponseData CartController::handleAddToCart(const std::string& body, const TokenPayload& user) {
    HttpResponseData resp;
    if (!user.isValid) {
        resp.status = 401;
        resp.body = ResponseUtil::error("Unauthorized. Please log in to add items to cart.", 401);
        return resp;
    }

    std::string pIdStr = extractJsonValue(body, "productId");
    std::string qStr = extractJsonValue(body, "quantity");

    if (pIdStr.empty()) {
        resp.status = 400;
        resp.body = ResponseUtil::error("Product ID is required.", 400);
        return resp;
    }

    int productId = 0;
    int quantity = 1;
    try {
        productId = std::stoi(pIdStr);
        if (!qStr.empty()) {
            quantity = std::stoi(qStr);
        }
    } catch (...) {
        resp.status = 400;
        resp.body = ResponseUtil::error("Invalid numeric product ID or quantity.", 400);
        return resp;
    }

    if (quantity <= 0) {
        resp.status = 400;
        resp.body = ResponseUtil::error("Quantity must be at least 1.", 400);
        return resp;
    }

    auto& db = DatabaseManager::getInstance();
    std::string errMsg;
    if (!db.addToCart(user.userId, productId, quantity, errMsg)) {
        resp.status = 400;
        resp.body = ResponseUtil::error(errMsg.empty() ? "Failed to add to cart." : errMsg, 400);
        return resp;
    }

    double subtotal = 0.0, total = 0.0;
    auto items = db.getCartItems(user.userId, subtotal, total);
    resp.status = 200;
    resp.body = ResponseUtil::success("Product added to cart.", serializeCart(items, subtotal, total));
    return resp;
}

HttpResponseData CartController::handleUpdateQuantity(int cartItemId, const std::string& body, const TokenPayload& user) {
    HttpResponseData resp;
    if (!user.isValid) {
        resp.status = 401;
        resp.body = ResponseUtil::error("Unauthorized. Please log in.", 401);
        return resp;
    }

    std::string qStr = extractJsonValue(body, "quantity");
    if (qStr.empty()) {
        resp.status = 400;
        resp.body = ResponseUtil::error("Quantity is required.", 400);
        return resp;
    }

    int quantity = 0;
    try {
        quantity = std::stoi(qStr);
    } catch (...) {
        resp.status = 400;
        resp.body = ResponseUtil::error("Invalid quantity value.", 400);
        return resp;
    }

    auto& db = DatabaseManager::getInstance();
    std::string errMsg;
    if (!db.updateCartQuantity(user.userId, cartItemId, quantity, errMsg)) {
        resp.status = 400;
        resp.body = ResponseUtil::error(errMsg.empty() ? "Failed to update cart quantity." : errMsg, 400);
        return resp;
    }

    double subtotal = 0.0, total = 0.0;
    auto items = db.getCartItems(user.userId, subtotal, total);
    resp.status = 200;
    resp.body = ResponseUtil::success("Cart updated successfully.", serializeCart(items, subtotal, total));
    return resp;
}

HttpResponseData CartController::handleRemoveItem(int cartItemId, const TokenPayload& user) {
    HttpResponseData resp;
    if (!user.isValid) {
        resp.status = 401;
        resp.body = ResponseUtil::error("Unauthorized. Please log in.", 401);
        return resp;
    }

    auto& db = DatabaseManager::getInstance();
    if (!db.removeFromCart(user.userId, cartItemId)) {
        resp.status = 500;
        resp.body = ResponseUtil::error("Failed to remove item from cart.", 500);
        return resp;
    }

    double subtotal = 0.0, total = 0.0;
    auto items = db.getCartItems(user.userId, subtotal, total);
    resp.status = 200;
    resp.body = ResponseUtil::success("Item removed from cart.", serializeCart(items, subtotal, total));
    return resp;
}

HttpResponseData CartController::handleClearCart(const TokenPayload& user) {
    HttpResponseData resp;
    if (!user.isValid) {
        resp.status = 401;
        resp.body = ResponseUtil::error("Unauthorized. Please log in.", 401);
        return resp;
    }

    auto& db = DatabaseManager::getInstance();
    db.clearCart(user.userId);
    resp.status = 200;
    resp.body = ResponseUtil::success("Cart cleared successfully.", "{\"items\":[],\"subtotal\":0.0,\"total\":0.0,\"itemCount\":0}");
    return resp;
}

} // namespace PricillaMart
