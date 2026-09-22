#pragma once
#include <string>

namespace PricillaMart {

struct User {
    int id = 0;
    std::string name;
    std::string email;
    std::string phone;
    std::string passwordHash;
    std::string role; // "buyer", "seller", "admin"
    std::string createdAt;
};

} // namespace PricillaMart
