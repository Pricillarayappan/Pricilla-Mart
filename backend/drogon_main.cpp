#include <iostream>
#include "config/AppConfig.h"
#include "database/DatabaseManager.h"
#include "controllers/AuthController.h"
#include "controllers/ProductController.h"
#include "controllers/CategoryController.h"
#include "controllers/CartController.h"
#include "controllers/OrderController.h"
#include "controllers/ReviewController.h"
#include "controllers/AdminController.h"
#include "controllers/ChatbotController.h"
#include "utils/JwtUtil.h"

#ifdef DROGON_FOUND
#include <drogon/drogon.h>

using namespace drogon;
using namespace PricillaMart;

TokenPayload getDrogonAuth(const HttpRequestPtr& req, const std::string& secret) {
    std::string authHeader = req->getHeader("Authorization");
    if (authHeader.rfind("Bearer ", 0) == 0) {
        return JwtUtil::verifyToken(authHeader.substr(7), secret);
    }
    return TokenPayload{};
}

void makeDrogonResponse(const HttpResponseData& data, const std::function<void(const HttpResponsePtr&)>& callback) {
    auto resp = HttpResponse::newHttpResponse();
    resp->setStatusCode(static_cast<HttpStatusCode>(data.status));
    resp->setContentTypeCode(CT_APPLICATION_JSON);
    resp->setBody(data.body);
    resp->addHeader("Access-Control-Allow-Origin", "*");
    resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
    callback(resp);
}

int main(int argc, char* argv[]) {
    auto config = AppConfig::load();
    std::cout << "============================================================" << std::endl;
    std::cout << "          PRICILLA MART - DROGON C++ BACKEND                " << std::endl;
    std::cout << "============================================================" << std::endl;

    if (!DatabaseManager::getInstance().initialize(config.dbPath, config.schemaPath, config.seedPath)) {
        std::cerr << "Failed to initialize database. Exiting." << std::endl;
        return 1;
    }

    app().loadConfigFile("backend/config/config.json");
    app().setDocumentRoot(config.staticDir);
    app().setHomePage("index.html");

    // AUTH ROUTES
    app().registerHandler("/api/register", [config](const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
        auto data = AuthController::handleRegister(std::string(req->body()), config.jwtSecret);
        makeDrogonResponse(data, callback);
    }, {Post});

    app().registerHandler("/api/login", [config](const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
        auto data = AuthController::handleLogin(std::string(req->body()), config.jwtSecret);
        makeDrogonResponse(data, callback);
    }, {Post});

    app().registerHandler("/api/logout", [](const HttpRequestPtr& /*req*/, std::function<void(const HttpResponsePtr&)>&& callback) {
        auto data = AuthController::handleLogout();
        makeDrogonResponse(data, callback);
    }, {Post});

    app().registerHandler("/api/profile", [config](const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
        auto user = getDrogonAuth(req, config.jwtSecret);
        auto data = AuthController::handleGetProfile(user);
        makeDrogonResponse(data, callback);
    }, {Get});

    // CATEGORIES
    app().registerHandler("/api/categories", [](const HttpRequestPtr& /*req*/, std::function<void(const HttpResponsePtr&)>&& callback) {
        auto data = CategoryController::handleGetCategories();
        makeDrogonResponse(data, callback);
    }, {Get});

    // PRODUCTS
    app().registerHandler("/api/products", [](const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
        std::map<std::string, std::string> params;
        for (const auto& [k, v] : req->getParameters()) {
            params[k] = v;
        }
        auto data = ProductController::handleGetProducts(params);
        makeDrogonResponse(data, callback);
    }, {Get});

    app().registerHandler("/api/products", [config](const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
        auto user = getDrogonAuth(req, config.jwtSecret);
        auto data = ProductController::handleCreateProduct(std::string(req->body()), user);
        makeDrogonResponse(data, callback);
    }, {Post});

    // CART
    app().registerHandler("/api/cart", [config](const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
        auto user = getDrogonAuth(req, config.jwtSecret);
        auto data = CartController::handleGetCart(user);
        makeDrogonResponse(data, callback);
    }, {Get});

    app().registerHandler("/api/cart", [config](const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
        auto user = getDrogonAuth(req, config.jwtSecret);
        auto data = CartController::handleAddToCart(std::string(req->body()), user);
        makeDrogonResponse(data, callback);
    }, {Post});

    // ORDERS
    app().registerHandler("/api/orders", [config](const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
        auto user = getDrogonAuth(req, config.jwtSecret);
        auto data = OrderController::handleCreateOrder(std::string(req->body()), user);
        makeDrogonResponse(data, callback);
    }, {Post});

    app().registerHandler("/api/orders", [config](const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
        auto user = getDrogonAuth(req, config.jwtSecret);
        auto data = OrderController::handleGetOrders(user);
        makeDrogonResponse(data, callback);
    }, {Get});

    // REVIEWS
    app().registerHandler("/api/reviews", [config](const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
        auto user = getDrogonAuth(req, config.jwtSecret);
        auto data = ReviewController::handleAddReview(std::string(req->body()), user);
        makeDrogonResponse(data, callback);
    }, {Post});

    // CHATBOT
    app().registerHandler("/api/chatbot", [config](const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
        auto user = getDrogonAuth(req, config.jwtSecret);
        auto data = ChatbotController::handleChat(std::string(req->body()), user, config.chatbotApiKey);
        makeDrogonResponse(data, callback);
    }, {Post});

    // ADMIN
    app().registerHandler("/api/admin/stats", [config](const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
        auto user = getDrogonAuth(req, config.jwtSecret);
        auto data = AdminController::handleGetAdminStats(user);
        makeDrogonResponse(data, callback);
    }, {Get});

    std::cout << "[Drogon] Server starting on http://" << config.host << ":" << config.port << std::endl;
    app().run();
    return 0;
}
#else
int main() {
    std::cout << "Drogon framework build target placeholder." << std::endl;
    return 0;
}
#endif
