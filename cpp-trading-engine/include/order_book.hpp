#ifndef TRADING_ORDER_BOOK_HPP
#define TRADING_ORDER_BOOK_HPP

#include "order.hpp"
#include <map>
#include <list>
#include <unordered_map>
#include <optional>
#include <vector>

namespace trading {

/**
 * @brief Represents a price level in the order book
 * 
 * Contains all orders at a specific price, maintaining FIFO order
 * for time priority.
 */
struct PriceLevel {
    Price price;
    Quantity total_quantity;
    std::list<Order> orders;  // FIFO queue for time priority
    
    PriceLevel(Price p = 0.0) : price(p), total_quantity(0) {}
    
    bool empty() const { return orders.empty(); }
    size_t order_count() const { return orders.size(); }
};

/**
 * @brief Order book implementation with price-time priority
 * 
 * Maintains separate bid and ask sides with efficient order lookup
 * and price level management.
 */
class OrderBook {
public:
    explicit OrderBook(const Symbol& symbol);
    ~OrderBook() = default;
    
    // Non-copyable
    OrderBook(const OrderBook&) = delete;
    OrderBook& operator=(const OrderBook&) = delete;
    
    // Movable
    OrderBook(OrderBook&&) = default;
    OrderBook& operator=(OrderBook&&) = default;
    
    /**
     * @brief Add an order to the book
     * @param order The order to add
     * @return true if order was added successfully
     */
    bool addOrder(Order order);
    
    /**
     * @brief Cancel an order by ID
     * @param order_id The order ID to cancel
     * @return true if order was found and cancelled
     */
    bool cancelOrder(OrderId order_id);
    
    /**
     * @brief Modify an existing order
     * @param order_id The order ID to modify
     * @param new_price New price (or 0 to keep current)
     * @param new_quantity New quantity (or 0 to keep current)
     * @return true if order was found and modified
     */
    bool modifyOrder(OrderId order_id, Price new_price, Quantity new_quantity);
    
    /**
     * @brief Get the best bid price and quantity
     * @return Optional pair of (price, quantity), empty if no bids
     */
    std::optional<std::pair<Price, Quantity>> getBestBid() const;
    
    /**
     * @brief Get the best ask price and quantity
     * @return Optional pair of (price, quantity), empty if no asks
     */
    std::optional<std::pair<Price, Quantity>> getBestAsk() const;
    
    /**
     * @brief Get the current spread
     * @return Optional spread value, empty if no two-sided market
     */
    std::optional<Price> getSpread() const;
    
    /**
     * @brief Get mid price
     * @return Optional mid price, empty if no two-sided market
     */
    std::optional<Price> getMidPrice() const;
    
    /**
     * @brief Get order by ID
     * @param order_id The order ID to look up
     * @return Pointer to order if found, nullptr otherwise
     */
    const Order* getOrder(OrderId order_id) const;
    
    /**
     * @brief Get multiple price levels from bid side
     * @param levels Number of levels to retrieve
     * @return Vector of price levels
     */
    std::vector<PriceLevel> getBidLevels(size_t levels = 5) const;
    
    /**
     * @brief Get multiple price levels from ask side
     * @param levels Number of levels to retrieve
     * @return Vector of price levels
     */
    std::vector<PriceLevel> getAskLevels(size_t levels = 5) const;
    
    /**
     * @brief Execute a fill against the best price level
     * @param side Side to execute against (opposite of aggressor)
     * @param quantity Maximum quantity to fill
     * @param limit_price Limit price (0 for market orders)
     * @return Vector of fills generated
     */
    std::vector<Fill> executeFill(Side aggressor_side, Quantity quantity, 
                                  Price limit_price, OrderId aggressor_id);
    
    // Accessors
    const Symbol& symbol() const { return symbol_; }
    size_t bidOrderCount() const { return bid_orders_.size(); }
    size_t askOrderCount() const { return ask_orders_.size(); }
    size_t totalOrderCount() const { return order_lookup_.size(); }
    
private:
    Symbol symbol_;
    
    // Bid side: sorted by price descending (highest first)
    std::map<Price, PriceLevel, std::greater<Price>> bid_levels_;
    
    // Ask side: sorted by price ascending (lowest first)
    std::map<Price, PriceLevel> ask_levels_;
    
    // Order ID to iterator lookup for O(1) cancel
    struct OrderLocation {
        Side side;
        Price price;
        std::list<Order>::iterator iter;
    };
    std::unordered_map<OrderId, OrderLocation> order_lookup_;
    
    // Order tracking by side
    std::unordered_map<OrderId, Order*> bid_orders_;
    std::unordered_map<OrderId, Order*> ask_orders_;
    
    // Helper to clean up empty price levels
    void cleanupLevel(Side side, Price price);
    
    // Helper to validate order parameters
    bool isValidOrder(const Order& order) const;
};

} // namespace trading

#endif // TRADING_ORDER_BOOK_HPP
