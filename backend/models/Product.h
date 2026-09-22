#pragma once
#include <string>

namespace PricillaMart {

struct Product {
    int id = 0;
    std::string name;
    std::string description;
    int categoryId = 0;
    std::string categoryName;
    double price = 0.0;
    int stockQuantity = 0;
    std::string imageUrl;
    int sellerId = 0;
    std::string sellerName;
    double averageRating = 0.0;
    int reviewCount = 0;
    std::string createdAt;
};

} // namespace PricillaMart
