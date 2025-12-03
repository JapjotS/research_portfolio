#ifndef TRADING_ORDER_HPP
#define TRADING_ORDER_HPP

#include "types.hpp"
#include <iostream>

namespace trading {

/**
 * @brief Represents a trading order
 * 
 * Contains all information needed to process an order including
 * identification, pricing, quantity, and timing information.
 */
struct Order {
    OrderId id;              // Unique order identifier
    Symbol symbol;           // Trading symbol (e.g., "AAPL")
    Side side;               // Buy or Sell
    OrderType type;          // Order type (Limit, Market, etc.)
    Price price;             // Limit price (0 for market orders)
    Quantity quantity;       // Original order quantity
    Quantity filled_qty;     // Quantity already filled
    OrderStatus status;      // Current order status
    Timestamp timestamp;     // Order submission time
    
    // Default constructor
    Order() 
        : id(0)
        , symbol("")
        , side(Side::Buy)
        , type(OrderType::Limit)
        , price(0.0)
        , quantity(0)
        , filled_qty(0)
        , status(OrderStatus::New)
        , timestamp(std::chrono::steady_clock::now())
    {}
    
    // Parameterized constructor
    Order(OrderId id, const Symbol& symbol, Side side, OrderType type,
          Price price, Quantity quantity)
        : id(id)
        , symbol(symbol)
        , side(side)
        , type(type)
        , price(price)
        , quantity(quantity)
        , filled_qty(0)
        , status(OrderStatus::New)
        , timestamp(std::chrono::steady_clock::now())
    {}
    
    // Get remaining quantity to be filled
    Quantity remaining_qty() const {
        return quantity - filled_qty;
    }
    
    // Check if order is fully filled
    bool is_filled() const {
        return filled_qty >= quantity;
    }
    
    // Check if order is active (can be matched)
    bool is_active() const {
        return status == OrderStatus::New || 
               status == OrderStatus::PartiallyFilled;
    }
    
    // Apply a fill to this order
    void apply_fill(Quantity fill_qty) {
        filled_qty += fill_qty;
        if (filled_qty >= quantity) {
            status = OrderStatus::Filled;
        } else if (filled_qty > 0) {
            status = OrderStatus::PartiallyFilled;
        }
    }
    
    // Cancel the order
    void cancel() {
        status = OrderStatus::Cancelled;
    }
    
    // Reject the order
    void reject() {
        status = OrderStatus::Rejected;
    }
};

/**
 * @brief Represents a trade execution (fill)
 * 
 * Generated when an order is matched against resting liquidity.
 */
struct Fill {
    OrderId order_id;        // Order that was filled
    OrderId counter_order_id; // Counter-party order
    Symbol symbol;           // Trading symbol
    Side side;               // Side of the aggressor order
    Price price;             // Execution price
    Quantity quantity;       // Executed quantity
    Timestamp timestamp;     // Execution time
    
    Fill(OrderId order_id, OrderId counter_id, const Symbol& symbol,
         Side side, Price price, Quantity quantity)
        : order_id(order_id)
        , counter_order_id(counter_id)
        , symbol(symbol)
        , side(side)
        , price(price)
        , quantity(quantity)
        , timestamp(std::chrono::steady_clock::now())
    {}
};

// Stream output for Order
inline std::ostream& operator<<(std::ostream& os, const Order& order) {
    os << "Order{id=" << order.id 
       << ", symbol=" << order.symbol
       << ", side=" << to_string(order.side)
       << ", type=" << to_string(order.type)
       << ", price=" << order.price
       << ", qty=" << order.quantity
       << ", filled=" << order.filled_qty
       << ", status=" << to_string(order.status)
       << "}";
    return os;
}

// Stream output for Fill
inline std::ostream& operator<<(std::ostream& os, const Fill& fill) {
    os << "Fill{order_id=" << fill.order_id
       << ", counter_id=" << fill.counter_order_id
       << ", symbol=" << fill.symbol
       << ", side=" << to_string(fill.side)
       << ", price=" << fill.price
       << ", qty=" << fill.quantity
       << "}";
    return os;
}

} // namespace trading

#endif // TRADING_ORDER_HPP
