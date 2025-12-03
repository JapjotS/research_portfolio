#include "order_book.hpp"
#include <algorithm>

namespace trading {

OrderBook::OrderBook(const Symbol& symbol) : symbol_(symbol) {}

/**
 * @brief Validate order parameters
 * @param order The order to validate
 * @return true if order is valid
 */
bool OrderBook::isValidOrder(const Order& order) const {
    return order.remaining_qty() > 0 && order.price >= 0;
}

bool OrderBook::addOrder(Order order) {
    if (!isValidOrder(order)) {
        return false;
    }
    
    // Check for duplicate order ID
    if (order_lookup_.find(order.id) != order_lookup_.end()) {
        return false;
    }
    
    // Get the appropriate side
    if (order.side == Side::Buy) {
        auto& level = bid_levels_[order.price];
        if (level.orders.empty()) {
            level.price = order.price;
        }
        level.orders.push_back(order);
        level.total_quantity += order.remaining_qty();
        
        auto iter = std::prev(level.orders.end());
        order_lookup_[order.id] = {Side::Buy, order.price, iter};
        bid_orders_[order.id] = &(*iter);
    } else {
        auto& level = ask_levels_[order.price];
        if (level.orders.empty()) {
            level.price = order.price;
        }
        level.orders.push_back(order);
        level.total_quantity += order.remaining_qty();
        
        auto iter = std::prev(level.orders.end());
        order_lookup_[order.id] = {Side::Sell, order.price, iter};
        ask_orders_[order.id] = &(*iter);
    }
    
    return true;
}

bool OrderBook::cancelOrder(OrderId order_id) {
    auto it = order_lookup_.find(order_id);
    if (it == order_lookup_.end()) {
        return false;
    }
    
    const auto& loc = it->second;
    
    if (loc.side == Side::Buy) {
        auto level_it = bid_levels_.find(loc.price);
        if (level_it != bid_levels_.end()) {
            level_it->second.total_quantity -= loc.iter->remaining_qty();
            level_it->second.orders.erase(loc.iter);
            
            if (level_it->second.orders.empty()) {
                bid_levels_.erase(level_it);
            }
        }
        bid_orders_.erase(order_id);
    } else {
        auto level_it = ask_levels_.find(loc.price);
        if (level_it != ask_levels_.end()) {
            level_it->second.total_quantity -= loc.iter->remaining_qty();
            level_it->second.orders.erase(loc.iter);
            
            if (level_it->second.orders.empty()) {
                ask_levels_.erase(level_it);
            }
        }
        ask_orders_.erase(order_id);
    }
    
    order_lookup_.erase(it);
    return true;
}

bool OrderBook::modifyOrder(OrderId order_id, Price new_price, Quantity new_quantity) {
    auto it = order_lookup_.find(order_id);
    if (it == order_lookup_.end()) {
        return false;
    }
    
    const auto& loc = it->second;
    Order old_order = *loc.iter;
    
    // If price changes, need to remove and re-add
    if (new_price > 0 && new_price != loc.price) {
        cancelOrder(order_id);
        old_order.price = new_price;
        if (new_quantity > 0) {
            old_order.quantity = new_quantity;
        }
        old_order.timestamp = std::chrono::steady_clock::now();
        return addOrder(old_order);
    }
    
    // Price unchanged, just modify quantity
    if (new_quantity > 0 && new_quantity != loc.iter->quantity) {
        Quantity diff = new_quantity - loc.iter->quantity;
        loc.iter->quantity = new_quantity;
        
        if (loc.side == Side::Buy) {
            bid_levels_[loc.price].total_quantity += diff;
        } else {
            ask_levels_[loc.price].total_quantity += diff;
        }
    }
    
    return true;
}

std::optional<std::pair<Price, Quantity>> OrderBook::getBestBid() const {
    if (bid_levels_.empty()) {
        return std::nullopt;
    }
    const auto& best = bid_levels_.begin()->second;
    return std::make_pair(best.price, best.total_quantity);
}

std::optional<std::pair<Price, Quantity>> OrderBook::getBestAsk() const {
    if (ask_levels_.empty()) {
        return std::nullopt;
    }
    const auto& best = ask_levels_.begin()->second;
    return std::make_pair(best.price, best.total_quantity);
}

std::optional<Price> OrderBook::getSpread() const {
    auto bid = getBestBid();
    auto ask = getBestAsk();
    
    if (!bid || !ask) {
        return std::nullopt;
    }
    
    return ask->first - bid->first;
}

std::optional<Price> OrderBook::getMidPrice() const {
    auto bid = getBestBid();
    auto ask = getBestAsk();
    
    if (!bid || !ask) {
        return std::nullopt;
    }
    
    return (bid->first + ask->first) / 2.0;
}

const Order* OrderBook::getOrder(OrderId order_id) const {
    auto it = order_lookup_.find(order_id);
    if (it == order_lookup_.end()) {
        return nullptr;
    }
    return &(*it->second.iter);
}

std::vector<PriceLevel> OrderBook::getBidLevels(size_t levels) const {
    std::vector<PriceLevel> result;
    result.reserve(levels);
    
    size_t count = 0;
    for (const auto& [price, level] : bid_levels_) {
        if (count >= levels) break;
        result.push_back(level);
        ++count;
    }
    
    return result;
}

std::vector<PriceLevel> OrderBook::getAskLevels(size_t levels) const {
    std::vector<PriceLevel> result;
    result.reserve(levels);
    
    size_t count = 0;
    for (const auto& [price, level] : ask_levels_) {
        if (count >= levels) break;
        result.push_back(level);
        ++count;
    }
    
    return result;
}

std::vector<Fill> OrderBook::executeFill(Side aggressor_side, Quantity quantity,
                                          Price limit_price, OrderId aggressor_id) {
    std::vector<Fill> fills;
    Quantity remaining = quantity;
    
    // Match against ask side for buy orders
    if (aggressor_side == Side::Buy) {
        while (remaining > 0 && !ask_levels_.empty()) {
            auto level_it = ask_levels_.begin();
            auto& level = level_it->second;
            
            // Check price limit
            if (limit_price > 0 && level.price > limit_price) {
                break;
            }
            
            // Match against orders at this price level
            while (remaining > 0 && !level.orders.empty()) {
                auto& passive_order = level.orders.front();
                
                Quantity fill_qty = std::min(remaining, passive_order.remaining_qty());
                
                // Create fill
                fills.emplace_back(
                    aggressor_id,
                    passive_order.id,
                    symbol_,
                    aggressor_side,
                    level.price,
                    fill_qty
                );
                
                // Update passive order
                passive_order.apply_fill(fill_qty);
                level.total_quantity -= fill_qty;
                remaining -= fill_qty;
                
                // Remove filled order
                if (passive_order.is_filled()) {
                    OrderId filled_id = passive_order.id;
                    level.orders.pop_front();
                    order_lookup_.erase(filled_id);
                    ask_orders_.erase(filled_id);
                }
            }
            
            // Remove empty price level
            if (level.orders.empty()) {
                ask_levels_.erase(level_it);
            }
        }
    }
    // Match against bid side for sell orders
    else {
        while (remaining > 0 && !bid_levels_.empty()) {
            auto level_it = bid_levels_.begin();
            auto& level = level_it->second;
            
            // Check price limit
            if (limit_price > 0 && level.price < limit_price) {
                break;
            }
            
            // Match against orders at this price level
            while (remaining > 0 && !level.orders.empty()) {
                auto& passive_order = level.orders.front();
                
                Quantity fill_qty = std::min(remaining, passive_order.remaining_qty());
                
                // Create fill
                fills.emplace_back(
                    aggressor_id,
                    passive_order.id,
                    symbol_,
                    aggressor_side,
                    level.price,
                    fill_qty
                );
                
                // Update passive order
                passive_order.apply_fill(fill_qty);
                level.total_quantity -= fill_qty;
                remaining -= fill_qty;
                
                // Remove filled order
                if (passive_order.is_filled()) {
                    OrderId filled_id = passive_order.id;
                    level.orders.pop_front();
                    order_lookup_.erase(filled_id);
                    bid_orders_.erase(filled_id);
                }
            }
            
            // Remove empty price level
            if (level.orders.empty()) {
                bid_levels_.erase(level_it);
            }
        }
    }
    
    return fills;
}

void OrderBook::cleanupLevel(Side side, Price price) {
    if (side == Side::Buy) {
        auto it = bid_levels_.find(price);
        if (it != bid_levels_.end() && it->second.orders.empty()) {
            bid_levels_.erase(it);
        }
    } else {
        auto it = ask_levels_.find(price);
        if (it != ask_levels_.end() && it->second.orders.empty()) {
            ask_levels_.erase(it);
        }
    }
}

} // namespace trading
