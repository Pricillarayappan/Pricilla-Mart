#pragma once
#include <string>

namespace PricillaMart {

struct Review {
    int id = 0;
    int productId = 0;
    std::string productName;
    int buyerId = 0;
    std::string buyerName;
    int rating = 5;
    std::string comment;
    std::string createdAt;
};

} // namespace PricillaMart
