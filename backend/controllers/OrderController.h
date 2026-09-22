#pragma once
#include <string>
#include "AuthController.h"
#include "utils/JwtUtil.h"

namespace PricillaMart {

class OrderController {
public:
    static HttpResponseData handleCreateOrder(const std::string& body, const TokenPayload& user);
    static HttpResponseData handleGetOrders(const TokenPayload& user);
    static HttpResponseData handleGetOrderById(int id, const TokenPayload& user);
    static HttpResponseData handleUpdateStatus(int id, const std::string& body, const TokenPayload& user);
};

} // namespace PricillaMart
