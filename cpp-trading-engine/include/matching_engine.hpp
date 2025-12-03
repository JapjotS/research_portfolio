#ifndef TRADING_MATCHING_ENGINE_HPP
#define TRADING_MATCHING_ENGINE_HPP

#include "order_book.hpp"
#include "risk_manager.hpp"
#include <memory>
#include <functional>

namespace trading {

/**
 * @brief Callback type for fill notifications
 */
using FillCallback = std::function<void(const Fill&)>;

/**
 * @brief Callback type for order status updates
 */
using OrderCallback = std::function<void(const Order&)>;

/**
 * @brief High-performance matching engine
 * 
 * Processes orders using price-time priority matching algorithm.
 * Single-threaded design optimized for low latency.
 */
class MatchingEngine {
public:
    MatchingEngine();
    ~MatchingEngine() = default;
    
    // Non-copyable
    MatchingEngine(const MatchingEngine&) = delete;
    MatchingEngine& operator=(const MatchingEngine&) = delete;
    
    /**
     * @brief Submit a new order
     * @param order The order to submit
     * @return Vector of fills generated (empty if no immediate matches)
     */
    std::vector<Fill> submitOrder(Order order);
    
    /**
     * @brief Cancel an existing order
     * @param symbol The symbol
     * @param order_id The order ID to cancel
     * @return true if order was found and cancelled
     */
    bool cancelOrder(const Symbol& symbol, OrderId order_id);
    
    /**
     * @brief Modify an existing order
     * @param symbol The symbol
     * @param order_id The order ID to modify
     * @param new_price New price (or 0 to keep current)
     * @param new_quantity New quantity (or 0 to keep current)
     * @return true if order was found and modified
     */
    bool modifyOrder(const Symbol& symbol, OrderId order_id, 
                     Price new_price, Quantity new_quantity);
    
    /**
     * @brief Get order book for a symbol
     * @param symbol The symbol to look up
     * @return Pointer to order book, nullptr if not found
     */
    const OrderBook* getOrderBook(const Symbol& symbol) const;
    
    /**
     * @brief Get or create order book for a symbol
     * @param symbol The symbol
     * @return Reference to the order book
     */
    OrderBook& getOrCreateOrderBook(const Symbol& symbol);
    
    /**
     * @brief Set fill callback
     * @param callback Function to call on each fill
     */
    void setFillCallback(FillCallback callback);
    
    /**
     * @brief Set order callback
     * @param callback Function to call on order status changes
     */
    void setOrderCallback(OrderCallback callback);
    
    /**
     * @brief Set risk manager
     * @param risk_manager Shared pointer to risk manager
     */
    void setRiskManager(std::shared_ptr<RiskManager> risk_manager);
    
    /**
     * @brief Get statistics
     */
    uint64_t totalOrdersProcessed() const { return total_orders_; }
    uint64_t totalFillsGenerated() const { return total_fills_; }
    
private:
    std::unordered_map<Symbol, std::unique_ptr<OrderBook>> order_books_;
    std::shared_ptr<RiskManager> risk_manager_;
    
    FillCallback fill_callback_;
    OrderCallback order_callback_;
    
    uint64_t total_orders_ = 0;
    uint64_t total_fills_ = 0;
    
    /**
     * @brief Match an order against the book
     * @param book The order book
     * @param order The aggressor order
     * @return Vector of fills generated
     */
    std::vector<Fill> matchOrder(OrderBook& book, Order& order);
    
    /**
     * @brief Notify fill via callback
     */
    void notifyFill(const Fill& fill);
    
    /**
     * @brief Notify order status via callback
     */
    void notifyOrder(const Order& order);
};

} // namespace trading

#endif // TRADING_MATCHING_ENGINE_HPP
