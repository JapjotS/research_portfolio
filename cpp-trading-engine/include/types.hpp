#ifndef TRADING_TYPES_HPP
#define TRADING_TYPES_HPP

#include <cstdint>
#include <string>
#include <chrono>

namespace trading {

// Type aliases for clarity and potential future changes
using OrderId = uint64_t;
using Price = double;
using Quantity = int64_t;
using Symbol = std::string;
using Timestamp = std::chrono::steady_clock::time_point;

// Order side enumeration
enum class Side : uint8_t {
    Buy = 0,
    Sell = 1
};

// Order type enumeration
enum class OrderType : uint8_t {
    Limit = 0,      // Execute at specified price or better
    Market = 1,     // Execute at best available price
    IOC = 2,        // Immediate-or-Cancel: fill what you can, cancel rest
    FOK = 3         // Fill-or-Kill: fill entire order or cancel
};

// Order status enumeration
enum class OrderStatus : uint8_t {
    New = 0,
    PartiallyFilled = 1,
    Filled = 2,
    Cancelled = 3,
    Rejected = 4
};

// Convert Side to string
inline const char* to_string(Side side) {
    switch (side) {
        case Side::Buy: return "BUY";
        case Side::Sell: return "SELL";
        default: return "UNKNOWN";
    }
}

// Convert OrderType to string
inline const char* to_string(OrderType type) {
    switch (type) {
        case OrderType::Limit: return "LIMIT";
        case OrderType::Market: return "MARKET";
        case OrderType::IOC: return "IOC";
        case OrderType::FOK: return "FOK";
        default: return "UNKNOWN";
    }
}

// Convert OrderStatus to string
inline const char* to_string(OrderStatus status) {
    switch (status) {
        case OrderStatus::New: return "NEW";
        case OrderStatus::PartiallyFilled: return "PARTIAL";
        case OrderStatus::Filled: return "FILLED";
        case OrderStatus::Cancelled: return "CANCELLED";
        case OrderStatus::Rejected: return "REJECTED";
        default: return "UNKNOWN";
    }
}

} // namespace trading

#endif // TRADING_TYPES_HPP
