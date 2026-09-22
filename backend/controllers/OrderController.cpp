#include "OrderController.h"
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

    std::string serializeOrder(const Order& o) {
        std::ostringstream ss;
        ss << "{"
           << "\"id\":" << o.id << ","
           << "\"orderCode\":\"" << ResponseUtil::escapeJson(o.orderCode) << "\","
           << "\"buyerId\":" << o.buyerId << ","
           << "\"buyerName\":\"" << ResponseUtil::escapeJson(o.buyerName) << "\","
           << "\"email\":\"" << ResponseUtil::escapeJson(o.email) << "\","
           << "\"phone\":\"" << ResponseUtil::escapeJson(o.phone) << "\","
           << "\"deliveryAddress\":\"" << ResponseUtil::escapeJson(o.deliveryAddress) << "\","
           << "\"totalAmount\":" << o.totalAmount << ","
           << "\"status\":\"" << ResponseUtil::escapeJson(o.status) << "\","
           << "\"createdAt\":\"" << ResponseUtil::escapeJson(o.createdAt) << "\","
           << "\"items\":[";
        for (size_t i = 0; i < o.items.size(); ++i) {
            const auto& it = o.items[i];
            if (i > 0) ss << ",";
            ss << "{"
               << "\"id\":" << it.id << ","
               << "\"productId\":" << it.productId << ","
               << "\"productName\":\"" << ResponseUtil::escapeJson(it.productName) << "\","
               << "\"imageUrl\":\"" << ResponseUtil::escapeJson(it.imageUrl) << "\","
               << "\"sellerId\":" << it.sellerId << ","
               << "\"sellerName\":\"" << ResponseUtil::escapeJson(it.sellerName) << "\","
               << "\"quantity\":" << it.quantity << ","
               << "\"unitPrice\":" << it.unitPrice << ","
               << "\"subtotal\":" << it.subtotal
               << "}";
        }
        ss << "]}";
        return ss.str();
    }
}

HttpResponseData OrderController::handleCreateOrder(const std::string& body, const TokenPayload& user) {
    HttpResponseData resp;
    if (!user.isValid) {
        resp.status = 401;
        resp.body = ResponseUtil::error("Unauthorized. Please log in to complete checkout.", 401);
        return resp;
    }

    std::string buyerName = extractJsonValue(body, "buyerName");
    std::string email = extractJsonValue(body, "email");
    std::string phone = extractJsonValue(body, "phone");
    std::string address = extractJsonValue(body, "deliveryAddress");

    if (buyerName.empty() || email.empty() || phone.empty() || address.empty()) {
        resp.status = 400;
        resp.body = ResponseUtil::error("All checkout fields (Name, Email, Phone, Delivery Address) are required.", 400);
        return resp;
    }

    auto& db = DatabaseManager::getInstance();
    std::string orderCode;
    std::string errMsg;

    if (!db.createOrder(user.userId, buyerName, email, phone, address, orderCode, errMsg)) {
        resp.status = 400;
        resp.body = ResponseUtil::error(errMsg.empty() ? "Order checkout failed." : errMsg, 400);
        return resp;
    }

    auto orderOpt = db.getOrderByCode(orderCode);
    std::string orderData = orderOpt.has_value() ? serializeOrder(orderOpt.value()) : ("{\"orderCode\":\"" + orderCode + "\"}");

    resp.status = 201;
    resp.body = ResponseUtil::success("Order placed successfully. Thank you for shopping with Pricilla Mart!", orderData);
    return resp;
}

HttpResponseData OrderController::handleGetOrders(const TokenPayload& user) {
    HttpResponseData resp;
    if (!user.isValid) {
        resp.status = 401;
        resp.body = ResponseUtil::error("Unauthorized. Please log in.", 401);
        return resp;
    }

    auto& db = DatabaseManager::getInstance();
    std::vector<Order> orders;

    if (user.role == "admin") {
        orders = db.getAllOrders();
    } else if (user.role == "seller") {
        orders = db.getOrdersBySeller(user.userId);
    } else {
        orders = db.getOrdersByBuyer(user.userId);
    }

    std::ostringstream ss;
    ss << "[";
    for (size_t i = 0; i < orders.size(); ++i) {
        if (i > 0) ss << ",";
        ss << serializeOrder(orders[i]);
    }
    ss << "]";

    resp.status = 200;
    resp.body = ResponseUtil::success("Orders retrieved successfully.", ss.str());
    return resp;
}

HttpResponseData OrderController::handleGetOrderById(int id, const TokenPayload& user) {
    HttpResponseData resp;
    if (!user.isValid) {
        resp.status = 401;
        resp.body = ResponseUtil::error("Unauthorized. Please log in.", 401);
        return resp;
    }

    auto& db = DatabaseManager::getInstance();
    auto oOpt = db.getOrderById(id);
    if (!oOpt.has_value()) {
        resp.status = 404;
        resp.body = ResponseUtil::error("Order not found.", 404);
        return resp;
    }

    const auto& order = oOpt.value();
    // Authorization check
    bool canView = (user.role == "admin" || order.buyerId == user.userId);
    if (!canView && user.role == "seller") {
        for (const auto& it : order.items) {
            if (it.sellerId == user.userId) {
                canView = true;
                break;
            }
        }
    }

    if (!canView) {
        resp.status = 403;
        resp.body = ResponseUtil::error("Forbidden: You do not have permission to view this order.", 403);
        return resp;
    }

    resp.status = 200;
    resp.body = ResponseUtil::success("Order details fetched.", serializeOrder(order));
    return resp;
}

HttpResponseData OrderController::handleUpdateStatus(int id, const std::string& body, const TokenPayload& user) {
    HttpResponseData resp;
    if (!user.isValid) {
        resp.status = 401;
        resp.body = ResponseUtil::error("Unauthorized. Please log in.", 401);
        return resp;
    }

    if (user.role != "seller" && user.role != "admin") {
        resp.status = 403;
        resp.body = ResponseUtil::error("Forbidden: Only sellers and administrators can update order status.", 403);
        return resp;
    }

    std::string status = extractJsonValue(body, "status");
    if (status.empty()) {
        resp.status = 400;
        resp.body = ResponseUtil::error("New order status is required.", 400);
        return resp;
    }

    auto& db = DatabaseManager::getInstance();
    bool isAdmin = (user.role == "admin");
    if (!db.updateOrderStatus(id, status, user.userId, isAdmin)) {
        resp.status = 400;
        resp.body = ResponseUtil::error("Failed to update order status. Please verify permissions and valid status values.", 400);
        return resp;
    }

    auto updated = db.getOrderById(id);
    std::string orderData = updated.has_value() ? serializeOrder(updated.value()) : "{}";

    resp.status = 200;
    resp.body = ResponseUtil::success("Order status updated successfully to " + status + ".", orderData);
    return resp;
}

} // namespace PricillaMart
