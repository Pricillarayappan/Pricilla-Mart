#pragma once
#include <string>
#include "AuthController.h"

namespace PricillaMart {

class CategoryController {
public:
    static HttpResponseData handleGetCategories();
};

} // namespace PricillaMart
