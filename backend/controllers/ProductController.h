#pragma once
#include <string>
#include <map>
#include "AuthController.h"
#include "utils/JwtUtil.h"

namespace PricillaMart {

class ProductController {
public:
    static HttpResponseData handleGetProducts(const std::map<std::string, std::string>& params);
    static HttpResponseData handleGetProductById(int id);
    static HttpResponseData handleCreateProduct(const std::string& body, const TokenPayload& user);
    static HttpResponseData handleUpdateProduct(int id, const std::string& body, const TokenPayload& user);
    static HttpResponseData handleDeleteProduct(int id, const TokenPayload& user);
};

} // namespace PricillaMart
