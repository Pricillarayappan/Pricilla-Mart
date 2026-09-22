#pragma once
#include <string>
#include <vector>

namespace PricillaMart {

struct OrderItem {
    int id = 0;
    int orderId = 0;
    int productId = 0;
    std::string productName;
    std::string imageUrl;
    int sellerId = 0;
    std::string sellerName;
    int quantity = 0;
    double unitPrice = 0.0;
    double subtotal = 0.0;
};

struct Order {
    int id = 0;
    std::string orderCode;
    int buyerId = 0;
    std::string buyerName;
    std::string email;
    std::string phone;
    std::string deliveryAddress;
    double totalAmount = 0.0;
    std::string status; // "Pending", "Confirmed", "Shipped", "Delivered", "Cancelled"
    std::string createdAt;
    std::vector<OrderItem> items;
};

} // namespace PricillaMart
