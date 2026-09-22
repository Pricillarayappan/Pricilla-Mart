#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>
#include <algorithm>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef SOCKET socket_t;
#define CLOSE_SOCKET(s) closesocket(s)
#define IS_VALID_SOCKET(s) ((s) != INVALID_SOCKET)
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int socket_t;
#define CLOSE_SOCKET(s) close(s)
#define IS_VALID_SOCKET(s) ((s) >= 0)
#define INVALID_SOCKET -1
#endif

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
#include "utils/ResponseUtil.h"

using namespace PricillaMart;

// Simple URL decoding
std::string urlDecode(const std::string& in) {
    std::string out;
    for (size_t i = 0; i < in.length(); ++i) {
        if (in[i] == '%') {
            if (i + 2 < in.length()) {
                int hex = 0;
                std::istringstream iss(in.substr(i + 1, 2));
                if (iss >> std::hex >> hex) {
                    out += static_cast<char>(hex);
                    i += 2;
                } else {
                    out += in[i];
                }
            }
        } else if (in[i] == '+') {
            out += ' ';
        } else {
            out += in[i];
        }
    }
    return out;
}

std::string getMimeType(const std::string& path) {
    if (path.ends_with(".html") || path.ends_with(".htm")) return "text/html; charset=utf-8";
    if (path.ends_with(".css")) return "text/css; charset=utf-8";
    if (path.ends_with(".js")) return "application/javascript; charset=utf-8";
    if (path.ends_with(".json")) return "application/json; charset=utf-8";
    if (path.ends_with(".png")) return "image/png";
    if (path.ends_with(".jpg") || path.ends_with(".jpeg")) return "image/jpeg";
    if (path.ends_with(".gif")) return "image/gif";
    if (path.ends_with(".svg")) return "image/svg+xml";
    if (path.ends_with(".ico")) return "image/x-icon";
    return "application/octet-stream";
}

void handleClient(socket_t clientSocket, const AppConfig& config) {
    char buffer[16384];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        CLOSE_SOCKET(clientSocket);
        return;
    }
    buffer[bytesReceived] = '\0';
    std::string requestStr(buffer, bytesReceived);

    std::istringstream reqStream(requestStr);
    std::string method, fullPath, httpVersion;
    reqStream >> method >> fullPath >> httpVersion;

    std::map<std::string, std::string> headers;
    std::string line;
    std::getline(reqStream, line); // finish request line
    while (std::getline(reqStream, line) && line != "\r" && !line.empty()) {
        if (line.back() == '\r') line.pop_back();
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string val = line.substr(colon + 1);
            while (!val.empty() && (val.front() == ' ' || val.front() == '\t')) val.erase(val.begin());
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            headers[key] = val;
        }
    }

    // Read remaining body if Content-Length specified
    std::string body;
    size_t headerEnd = requestStr.find("\r\n\r\n");
    if (headerEnd != std::string::npos) {
        body = requestStr.substr(headerEnd + 4);
    }
    auto itCl = headers.find("content-length");
    if (itCl != headers.end()) {
        try {
            size_t expectedLen = std::stoul(itCl->second);
            while (body.length() < expectedLen) {
                int moreBytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
                if (moreBytes <= 0) break;
                body.append(buffer, moreBytes);
            }
        } catch (...) {}
    }

    // Split path and query parameters
    std::string path = fullPath;
    std::map<std::string, std::string> queryParams;
    size_t qPos = fullPath.find('?');
    if (qPos != std::string::npos) {
        path = fullPath.substr(0, qPos);
        std::string qStr = fullPath.substr(qPos + 1);
        std::stringstream ss(qStr);
        std::string pair;
        while (std::getline(ss, pair, '&')) {
            size_t eq = pair.find('=');
            if (eq != std::string::npos) {
                queryParams[urlDecode(pair.substr(0, eq))] = urlDecode(pair.substr(eq + 1));
            } else if (!pair.empty()) {
                queryParams[urlDecode(pair)] = "";
            }
        }
    }

    // Extract Bearer Token
    TokenPayload user;
    auto itAuth = headers.find("authorization");
    if (itAuth != headers.end()) {
        std::string authVal = itAuth->second;
        if (authVal.rfind("Bearer ", 0) == 0) {
            user = JwtUtil::verifyToken(authVal.substr(7), config.jwtSecret);
        }
    }

    HttpResponseData response;

    // Handle CORS preflight
    if (method == "OPTIONS") {
        std::string corsHeader = "HTTP/1.1 200 OK\r\n"
                                 "Access-Control-Allow-Origin: *\r\n"
                                 "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
                                 "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
                                 "Content-Length: 0\r\n\r\n";
        send(clientSocket, corsHeader.c_str(), static_cast<int>(corsHeader.length()), 0);
        CLOSE_SOCKET(clientSocket);
        return;
    }

    // ---------------- ROUTING ENGINE ----------------
    if (path == "/api/register" && method == "POST") {
        response = AuthController::handleRegister(body, config.jwtSecret);
    } else if (path == "/api/login" && method == "POST") {
        response = AuthController::handleLogin(body, config.jwtSecret);
    } else if (path == "/api/logout" && method == "POST") {
        response = AuthController::handleLogout();
    } else if (path == "/api/profile" && method == "GET") {
        response = AuthController::handleGetProfile(user);
    } else if (path == "/api/categories" && method == "GET") {
        response = CategoryController::handleGetCategories();
    } else if (path == "/api/products" && method == "GET") {
        response = ProductController::handleGetProducts(queryParams);
    } else if (path == "/api/products" && method == "POST") {
        response = ProductController::handleCreateProduct(body, user);
    } else if (path.rfind("/api/products/", 0) == 0) {
        std::string rest = path.substr(14);
        size_t slash = rest.find('/');
        if (slash == std::string::npos) {
            int prodId = 0;
            try { prodId = std::stoi(rest); } catch (...) {}
            if (method == "GET") response = ProductController::handleGetProductById(prodId);
            else if (method == "PUT") response = ProductController::handleUpdateProduct(prodId, body, user);
            else if (method == "DELETE") response = ProductController::handleDeleteProduct(prodId, user);
            else response = {405, "application/json", ResponseUtil::error("Method Not Allowed", 405)};
        } else {
            std::string idStr = rest.substr(0, slash);
            std::string sub = rest.substr(slash + 1);
            int prodId = 0;
            try { prodId = std::stoi(idStr); } catch (...) {}
            if (sub == "reviews" && method == "GET") {
                response = ReviewController::handleGetProductReviews(prodId);
            } else {
                response = {404, "application/json", ResponseUtil::error("Not Found", 404)};
            }
        }
    } else if (path == "/api/cart" && method == "GET") {
        response = CartController::handleGetCart(user);
    } else if (path == "/api/cart" && method == "POST") {
        response = CartController::handleAddToCart(body, user);
    } else if (path == "/api/cart" && method == "DELETE") {
        response = CartController::handleClearCart(user);
    } else if (path.rfind("/api/cart/", 0) == 0) {
        int cartItemId = 0;
        try { cartItemId = std::stoi(path.substr(10)); } catch (...) {}
        if (method == "PUT") response = CartController::handleUpdateQuantity(cartItemId, body, user);
        else if (method == "DELETE") response = CartController::handleRemoveItem(cartItemId, user);
        else response = {405, "application/json", ResponseUtil::error("Method Not Allowed", 405)};
    } else if (path == "/api/orders" && method == "GET") {
        response = OrderController::handleGetOrders(user);
    } else if (path == "/api/orders" && method == "POST") {
        response = OrderController::handleCreateOrder(body, user);
    } else if (path.rfind("/api/orders/", 0) == 0) {
        std::string rest = path.substr(12);
        size_t slash = rest.find('/');
        if (slash == std::string::npos) {
            int orderId = 0;
            try { orderId = std::stoi(rest); } catch (...) {}
            if (method == "GET") response = OrderController::handleGetOrderById(orderId, user);
            else response = {405, "application/json", ResponseUtil::error("Method Not Allowed", 405)};
        } else {
            std::string idStr = rest.substr(0, slash);
            std::string sub = rest.substr(slash + 1);
            int orderId = 0;
            try { orderId = std::stoi(idStr); } catch (...) {}
            if (sub == "status" && method == "PUT") {
                response = OrderController::handleUpdateStatus(orderId, body, user);
            } else {
                response = {404, "application/json", ResponseUtil::error("Not Found", 404)};
            }
        }
    } else if (path == "/api/reviews" && method == "POST") {
        response = ReviewController::handleAddReview(body, user);
    } else if (path == "/api/admin/stats" && method == "GET") {
        response = AdminController::handleGetAdminStats(user);
    } else if (path == "/api/admin/users" && method == "GET") {
        response = AdminController::handleGetUsers(queryParams, user);
    } else if (path == "/api/seller/stats" && method == "GET") {
        response = AdminController::handleGetSellerStats(user);
    } else if (path == "/api/chatbot" && method == "POST") {
        response = ChatbotController::handleChat(body, user, config.chatbotApiKey);
    } else if (path.rfind("/api/", 0) == 0) {
        response = {404, "application/json", ResponseUtil::error("API endpoint not found", 404)};
    } else {
        // Static File Serving
        std::string filePath = config.staticDir + (path == "/" ? "/index.html" : path);
        std::ifstream file(filePath, std::ios::binary);
        if (file.is_open()) {
            std::ostringstream ss;
            ss << file.rdbuf();
            std::string content = ss.str();
            std::string mime = getMimeType(filePath);

            std::ostringstream out;
            out << "HTTP/1.1 200 OK\r\n"
                << "Content-Type: " << mime << "\r\n"
                << "Content-Length: " << content.length() << "\r\n"
                << "Access-Control-Allow-Origin: *\r\n"
                << "Connection: close\r\n\r\n"
                << content;
            std::string fullResp = out.str();
            send(clientSocket, fullResp.c_str(), static_cast<int>(fullResp.length()), 0);
            CLOSE_SOCKET(clientSocket);
            return;
        } else {
            // Check for 404.html fallback
            std::string notFoundPath = config.staticDir + "/404.html";
            std::ifstream nfFile(notFoundPath, std::ios::binary);
            if (nfFile.is_open()) {
                std::ostringstream ss;
                ss << nfFile.rdbuf();
                std::string content = ss.str();
                std::ostringstream out;
                out << "HTTP/1.1 404 Not Found\r\n"
                    << "Content-Type: text/html; charset=utf-8\r\n"
                    << "Content-Length: " << content.length() << "\r\n"
                    << "Access-Control-Allow-Origin: *\r\n"
                    << "Connection: close\r\n\r\n"
                    << content;
                std::string fullResp = out.str();
                send(clientSocket, fullResp.c_str(), static_cast<int>(fullResp.length()), 0);
                CLOSE_SOCKET(clientSocket);
                return;
            }
            response = {404, "text/plain", "404 Not Found"};
        }
    }

    // Format HTTP Response
    std::string statusMsg = (response.status == 200) ? "OK" :
                           (response.status == 201) ? "Created" :
                           (response.status == 400) ? "Bad Request" :
                           (response.status == 401) ? "Unauthorized" :
                           (response.status == 403) ? "Forbidden" :
                           (response.status == 404) ? "Not Found" :
                           (response.status == 409) ? "Conflict" : "Internal Server Error";

    std::ostringstream respStream;
    respStream << "HTTP/1.1 " << response.status << " " << statusMsg << "\r\n"
               << "Content-Type: " << response.contentType << "\r\n"
               << "Content-Length: " << response.body.length() << "\r\n"
               << "Access-Control-Allow-Origin: *\r\n"
               << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
               << "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
               << "Connection: close\r\n\r\n"
               << response.body;

    std::string finalOutput = respStream.str();
    send(clientSocket, finalOutput.c_str(), static_cast<int>(finalOutput.length()), 0);
    CLOSE_SOCKET(clientSocket);
}

int main(int argc, char* argv[]) {
    auto config = AppConfig::load();

    std::cout << "============================================================" << std::endl;
    std::cout << "                     PRICILLA MART                          " << std::endl;
    std::cout << "         FULL-STACK C++20 E-COMMERCE PLATFORM               " << std::endl;
    std::cout << "============================================================" << std::endl;
    std::cout << "[System] Initializing SQLite3 database..." << std::endl;

    if (!DatabaseManager::getInstance().initialize(config.dbPath, config.schemaPath, config.seedPath)) {
        std::cerr << "[Error] Database initialization failed." << std::endl;
        return 1;
    }

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[Error] WSAStartup failed." << std::endl;
        return 1;
    }
#endif

    socket_t serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (!IS_VALID_SOCKET(serverSocket)) {
        std::cerr << "[Error] Failed to create socket." << std::endl;
        return 1;
    }

    int opt = 1;
#ifdef _WIN32
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(static_cast<uint16_t>(config.port));

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "[Error] Bind failed on port " << config.port << std::endl;
        CLOSE_SOCKET(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 50) < 0) {
        std::cerr << "[Error] Listen failed on port " << config.port << std::endl;
        CLOSE_SOCKET(serverSocket);
        return 1;
    }

    std::cout << "[Server] Pricilla Mart is LIVE and listening on http://localhost:" << config.port << std::endl;
    std::cout << "[Server] Serving frontend from: " << config.staticDir << std::endl;
    std::cout << "------------------------------------------------------------" << std::endl;
    std::cout << "DEFAULT DEMO ACCOUNTS:" << std::endl;
    std::cout << "  Admin:  admin@pricillamart.com    | Password: Admin@123" << std::endl;
    std::cout << "  Seller: techworld@pricillamart.com| Password: Seller@123" << std::endl;
    std::cout << "  Buyer:  rahul@gmail.com           | Password: Buyer@123" << std::endl;
    std::cout << "============================================================" << std::endl;

    while (true) {
        sockaddr_in clientAddr{};
#ifdef _WIN32
        int clientLen = sizeof(clientAddr);
#else
        socklen_t clientLen = sizeof(clientAddr);
#endif
        socket_t clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        if (IS_VALID_SOCKET(clientSocket)) {
            std::thread([clientSocket, config]() {
                handleClient(clientSocket, config);
            }).detach();
        }
    }

    CLOSE_SOCKET(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
