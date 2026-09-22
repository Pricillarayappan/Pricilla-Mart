#pragma once
#include <string>
#include <cstdlib>

namespace PricillaMart {

struct AppConfig {
    int port = 8080;
    std::string host = "0.0.0.0";
    std::string dbPath = "database/pricillamart.db";
    std::string schemaPath = "database/schema.sql";
    std::string seedPath = "database/seed.sql";
    std::string staticDir = "./frontend";
    std::string jwtSecret = "pricillamart_secret_key_college_capstone_2026";
    std::string chatbotApiKey = "";

    static AppConfig load() {
        AppConfig config;
        if (const char* p = std::getenv("PORT")) {
            config.port = std::stoi(p);
        }
        if (const char* h = std::getenv("HOST")) {
            config.host = h;
        }
        if (const char* db = std::getenv("DATABASE_PATH")) {
            config.dbPath = db;
        }
        if (const char* s = std::getenv("JWT_SECRET")) {
            config.jwtSecret = s;
        }
        if (const char* key = std::getenv("CHATBOT_API_KEY")) {
            config.chatbotApiKey = key;
        }
        return config;
    }
};

} // namespace PricillaMart
