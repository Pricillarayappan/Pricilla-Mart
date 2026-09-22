#include "CategoryController.h"
#include "database/DatabaseManager.h"
#include "utils/ResponseUtil.h"
#include <sstream>

namespace PricillaMart {

HttpResponseData CategoryController::handleGetCategories() {
    HttpResponseData resp;
    auto& db = DatabaseManager::getInstance();
    auto categories = db.getAllCategories();

    std::ostringstream ss;
    ss << "[";
    for (size_t i = 0; i < categories.size(); ++i) {
        const auto& c = categories[i];
        if (i > 0) ss << ",";
        ss << "{"
           << "\"id\":" << c.id << ","
           << "\"name\":\"" << ResponseUtil::escapeJson(c.name) << "\","
           << "\"slug\":\"" << ResponseUtil::escapeJson(c.slug) << "\","
           << "\"icon\":\"" << ResponseUtil::escapeJson(c.icon) << "\","
           << "\"description\":\"" << ResponseUtil::escapeJson(c.description) << "\","
           << "\"productCount\":" << c.productCount
           << "}";
    }
    ss << "]";

    resp.status = 200;
    resp.body = ResponseUtil::success("Categories fetched successfully.", ss.str());
    return resp;
}

} // namespace PricillaMart
