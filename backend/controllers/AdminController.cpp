#include "AdminController.h"
#include "database/DatabaseManager.h"
#include "utils/ResponseUtil.h"
#include <sstream>

namespace PricillaMart {

HttpResponseData AdminController::handleGetAdminStats(const TokenPayload& user) {
    HttpResponseData resp;
    if (!user.isValid || user.role != "admin") {
        resp.status = 403;
        resp.body = ResponseUtil::error("Forbidden: Admin access required.", 403);
        return resp;
    }

    auto& db = DatabaseManager::getInstance();
    auto stats = db.getAdminStats();

    std::ostringstream ss;
    ss << "{"
       << "\"totalUsers\":" << stats.totalUsers << ","
       << "\"totalBuyers\":" << stats.totalBuyers << ","
       << "\"totalSellers\":" << stats.totalSellers << ","
       << "\"totalProducts\":" << stats.totalProducts << ","
       << "\"totalOrders\":" << stats.totalOrders << ","
       << "\"totalSales\":" << stats.totalSales
       << "}";

    resp.status = 200;
    resp.body = ResponseUtil::success("Admin statistics retrieved.", ss.str());
    return resp;
}

HttpResponseData AdminController::handleGetUsers(const std::map<std::string, std::string>& params, const TokenPayload& user) {
    HttpResponseData resp;
    if (!user.isValid || user.role != "admin") {
        resp.status = 403;
        resp.body = ResponseUtil::error("Forbidden: Admin access required.", 403);
        return resp;
    }

    std::string roleFilter = "";
    auto it = params.find("role");
    if (it != params.end()) roleFilter = it->second;

    auto& db = DatabaseManager::getInstance();
    auto users = db.getAllUsers(roleFilter);

    std::ostringstream ss;
    ss << "[";
    for (size_t i = 0; i < users.size(); ++i) {
        const auto& u = users[i];
        if (i > 0) ss << ",";
        ss << "{"
           << "\"id\":" << u.id << ","
           << "\"name\":\"" << ResponseUtil::escapeJson(u.name) << "\","
           << "\"email\":\"" << ResponseUtil::escapeJson(u.email) << "\","
           << "\"phone\":\"" << ResponseUtil::escapeJson(u.phone) << "\","
           << "\"role\":\"" << u.role << "\","
           << "\"createdAt\":\"" << ResponseUtil::escapeJson(u.createdAt) << "\""
           << "}";
    }
    ss << "]";

    resp.status = 200;
    resp.body = ResponseUtil::success("Users retrieved.", ss.str());
    return resp;
}

HttpResponseData AdminController::handleGetSellerStats(const TokenPayload& user) {
    HttpResponseData resp;
    if (!user.isValid || (user.role != "seller" && user.role != "admin")) {
        resp.status = 403;
        resp.body = ResponseUtil::error("Forbidden: Seller access required.", 403);
        return resp;
    }

    auto& db = DatabaseManager::getInstance();
    auto stats = db.getSellerStats(user.userId);

    std::ostringstream ss;
    ss << "{"
       << "\"totalProducts\":" << stats.totalProducts << ","
       << "\"totalOrders\":" << stats.totalOrders << ","
       << "\"totalSales\":" << stats.totalSales
       << "}";

    resp.status = 200;
    resp.body = ResponseUtil::success("Seller statistics retrieved.", ss.str());
    return resp;
}

} // namespace PricillaMart
