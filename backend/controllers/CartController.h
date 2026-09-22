#pragma once
#include <string>
#include "AuthController.h"
#include "utils/JwtUtil.h"

namespace PricillaMart {

class CartController {
public:
    static HttpResponseData handleGetCart(const TokenPayload& user);
    static HttpResponseData handleAddToCart(const std::string& body, const TokenPayload& user);
    static HttpResponseData handleUpdateQuantity(int cartItemId, const std::string& body, const TokenPayload& user);
    static HttpResponseData handleRemoveItem(int cartItemId, const TokenPayload& user);
    static HttpResponseData handleClearCart(const TokenPayload& user);
};

} // namespace PricillaMart
