#pragma once
#include <string>
#include "AuthController.h"
#include "utils/JwtUtil.h"

namespace PricillaMart {

class ReviewController {
public:
    static HttpResponseData handleAddReview(const std::string& body, const TokenPayload& user);
    static HttpResponseData handleGetProductReviews(int productId);
};

} // namespace PricillaMart
