#pragma once
#include <string>

namespace PricillaMart {

class PasswordHasher {
public:
    static std::string sha256(const std::string& input);
    static bool verify(const std::string& password, const std::string& storedHash);
};

} // namespace PricillaMart
