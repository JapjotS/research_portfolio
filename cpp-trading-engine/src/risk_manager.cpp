#include "risk_manager.hpp"
#include <cmath>

namespace trading {

RiskManager::RiskManager() 
    : rate_window_start_(std::chrono::steady_clock::now()) {}

RiskCheckResult RiskManager::checkOrder(const Order& order) {
    // Check order rate limit
    auto rate_check = checkOrderRate();
    if (!rate_check) {
        return rate_check;
    }
    
    // Check order size limit
    auto size_check = checkOrderSizeLimit(order);
    if (!size_check) {
        return size_check;
    }
    
    // Check position limit
    auto pos_check = checkPositionLimit(order);
    if (!pos_check) {
        return pos_check;
    }
    
    // Check notional limit
    auto notional_check = checkNotionalLimit(order);
    if (!notional_check) {
        return notional_check;
    }
    
    return RiskCheckResult(true);
}

void RiskManager::updatePosition(const Symbol& symbol, Side side,
                                  Quantity quantity, Price price) {
    Quantity direction = (side == Side::Buy) ? 1 : -1;
    Quantity position_change = direction * quantity;
    
    positions_[symbol] += position_change;
    
    // Update average price and notional exposure
    double notional = price * quantity;
    if (direction > 0) {
        notional_exposures_[symbol] += notional;
    } else {
        notional_exposures_[symbol] -= notional;
    }
}

void RiskManager::setPositionLimit(const Symbol& symbol, Quantity limit) {
    position_limits_[symbol] = limit;
}

Quantity RiskManager::getPositionLimit(const Symbol& symbol) const {
    auto it = position_limits_.find(symbol);
    return (it != position_limits_.end()) ? it->second : DEFAULT_POSITION_LIMIT;
}

void RiskManager::setOrderSizeLimit(const Symbol& symbol, Quantity limit) {
    order_size_limits_[symbol] = limit;
}

Quantity RiskManager::getOrderSizeLimit(const Symbol& symbol) const {
    auto it = order_size_limits_.find(symbol);
    return (it != order_size_limits_.end()) ? it->second : DEFAULT_ORDER_SIZE_LIMIT;
}

void RiskManager::setNotionalLimit(const Symbol& symbol, double limit) {
    notional_limits_[symbol] = limit;
}

double RiskManager::getNotionalLimit(const Symbol& symbol) const {
    auto it = notional_limits_.find(symbol);
    return (it != notional_limits_.end()) ? it->second : DEFAULT_NOTIONAL_LIMIT;
}

void RiskManager::setOrderRateLimit(size_t orders_per_second) {
    order_rate_limit_ = orders_per_second;
}

size_t RiskManager::getOrderRateLimit() const {
    return order_rate_limit_;
}

void RiskManager::setGlobalPositionLimit(Quantity limit) {
    global_position_limit_ = limit;
}

void RiskManager::setGlobalNotionalLimit(double limit) {
    global_notional_limit_ = limit;
}

Quantity RiskManager::getPosition(const Symbol& symbol) const {
    auto it = positions_.find(symbol);
    return (it != positions_.end()) ? it->second : 0;
}

double RiskManager::getNotionalExposure(const Symbol& symbol) const {
    auto it = notional_exposures_.find(symbol);
    return (it != notional_exposures_.end()) ? it->second : 0.0;
}

double RiskManager::getTotalNotionalExposure() const {
    double total = 0.0;
    for (const auto& [symbol, exposure] : notional_exposures_) {
        total += std::abs(exposure);
    }
    return total;
}

void RiskManager::reset() {
    positions_.clear();
    average_prices_.clear();
    notional_exposures_.clear();
    orders_this_second_ = 0;
    rate_window_start_ = std::chrono::steady_clock::now();
}

RiskCheckResult RiskManager::checkPositionLimit(const Order& order) const {
    Quantity current_pos = getPosition(order.symbol);
    Quantity limit = getPositionLimit(order.symbol);
    
    Quantity direction = (order.side == Side::Buy) ? 1 : -1;
    Quantity new_pos = current_pos + direction * order.quantity;
    
    if (std::abs(new_pos) > limit) {
        return RiskCheckResult(false, 
            "Position limit exceeded: " + std::to_string(new_pos) + 
            " > " + std::to_string(limit));
    }
    
    // Check global position limit
    if (global_position_limit_ > 0) {
        Quantity total_pos = 0;
        for (const auto& [sym, pos] : positions_) {
            if (sym == order.symbol) {
                total_pos += std::abs(new_pos);
            } else {
                total_pos += std::abs(pos);
            }
        }
        if (total_pos > global_position_limit_) {
            return RiskCheckResult(false, "Global position limit exceeded");
        }
    }
    
    return RiskCheckResult(true);
}

RiskCheckResult RiskManager::checkOrderSizeLimit(const Order& order) const {
    Quantity limit = getOrderSizeLimit(order.symbol);
    
    if (order.quantity > limit) {
        return RiskCheckResult(false,
            "Order size limit exceeded: " + std::to_string(order.quantity) +
            " > " + std::to_string(limit));
    }
    
    return RiskCheckResult(true);
}

RiskCheckResult RiskManager::checkNotionalLimit(const Order& order) const {
    double limit = getNotionalLimit(order.symbol);
    double current = getNotionalExposure(order.symbol);
    double order_notional = order.price * order.quantity;
    
    Quantity direction = (order.side == Side::Buy) ? 1 : -1;
    double new_exposure = current + direction * order_notional;
    
    if (std::abs(new_exposure) > limit) {
        return RiskCheckResult(false,
            "Notional limit exceeded: " + std::to_string(new_exposure) +
            " > " + std::to_string(limit));
    }
    
    // Check global notional limit
    if (global_notional_limit_ > 0) {
        double total_exposure = 0.0;
        for (const auto& [sym, exp] : notional_exposures_) {
            if (sym == order.symbol) {
                total_exposure += std::abs(new_exposure);
            } else {
                total_exposure += std::abs(exp);
            }
        }
        if (total_exposure > global_notional_limit_) {
            return RiskCheckResult(false, "Global notional limit exceeded");
        }
    }
    
    return RiskCheckResult(true);
}

RiskCheckResult RiskManager::checkOrderRate() {
    if (order_rate_limit_ == 0) {
        return RiskCheckResult(true);
    }
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        now - rate_window_start_).count();
    
    if (elapsed >= 1) {
        // Reset window
        orders_this_second_ = 0;
        rate_window_start_ = now;
    }
    
    if (orders_this_second_ >= order_rate_limit_) {
        return RiskCheckResult(false, "Order rate limit exceeded");
    }
    
    ++orders_this_second_;
    return RiskCheckResult(true);
}

} // namespace trading
