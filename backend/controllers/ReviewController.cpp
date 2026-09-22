#include "ReviewController.h"
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
}

HttpResponseData ReviewController::handleAddReview(const std::string& body, const TokenPayload& user) {
    HttpResponseData resp;
    if (!user.isValid) {
        resp.status = 401;
        resp.body = ResponseUtil::error("Unauthorized. Please log in to write a review.", 401);
        return resp;
    }

    std::string pIdStr = extractJsonValue(body, "productId");
    std::string rStr = extractJsonValue(body, "rating");
    std::string comment = extractJsonValue(body, "comment");

    if (pIdStr.empty() || rStr.empty() || comment.empty()) {
        resp.status = 400;
        resp.body = ResponseUtil::error("Product ID, rating (1-5), and written comment are required.", 400);
        return resp;
    }

    int productId = 0;
    int rating = 5;
    try {
        productId = std::stoi(pIdStr);
        rating = std::stoi(rStr);
    } catch (...) {
        resp.status = 400;
        resp.body = ResponseUtil::error("Invalid numeric value for product ID or rating.", 400);
        return resp;
    }

    if (rating < 1 || rating > 5) {
        resp.status = 400;
        resp.body = ResponseUtil::error("Rating must be between 1 and 5 stars.", 400);
        return resp;
    }

    auto& db = DatabaseManager::getInstance();
    auto prodOpt = db.getProductById(productId);
    if (!prodOpt.has_value()) {
        resp.status = 404;
        resp.body = ResponseUtil::error("Product not found.", 404);
        return resp;
    }

    Review r;
    r.productId = productId;
    r.buyerId = user.userId;
    r.rating = rating;
    r.comment = comment;

    std::string errMsg;
    if (!db.addReview(r, errMsg)) {
        resp.status = 400;
        resp.body = ResponseUtil::error(errMsg.empty() ? "Failed to save review." : errMsg, 400);
        return resp;
    }

    resp.status = 201;
    resp.body = ResponseUtil::success("Thank you! Your product review has been submitted successfully.");
    return resp;
}

HttpResponseData ReviewController::handleGetProductReviews(int productId) {
    HttpResponseData resp;
    auto& db = DatabaseManager::getInstance();
    double avgRating = 0.0;
    int count = 0;
    auto reviews = db.getReviewsByProduct(productId, avgRating, count);

    std::ostringstream ss;
    ss << "{"
       << "\"averageRating\":" << avgRating << ","
       << "\"reviewCount\":" << count << ","
       << "\"reviews\":[";
    for (size_t i = 0; i < reviews.size(); ++i) {
        const auto& r = reviews[i];
        if (i > 0) ss << ",";
        ss << "{"
           << "\"id\":" << r.id << ","
           << "\"productId\":" << r.productId << ","
           << "\"buyerId\":" << r.buyerId << ","
           << "\"buyerName\":\"" << ResponseUtil::escapeJson(r.buyerName) << "\","
           << "\"rating\":" << r.rating << ","
           << "\"comment\":\"" << ResponseUtil::escapeJson(r.comment) << "\","
           << "\"createdAt\":\"" << ResponseUtil::escapeJson(r.createdAt) << "\""
           << "}";
    }
    ss << "]}";

    resp.status = 200;
    resp.body = ResponseUtil::success("Reviews fetched successfully.", ss.str());
    return resp;
}

} // namespace PricillaMart
