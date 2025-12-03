#include "matching_engine.hpp"

namespace trading {

MatchingEngine::MatchingEngine() = default;

std::vector<Fill> MatchingEngine::submitOrder(Order order) {
    ++total_orders_;
    
    // Risk check if risk manager is configured
    if (risk_manager_) {
        auto result = risk_manager_->checkOrder(order);
        if (!result) {
            order.reject();
            notifyOrder(order);
            return {};
        }
    }
    
    // Get or create order book
    auto& book = getOrCreateOrderBook(order.symbol);
    
    // Match the order
    auto fills = matchOrder(book, order);
    
    // Update risk manager with fills
    if (risk_manager_) {
        for (const auto& fill : fills) {
            risk_manager_->updatePosition(fill.symbol, fill.side, 
                                         fill.quantity, fill.price);
        }
    }
    
    // Notify order status
    notifyOrder(order);
    
    return fills;
}

bool MatchingEngine::cancelOrder(const Symbol& symbol, OrderId order_id) {
    auto it = order_books_.find(symbol);
    if (it == order_books_.end()) {
        return false;
    }
    
    return it->second->cancelOrder(order_id);
}

bool MatchingEngine::modifyOrder(const Symbol& symbol, OrderId order_id,
                                  Price new_price, Quantity new_quantity) {
    auto it = order_books_.find(symbol);
    if (it == order_books_.end()) {
        return false;
    }
    
    return it->second->modifyOrder(order_id, new_price, new_quantity);
}

const OrderBook* MatchingEngine::getOrderBook(const Symbol& symbol) const {
    auto it = order_books_.find(symbol);
    if (it == order_books_.end()) {
        return nullptr;
    }
    return it->second.get();
}

OrderBook& MatchingEngine::getOrCreateOrderBook(const Symbol& symbol) {
    auto it = order_books_.find(symbol);
    if (it == order_books_.end()) {
        auto [new_it, inserted] = order_books_.emplace(
            symbol, std::make_unique<OrderBook>(symbol));
        return *new_it->second;
    }
    return *it->second;
}

void MatchingEngine::setFillCallback(FillCallback callback) {
    fill_callback_ = std::move(callback);
}

void MatchingEngine::setOrderCallback(OrderCallback callback) {
    order_callback_ = std::move(callback);
}

void MatchingEngine::setRiskManager(std::shared_ptr<RiskManager> risk_manager) {
    risk_manager_ = std::move(risk_manager);
}

std::vector<Fill> MatchingEngine::matchOrder(OrderBook& book, Order& order) {
    std::vector<Fill> fills;
    
    // Market orders: set very aggressive price
    Price limit_price = order.price;
    if (order.type == OrderType::Market) {
        limit_price = (order.side == Side::Buy) ? 1e12 : 0.0;
    }
    
    // Try to match against resting orders
    if (order.remaining_qty() > 0) {
        fills = book.executeFill(order.side, order.remaining_qty(), 
                                 limit_price, order.id);
        
        // Update order with fills
        for (const auto& fill : fills) {
            order.apply_fill(fill.quantity);
            notifyFill(fill);
            ++total_fills_;
        }
    }
    
    // Handle remaining quantity based on order type
    if (order.remaining_qty() > 0) {
        switch (order.type) {
            case OrderType::Limit:
                // Add remaining to book
                book.addOrder(order);
                break;
                
            case OrderType::Market:
                // Cancel remaining (couldn't fill at any price)
                order.cancel();
                break;
                
            case OrderType::IOC:
                // Cancel remaining (immediate-or-cancel)
                order.cancel();
                break;
                
            case OrderType::FOK:
                // Should have been checked before - cancel all
                // Note: FOK logic is simplified here
                order.cancel();
                break;
        }
    }
    
    return fills;
}

void MatchingEngine::notifyFill(const Fill& fill) {
    if (fill_callback_) {
        fill_callback_(fill);
    }
}

void MatchingEngine::notifyOrder(const Order& order) {
    if (order_callback_) {
        order_callback_(order);
    }
}

} // namespace trading
