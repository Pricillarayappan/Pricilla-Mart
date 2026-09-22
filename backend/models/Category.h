#pragma once
#include <string>

namespace PricillaMart {

struct Category {
    int id = 0;
    std::string name;
    std::string slug;
    std::string icon;
    std::string description;
    int productCount = 0;
};

} // namespace PricillaMart
