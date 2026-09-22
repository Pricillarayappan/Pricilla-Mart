#pragma once
#include <string>
#include "AuthController.h"
#include "utils/JwtUtil.h"

namespace PricillaMart {

class ChatbotController {
public:
    static HttpResponseData handleChat(const std::string& requestBody, const TokenPayload& user, const std::string& apiKey);
};

} // namespace PricillaMart
