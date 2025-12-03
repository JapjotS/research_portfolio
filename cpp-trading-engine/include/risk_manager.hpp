#ifndef TRADING_RISK_MANAGER_HPP
#define TRADING_RISK_MANAGER_HPP

#include "order.hpp"
#include <unordered_map>
#include <string>
#include <mutex>

namespace trading {

/**
 * @brief Risk check result
 */
struct RiskCheckResult {
    bool passed;
    std::string reason;
    
    RiskCheckResult() : passed(true), reason("") {}
    RiskCheckResult(bool p, const std::string& r = "") : passed(p), reason(r) {}
    
    operator bool() const { return passed; }
};

/**
 * @brief Pre-trade risk management
 * 
 * Performs various risk checks before orders are submitted to the
 * matching engine.
 */
class RiskManager {
public:
    RiskManager();
    ~RiskManager() = default;
    
    /**
     * @brief Check if an order passes all risk limits
     * @param order The order to check
     * @return RiskCheckResult with pass/fail and reason
     */
    RiskCheckResult checkOrder(const Order& order);
    
    /**
     * @brief Update position after a fill
     * @param symbol The symbol
     * @param side The fill side
     * @param quantity The filled quantity
     * @param price The fill price
     */
    void updatePosition(const Symbol& symbol, Side side, 
                       Quantity quantity, Price price);
    
    // Position Limits
    void setPositionLimit(const Symbol& symbol, Quantity limit);
    Quantity getPositionLimit(const Symbol& symbol) const;
    
    // Order Size Limits
    void setOrderSizeLimit(const Symbol& symbol, Quantity limit);
    Quantity getOrderSizeLimit(const Symbol& symbol) const;
    
    // Notional Value Limits
    void setNotionalLimit(const Symbol& symbol, double limit);
    double getNotionalLimit(const Symbol& symbol) const;
    
    // Order Rate Limits
    void setOrderRateLimit(size_t orders_per_second);
    size_t getOrderRateLimit() const;
    
    // Global Limits
    void setGlobalPositionLimit(Quantity limit);
    void setGlobalNotionalLimit(double limit);
    
    // Position Queries
    Quantity getPosition(const Symbol& symbol) const;
    double getNotionalExposure(const Symbol& symbol) const;
    double getTotalNotionalExposure() const;
    
    // Reset state
    void reset();
    
private:
    // Per-symbol limits
    std::unordered_map<Symbol, Quantity> position_limits_;
    std::unordered_map<Symbol, Quantity> order_size_limits_;
    std::unordered_map<Symbol, double> notional_limits_;
    
    // Current positions
    std::unordered_map<Symbol, Quantity> positions_;
    std::unordered_map<Symbol, double> average_prices_;
    std::unordered_map<Symbol, double> notional_exposures_;
    
    // Global limits
    Quantity global_position_limit_ = 0;
    double global_notional_limit_ = 0.0;
    size_t order_rate_limit_ = 0;
    
    // Order rate tracking
    size_t orders_this_second_ = 0;
    std::chrono::steady_clock::time_point rate_window_start_;
    
    // Default limits
    static constexpr Quantity DEFAULT_POSITION_LIMIT = 100000;
    static constexpr Quantity DEFAULT_ORDER_SIZE_LIMIT = 10000;
    static constexpr double DEFAULT_NOTIONAL_LIMIT = 10000000.0;
    
    // Helper functions
    RiskCheckResult checkPositionLimit(const Order& order) const;
    RiskCheckResult checkOrderSizeLimit(const Order& order) const;
    RiskCheckResult checkNotionalLimit(const Order& order) const;
    RiskCheckResult checkOrderRate();
};

} // namespace trading

#endif // TRADING_RISK_MANAGER_HPP
