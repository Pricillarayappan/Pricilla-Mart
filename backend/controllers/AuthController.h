#pragma once
#include <string>
#include <map>
#include "utils/JwtUtil.h"

namespace PricillaMart {

struct HttpResponseData {
    int status = 200;
    std::string contentType = "application/json";
    std::string body;
};

class AuthController {
public:
    static HttpResponseData handleRegister(const std::string& requestBody, const std::string& jwtSecret);
    static HttpResponseData handleLogin(const std::string& requestBody, const std::string& jwtSecret);
    static HttpResponseData handleLogout();
    static HttpResponseData handleGetProfile(const TokenPayload& user);
};

} // namespace PricillaMart
