#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <ctime>
#include "PasswordHasher.h"

namespace PricillaMart {

struct TokenPayload {
    int userId = 0;
    std::string email;
    std::string role;
    int64_t expiresAt = 0;
    bool isValid = false;
};

class JwtUtil {
public:
    static std::string base64UrlEncode(const std::string& input) {
        static const char* b64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
        std::string result;
        int val = 0, valb = -6;
        for (unsigned char c : input) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                result.push_back(b64chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) {
            result.push_back(b64chars[((val << 8) >> (valb + 8)) & 0x3F]);
        }
        return result;
    }

    static std::string base64UrlDecode(const std::string& input) {
        std::vector<int> T(256, -1);
        static const char* b64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
        for (int i = 0; i < 64; i++) T[static_cast<unsigned char>(b64chars[i])] = i;

        std::string result;
        int val = 0, valb = -8;
        for (unsigned char c : input) {
            if (T[c] == -1) continue;
            val = (val << 6) + T[c];
            valb += 6;
            if (valb >= 0) {
                result.push_back(static_cast<char>((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return result;
    }

    static std::string createToken(int userId, const std::string& email, const std::string& role, const std::string& secret, int validSeconds = 86400 * 7) {
        int64_t exp = static_cast<int64_t>(std::time(nullptr)) + validSeconds;
        std::string payload = std::to_string(userId) + ":" + email + ":" + role + ":" + std::to_string(exp);
        std::string encodedPayload = base64UrlEncode(payload);
        std::string signature = PasswordHasher::sha256(encodedPayload + "." + secret);
        return encodedPayload + "." + signature;
    }

    static TokenPayload verifyToken(const std::string& token, const std::string& secret) {
        TokenPayload result;
        size_t dotPos = token.rfind('.');
        if (dotPos == std::string::npos) return result;

        std::string encodedPayload = token.substr(0, dotPos);
        std::string receivedSig = token.substr(dotPos + 1);

        std::string expectedSig = PasswordHasher::sha256(encodedPayload + "." + secret);
        if (receivedSig != expectedSig) return result;

        std::string decoded = base64UrlDecode(encodedPayload);
        std::stringstream ss(decoded);
        std::string uIdStr, email, role, expStr;
        if (!std::getline(ss, uIdStr, ':')) return result;
        if (!std::getline(ss, email, ':')) return result;
        if (!std::getline(ss, role, ':')) return result;
        if (!std::getline(ss, expStr)) return result;

        try {
            result.userId = std::stoi(uIdStr);
            result.expiresAt = std::stoll(expStr);
        } catch (...) {
            return result;
        }

        if (result.expiresAt < static_cast<int64_t>(std::time(nullptr))) {
            return result; // Expired
        }

        result.email = email;
        result.role = role;
        result.isValid = true;
        return result;
    }
};

} // namespace PricillaMart
