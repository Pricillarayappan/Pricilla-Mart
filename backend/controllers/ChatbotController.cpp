#include "ChatbotController.h"
#include "database/DatabaseManager.h"
#include "utils/ResponseUtil.h"
#include <sstream>
#include <regex>
#include <algorithm>
#include <iostream>

namespace PricillaMart {

namespace {
    std::string extractJsonValue(const std::string& json, const std::string& key) {
        std::string pattern = "\"" + key + "\"\\s*:\\s*\"([^\"]*)\"";
        std::regex r(pattern);
        std::smatch m;
        if (std::regex_search(json, m, r) && m.size() > 1) {
            return m[1].str();
        }
        return "";
    }

    std::string toLower(const std::string& str) {
        std::string out = str;
        std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return std::tolower(c); });
        return out;
    }

    bool contains(const std::string& text, const std::string& keyword) {
        return text.find(keyword) != std::string::npos;
    }
}

HttpResponseData ChatbotController::handleChat(const std::string& requestBody, const TokenPayload& user, const std::string& /*apiKey*/) {
    HttpResponseData resp;
    std::string message = extractJsonValue(requestBody, "message");

    if (message.empty()) {
        resp.status = 400;
        resp.body = ResponseUtil::error("Message cannot be empty.", 400);
        return resp;
    }

    std::string msgLower = toLower(message);
    auto& db = DatabaseManager::getInstance();

    std::string replyText;
    std::vector<Product> recommendedProducts;
    std::vector<std::string> quickReplies;

    // 1. GREETINGS
    if (contains(msgLower, "hi") || contains(msgLower, "hello") || contains(msgLower, "hey") || contains(msgLower, "who are you")) {
        replyText = "Hello! I am your Pricilla Mart AI Assistant. I can help you find products, check categories, understand cart & checkout steps, or answer questions about orders!";
        quickReplies = {"Find a Laptop", "Show Electronics", "How to order?", "Track my order"};
    }
    // 2. LAPTOPS / COMPUTERS
    else if (contains(msgLower, "laptop") || contains(msgLower, "computer") || contains(msgLower, "pc") || contains(msgLower, "macbook")) {
        replyText = "Sure! You can explore laptops in our Electronics category. Here are our top-rated laptops with high performance:";
        recommendedProducts = db.getProducts("Laptop", 1);
        quickReplies = {"Show all Electronics", "Add to Cart", "View Specs"};
    }
    // 3. MOBILES / SMARTPHONES
    else if (contains(msgLower, "mobile") || contains(msgLower, "phone") || contains(msgLower, "smartphone") || contains(msgLower, "5g")) {
        replyText = "We have high-end smartphones with 5G connectivity and superior cameras in our Electronics department:";
        recommendedProducts = db.getProducts("Mobile", 1);
        quickReplies = {"View Mobile", "Compare Price", "Check Headphones"};
    }
    // 4. HEADPHONES / AUDIO
    else if (contains(msgLower, "headphone") || contains(msgLower, "audio") || contains(msgLower, "earphone") || contains(msgLower, "sound") || contains(msgLower, "music")) {
        replyText = "Looking for great sound? Check out our ANC wireless audio gear:";
        recommendedProducts = db.getProducts("Headphones", 1);
        quickReplies = {"View Headphones", "Electronics Category", "Smart Watch"};
    }
    // 5. SMARTWATCH / ACCESSORIES
    else if (contains(msgLower, "watch") || contains(msgLower, "smartwatch") || contains(msgLower, "handbag") || contains(msgLower, "bag") || contains(msgLower, "accessories")) {
        replyText = "Here are popular picks in our Accessories collection:";
        recommendedProducts = db.getProducts("", 3);
        quickReplies = {"Smart Watch", "Vintage Handbag", "All Accessories"};
    }
    // 6. FASHION & CLOTHING
    else if (contains(msgLower, "fashion") || contains(msgLower, "cloth") || contains(msgLower, "jacket") || contains(msgLower, "dress") || contains(msgLower, "shirt")) {
        replyText = "Explore trendy apparel in our Fashion section:";
        recommendedProducts = db.getProducts("", 2);
        quickReplies = {"Denim Jacket", "Explore Fashion", "Accessories"};
    }
    // 7. HOME & DECOR
    else if (contains(msgLower, "home") || contains(msgLower, "lamp") || contains(msgLower, "decor") || contains(msgLower, "living")) {
        replyText = "Brighten up your living space with our curated Home & Living catalog:";
        recommendedProducts = db.getProducts("", 4);
        quickReplies = {"Desk Lamp", "All Home Products", "Best Sellers"};
    }
    // 8. BEAUTY & SKINCARE
    else if (contains(msgLower, "beauty") || contains(msgLower, "serum") || contains(msgLower, "skin") || contains(msgLower, "cosmetic")) {
        replyText = "Revitalize your skincare routine with top-rated beauty essentials:";
        recommendedProducts = db.getProducts("", 5);
        quickReplies = {"Vitamin C Serum", "Beauty Category", "Popular items"};
    }
    // 9. BOOKS & EDUCATION
    else if (contains(msgLower, "book") || contains(msgLower, "read") || contains(msgLower, "study") || contains(msgLower, "c++") || contains(msgLower, "dsa")) {
        replyText = "Check out our academic and technical library for competitive coding and engineering:";
        recommendedProducts = db.getProducts("", 6);
        quickReplies = {"C++ DSA Book", "All Books", "Electronics"};
    }
    // 10. CATEGORIES GENERAL
    else if (contains(msgLower, "category") || contains(msgLower, "categories") || contains(msgLower, "what do you sell")) {
        replyText = "Pricilla Mart offers 6 major shopping categories: 💻 Electronics, 👗 Fashion, ⌚ Accessories, 🛋️ Home, 💄 Beauty, and 📚 Books. Which one would you like to explore?";
        quickReplies = {"Electronics", "Fashion", "Accessories", "Home", "Beauty", "Books"};
    }
    // 11. CART & ORDER GUIDANCE
    else if (contains(msgLower, "cart") || contains(msgLower, "how to buy") || contains(msgLower, "how to order") || contains(msgLower, "checkout")) {
        replyText = "Ordering is easy on Pricilla Mart! 1) Browse products, 2) Click 'Add to Cart', 3) Open your Cart in the navbar, and 4) Proceed to Checkout with your delivery address. No real credit card is required for our capstone demo!";
        quickReplies = {"View Cart", "Browse Products", "Check My Orders"};
    }
    // 12. TRACKING / ORDER STATUS
    else if (contains(msgLower, "order") || contains(msgLower, "track") || contains(msgLower, "delivery") || contains(msgLower, "status")) {
        if (user.isValid) {
            auto orders = db.getOrdersByBuyer(user.userId);
            if (!orders.empty()) {
                const auto& latest = orders[0];
                replyText = "Your latest order is " + latest.orderCode + " (Status: " + latest.status + ") placed on " + latest.createdAt + " for ₹" + std::to_string(static_cast<int>(latest.totalAmount)) + ". You can review it anytime in your Orders tab!";
                quickReplies = {"Go to Orders", "Buy more items", "Write a review"};
            } else {
                replyText = "You don't have any active orders yet. Find exciting products on our homepage and place your first order today!";
                quickReplies = {"Explore Products", "Top Categories"};
            }
        } else {
            replyText = "To view your personalized order history and live tracking, please log in with your Buyer account.";
            quickReplies = {"Login now", "Register Account"};
        }
    }
    // 13. SELLER / BECOMING A SELLER
    else if (contains(msgLower, "seller") || contains(msgLower, "sell") || contains(msgLower, "store")) {
        replyText = "Pricilla Mart is a multi-seller platform! You can register with the 'Seller' role to get instant access to the Seller Dashboard, list your products, manage inventory, and fulfill customer orders.";
        quickReplies = {"Register as Seller", "Login"};
    }
    // 14. GENERAL SEARCH FALLBACK
    else {
        auto matches = db.searchProductsForChatbot(message, 3);
        if (!matches.empty()) {
            replyText = "Here are some items from our catalog matching your search:";
            recommendedProducts = matches;
            quickReplies = {"View Details", "Search more", "All Categories"};
        } else {
            replyText = "I'm here to help! You can ask me about laptops, smartphones, fashion, smartwatches, tracking your orders, or how to checkout.";
            quickReplies = {"Show Electronics", "Best Sellers", "Track Orders", "Categories"};
        }
    }

    std::ostringstream ss;
    ss << "{"
       << "\"reply\":\"" << ResponseUtil::escapeJson(replyText) << "\","
       << "\"recommendations\":[";
    for (size_t i = 0; i < recommendedProducts.size(); ++i) {
        const auto& p = recommendedProducts[i];
        if (i > 0) ss << ",";
        ss << "{"
           << "\"id\":" << p.id << ","
           << "\"name\":\"" << ResponseUtil::escapeJson(p.name) << "\","
           << "\"price\":" << p.price << ","
           << "\"imageUrl\":\"" << ResponseUtil::escapeJson(p.imageUrl) << "\","
           << "\"categoryName\":\"" << ResponseUtil::escapeJson(p.categoryName) << "\","
           << "\"stockQuantity\":" << p.stockQuantity
           << "}";
    }
    ss << "],\"quickReplies\":[";
    for (size_t i = 0; i < quickReplies.size(); ++i) {
        if (i > 0) ss << ",";
        ss << "\"" << ResponseUtil::escapeJson(quickReplies[i]) << "\"";
    }
    ss << "]}";

    resp.status = 200;
    resp.body = ResponseUtil::success("AI Chatbot response generated.", ss.str());
    return resp;
}

} // namespace PricillaMart
