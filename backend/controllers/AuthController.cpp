#include "AuthController.h"
#include "database/DatabaseManager.h"
#include "utils/PasswordHasher.h"
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
        // Check for non-quoted numeric/boolean
        std::string numPattern = "\"" + key + "\"\\s*:\\s*([0-9a-zA-Z._-]+)";
        std::regex rNum(numPattern);
        if (std::regex_search(json, m, rNum) && m.size() > 1) {
            return m[1].str();
        }
        return "";
    }

    bool isValidEmail(const std::string& email) {
        const std::regex pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
        return std::regex_match(email, pattern);
    }
}

HttpResponseData AuthController::handleRegister(const std::string& requestBody, const std::string& jwtSecret) {
    HttpResponseData resp;

    std::string name = extractJsonValue(requestBody, "name");
    std::string email = extractJsonValue(requestBody, "email");
    std::string phone = extractJsonValue(requestBody, "phone");
    std::string password = extractJsonValue(requestBody, "password");
    std::string confirmPassword = extractJsonValue(requestBody, "confirmPassword");
    std::string role = extractJsonValue(requestBody, "role");

    if (name.empty() || email.empty() || phone.empty() || password.empty() || confirmPassword.empty() || role.empty()) {
        resp.status = 400;
        resp.body = ResponseUtil::error("All registration fields are mandatory.", 400);
        return resp;
    }

    if (!isValidEmail(email)) {
        resp.status = 400;
        resp.body = ResponseUtil::error("Invalid email address format.", 400);
        return resp;
    }

    if (phone.length() < 7) {
        resp.status = 400;
        resp.body = ResponseUtil::error("Invalid phone number format.", 400);
        return resp;
    }

    if (password.length() < 6) {
        resp.status = 400;
        resp.body = ResponseUtil::error("Password must be at least 6 characters long.", 400);
        return resp;
    }

    if (password != confirmPassword) {
        resp.status = 400;
        resp.body = ResponseUtil::error("Passwords do not match.", 400);
        return resp;
    }

    if (role != "buyer" && role != "seller") {
        resp.status = 400;
        resp.body = ResponseUtil::error("Invalid role. Role must be 'buyer' or 'seller'.", 400);
        return resp;
    }

    auto& db = DatabaseManager::getInstance();
    if (db.findUserByEmail(email).has_value()) {
        resp.status = 409;
        resp.body = ResponseUtil::error("An account with this email address already exists.", 409);
        return resp;
    }

    User user;
    user.name = name;
    user.email = email;
    user.phone = phone;
    user.passwordHash = PasswordHasher::sha256(password);
    user.role = role;

    int newId = 0;
    if (!db.createUser(user, newId)) {
        resp.status = 500;
        resp.body = ResponseUtil::error("Failed to create user account in database.", 500);
        return resp;
    }

    std::string token = JwtUtil::createToken(newId, email, role, jwtSecret);

    std::ostringstream data;
    data << "{\"token\":\"" << token << "\","
         << "\"user\":{"
         << "\"id\":" << newId << ","
         << "\"name\":\"" << ResponseUtil::escapeJson(name) << "\","
         << "\"email\":\"" << ResponseUtil::escapeJson(email) << "\","
         << "\"phone\":\"" << ResponseUtil::escapeJson(phone) << "\","
         << "\"role\":\"" << role << "\""
         << "}}";

    resp.status = 201;
    resp.body = ResponseUtil::success("Account registered successfully.", data.str());
    return resp;
}

HttpResponseData AuthController::handleLogin(const std::string& requestBody, const std::string& jwtSecret) {
    HttpResponseData resp;

    std::string email = extractJsonValue(requestBody, "email");
    std::string password = extractJsonValue(requestBody, "password");

    if (email.empty() || password.empty()) {
        resp.status = 400;
        resp.body = ResponseUtil::error("Email and password are required.", 400);
        return resp;
    }

    auto& db = DatabaseManager::getInstance();
    auto userOpt = db.findUserByEmail(email);
    if (!userOpt.has_value()) {
        resp.status = 401;
        resp.body = ResponseUtil::error("Invalid email or password.", 401);
        return resp;
    }

    const auto& user = userOpt.value();
    if (!PasswordHasher::verify(password, user.passwordHash)) {
        resp.status = 401;
        resp.body = ResponseUtil::error("Invalid email or password.", 401);
        return resp;
    }

    std::string token = JwtUtil::createToken(user.id, user.email, user.role, jwtSecret);

    std::ostringstream data;
    data << "{\"token\":\"" << token << "\","
         << "\"user\":{"
         << "\"id\":" << user.id << ","
         << "\"name\":\"" << ResponseUtil::escapeJson(user.name) << "\","
         << "\"email\":\"" << ResponseUtil::escapeJson(user.email) << "\","
         << "\"phone\":\"" << ResponseUtil::escapeJson(user.phone) << "\","
         << "\"role\":\"" << user.role << "\""
         << "}}";

    resp.status = 200;
    resp.body = ResponseUtil::success("Login successful.", data.str());
    return resp;
}

HttpResponseData AuthController::handleLogout() {
    HttpResponseData resp;
    resp.status = 200;
    resp.body = ResponseUtil::success("Logged out successfully.");
    return resp;
}

HttpResponseData AuthController::handleGetProfile(const TokenPayload& user) {
    HttpResponseData resp;
    if (!user.isValid) {
        resp.status = 401;
        resp.body = ResponseUtil::error("Unauthorized. Please log in.", 401);
        return resp;
    }

    auto& db = DatabaseManager::getInstance();
    auto userOpt = db.findUserById(user.userId);
    if (!userOpt.has_value()) {
        resp.status = 404;
        resp.body = ResponseUtil::error("User account not found.", 404);
        return resp;
    }

    const auto& u = userOpt.value();
    std::ostringstream data;
    data << "{"
         << "\"id\":" << u.id << ","
         << "\"name\":\"" << ResponseUtil::escapeJson(u.name) << "\","
         << "\"email\":\"" << ResponseUtil::escapeJson(u.email) << "\","
         << "\"phone\":\"" << ResponseUtil::escapeJson(u.phone) << "\","
         << "\"role\":\"" << u.role << "\","
         << "\"createdAt\":\"" << ResponseUtil::escapeJson(u.createdAt) << "\""
         << "}";

    resp.status = 200;
    resp.body = ResponseUtil::success("User profile fetched.", data.str());
    return resp;
}

} // namespace PricillaMart
