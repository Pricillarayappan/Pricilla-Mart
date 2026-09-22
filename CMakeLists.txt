cmake_minimum_required(VERSION 3.15)
project(PricillaMart CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/backend
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/third_party
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/controllers
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/models
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/database
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/config
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/middleware
)

# Common Backend Sources
set(COMMON_SOURCES
    backend/database/DatabaseManager.cpp
    backend/utils/PasswordHasher.cpp
    backend/controllers/AuthController.cpp
    backend/controllers/ProductController.cpp
    backend/controllers/CategoryController.cpp
    backend/controllers/CartController.cpp
    backend/controllers/OrderController.cpp
    backend/controllers/ReviewController.cpp
    backend/controllers/AdminController.cpp
    backend/controllers/ChatbotController.cpp
)

# Check for SQLite3
find_package(SQLite3 QUIET)
if (SQLite3_FOUND)
    message(STATUS "Using system SQLite3: ${SQLite3_INCLUDE_DIRS}")
    set(SQLITE_LIB SQLite::SQLite3)
else()
    message(STATUS "Using embedded SQLite3 amalgamation")
    set(EMBEDDED_SQLITE backend/third_party/sqlite3.c)
endif()

# Check for Drogon Framework
find_package(Drogon QUIET)
if (Drogon_FOUND)
    message(STATUS "Drogon Framework found: ${Drogon_DIR}")
    add_executable(pricillamart_drogon
        backend/drogon_main.cpp
        ${COMMON_SOURCES}
        ${EMBEDDED_SQLITE}
    )
    target_link_libraries(pricillamart_drogon PRIVATE Drogon::Drogon ${SQLITE_LIB})
    if (WIN32)
        target_link_libraries(pricillamart_drogon PRIVATE ws2_32 wsock32 crypt32)
    endif()
endif()

# Standalone Capstone C++ REST Server (Runs anywhere on Windows/Linux/macOS out of the box)
add_executable(pricillamart_server
    backend/main.cpp
    ${COMMON_SOURCES}
    ${EMBEDDED_SQLITE}
)

if (WIN32)
    target_link_libraries(pricillamart_server PRIVATE ws2_32 crypt32)
endif()

if (SQLite3_FOUND)
    target_link_libraries(pricillamart_server PRIVATE SQLite::SQLite3)
endif()

# Compiler options
if (MSVC)
    target_compile_options(pricillamart_server PRIVATE /W4 /utf-8)
else()
    target_compile_options(pricillamart_server PRIVATE -Wall -Wextra -pthread)
endif()
