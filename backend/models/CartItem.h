#pragma once
#include <string>

namespace PricillaMart {

struct CartItem {
    int id = 0;
    int userId = 0;
    int productId = 0;
    std::string productName;
    double price = 0.0;
    std::string imageUrl;
    int availableStock = 0;
    int quantity = 0;
    double subtotal = 0.0;
    std::string createdAt;
};

} // namespace PricillaMart
