#pragma once
#include <string>
#include <map>
#include "AuthController.h"
#include "utils/JwtUtil.h"

namespace PricillaMart {

class AdminController {
public:
    static HttpResponseData handleGetAdminStats(const TokenPayload& user);
    static HttpResponseData handleGetUsers(const std::map<std::string, std::string>& params, const TokenPayload& user);
    static HttpResponseData handleGetSellerStats(const TokenPayload& user);
};

} // namespace PricillaMart
