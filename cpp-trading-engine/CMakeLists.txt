cmake_minimum_required(VERSION 3.14)
project(TradingEngine VERSION 1.0.0 LANGUAGES CXX)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
elseif(MSVC)
    add_compile_options(/W4 /O2)
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/order_book.cpp
    src/matching_engine.cpp
    src/risk_manager.cpp
)

# Create library
add_library(trading_engine STATIC ${SOURCES})

# Option to build tests
option(BUILD_TESTS "Build test executables" ON)

if(BUILD_TESTS)
    enable_testing()
    
    # Order book tests
    add_executable(test_order_book tests/test_order_book.cpp)
    target_link_libraries(test_order_book trading_engine)
    add_test(NAME OrderBookTests COMMAND test_order_book)
    
    # Matching engine tests
    add_executable(test_matching_engine tests/test_matching_engine.cpp)
    target_link_libraries(test_matching_engine trading_engine)
    add_test(NAME MatchingEngineTests COMMAND test_matching_engine)
endif()

# Installation
install(TARGETS trading_engine
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)
