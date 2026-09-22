#pragma once
#include <string>
#include <sstream>
#include <vector>

namespace PricillaMart {

class ResponseUtil {
public:
    static std::string escapeJson(const std::string& s) {
        std::ostringstream o;
        for (char c : s) {
            switch (c) {
                case '"': o << "\\\""; break;
                case '\\': o << "\\\\"; break;
                case '\b': o << "\\b"; break;
                case '\f': o << "\\f"; break;
                case '\n': o << "\\n"; break;
                case '\r': o << "\\r"; break;
                case '\t': o << "\\t"; break;
                default:
                    if (static_cast<unsigned char>(c) <= 0x1f) {
                        o << "\\u" << std::hex;
                        o.width(4);
                        o.fill('0');
                        o << static_cast<int>(static_cast<unsigned char>(c));
                    } else {
                        o << c;
                    }
            }
        }
        return o.str();
    }

    static std::string error(const std::string& message, int code = 400) {
        std::ostringstream ss;
        ss << "{\"success\":false,\"error\":\"" << escapeJson(message) << "\",\"code\":" << code << "}";
        return ss.str();
    }

    static std::string success(const std::string& message = "Operation successful", const std::string& dataJson = "{}") {
        std::ostringstream ss;
        ss << "{\"success\":true,\"message\":\"" << escapeJson(message) << "\",\"data\":" << dataJson << "}";
        return ss.str();
    }
};

} // namespace PricillaMart
