#include "../include/matching_engine.hpp"
#include <iostream>
#include <cassert>
#include <vector>

using namespace trading;

void test_submit_limit_order() {
    std::cout << "Testing submitOrder (limit)..." << std::endl;
    
    MatchingEngine engine;
    
    // Submit sell order - should rest in book
    Order sell(1, "AAPL", Side::Sell, OrderType::Limit, 150.0, 100);
    auto fills = engine.submitOrder(sell);
    
    assert(fills.empty());  // No match
    
    // Submit buy order below ask - should rest in book
    Order buy(2, "AAPL", Side::Buy, OrderType::Limit, 149.0, 50);
    fills = engine.submitOrder(buy);
    
    assert(fills.empty());  // No match
    
    // Verify book state
    const OrderBook* book = engine.getOrderBook("AAPL");
    assert(book != nullptr);
    assert(book->bidOrderCount() == 1);
    assert(book->askOrderCount() == 1);
    
    std::cout << "  PASSED" << std::endl;
}

void test_limit_order_match() {
    std::cout << "Testing limit order matching..." << std::endl;
    
    MatchingEngine engine;
    
    // Setup resting orders
    engine.submitOrder(Order(1, "AAPL", Side::Sell, OrderType::Limit, 150.0, 100));
    engine.submitOrder(Order(2, "AAPL", Side::Sell, OrderType::Limit, 151.0, 200));
    
    // Submit crossing buy order
    Order buy(3, "AAPL", Side::Buy, OrderType::Limit, 150.5, 150);
    auto fills = engine.submitOrder(buy);
    
    // Should match against sell at 150.0
    assert(fills.size() == 1);
    assert(fills[0].price == 150.0);
    assert(fills[0].quantity == 100);
    
    // Remaining 50 shares should rest in book at 150.5
    const OrderBook* book = engine.getOrderBook("AAPL");
    auto best_bid = book->getBestBid();
    assert(best_bid.has_value());
    assert(best_bid->first == 150.5);
    assert(best_bid->second == 50);
    
    std::cout << "  PASSED" << std::endl;
}

void test_market_order() {
    std::cout << "Testing market order..." << std::endl;
    
    MatchingEngine engine;
    
    // Setup resting orders
    engine.submitOrder(Order(1, "AAPL", Side::Sell, OrderType::Limit, 150.0, 100));
    engine.submitOrder(Order(2, "AAPL", Side::Sell, OrderType::Limit, 151.0, 200));
    
    // Submit market buy
    Order market_buy(3, "AAPL", Side::Buy, OrderType::Market, 0, 250);
    auto fills = engine.submitOrder(market_buy);
    
    // Should match all available liquidity
    assert(fills.size() == 2);
    assert(fills[0].price == 150.0);
    assert(fills[0].quantity == 100);
    assert(fills[1].price == 151.0);
    assert(fills[1].quantity == 150);
    
    // Order book should have remaining sell at 151.0
    const OrderBook* book = engine.getOrderBook("AAPL");
    auto best_ask = book->getBestAsk();
    assert(best_ask.has_value());
    assert(best_ask->first == 151.0);
    assert(best_ask->second == 50);
    
    std::cout << "  PASSED" << std::endl;
}

void test_ioc_order() {
    std::cout << "Testing IOC order..." << std::endl;
    
    MatchingEngine engine;
    
    // Setup resting order
    engine.submitOrder(Order(1, "AAPL", Side::Sell, OrderType::Limit, 150.0, 50));
    
    // Submit IOC buy for more than available
    Order ioc(2, "AAPL", Side::Buy, OrderType::IOC, 150.0, 100);
    auto fills = engine.submitOrder(ioc);
    
    // Should fill 50 and cancel remaining
    assert(fills.size() == 1);
    assert(fills[0].quantity == 50);
    
    // No orders should rest in book
    const OrderBook* book = engine.getOrderBook("AAPL");
    assert(book->bidOrderCount() == 0);
    
    std::cout << "  PASSED" << std::endl;
}

void test_cancel_order() {
    std::cout << "Testing cancelOrder..." << std::endl;
    
    MatchingEngine engine;
    
    // Submit order
    engine.submitOrder(Order(1, "AAPL", Side::Buy, OrderType::Limit, 150.0, 100));
    
    // Cancel it
    assert(engine.cancelOrder("AAPL", 1));
    
    // Verify book is empty
    const OrderBook* book = engine.getOrderBook("AAPL");
    assert(book->totalOrderCount() == 0);
    
    // Try to cancel non-existent
    assert(!engine.cancelOrder("AAPL", 999));
    
    std::cout << "  PASSED" << std::endl;
}

void test_multiple_symbols() {
    std::cout << "Testing multiple symbols..." << std::endl;
    
    MatchingEngine engine;
    
    // Submit orders for different symbols
    engine.submitOrder(Order(1, "AAPL", Side::Buy, OrderType::Limit, 150.0, 100));
    engine.submitOrder(Order(2, "GOOGL", Side::Sell, OrderType::Limit, 2800.0, 50));
    engine.submitOrder(Order(3, "MSFT", Side::Buy, OrderType::Limit, 300.0, 75));
    
    // Verify separate order books
    assert(engine.getOrderBook("AAPL") != nullptr);
    assert(engine.getOrderBook("GOOGL") != nullptr);
    assert(engine.getOrderBook("MSFT") != nullptr);
    assert(engine.getOrderBook("AMZN") == nullptr);
    
    assert(engine.getOrderBook("AAPL")->bidOrderCount() == 1);
    assert(engine.getOrderBook("GOOGL")->askOrderCount() == 1);
    
    std::cout << "  PASSED" << std::endl;
}

void test_callbacks() {
    std::cout << "Testing callbacks..." << std::endl;
    
    MatchingEngine engine;
    
    std::vector<Fill> received_fills;
    std::vector<Order> received_orders;
    
    engine.setFillCallback([&](const Fill& f) {
        received_fills.push_back(f);
    });
    
    engine.setOrderCallback([&](const Order& o) {
        received_orders.push_back(o);
    });
    
    // Submit and match orders
    engine.submitOrder(Order(1, "AAPL", Side::Sell, OrderType::Limit, 150.0, 100));
    engine.submitOrder(Order(2, "AAPL", Side::Buy, OrderType::Limit, 150.0, 100));
    
    assert(received_fills.size() == 1);
    assert(received_orders.size() == 2);
    assert(received_fills[0].quantity == 100);
    
    std::cout << "  PASSED" << std::endl;
}

void test_with_risk_manager() {
    std::cout << "Testing with risk manager..." << std::endl;
    
    MatchingEngine engine;
    auto risk_mgr = std::make_shared<RiskManager>();
    
    risk_mgr->setOrderSizeLimit("AAPL", 100);
    engine.setRiskManager(risk_mgr);
    
    // Submit order within limit
    Order small(1, "AAPL", Side::Buy, OrderType::Limit, 150.0, 50);
    auto fills = engine.submitOrder(small);
    assert(engine.getOrderBook("AAPL")->bidOrderCount() == 1);
    
    // Submit order exceeding limit - should be rejected
    Order large(2, "AAPL", Side::Buy, OrderType::Limit, 150.0, 200);
    fills = engine.submitOrder(large);
    
    // Order should be rejected, not added to book
    assert(engine.getOrderBook("AAPL")->bidOrderCount() == 1);
    
    std::cout << "  PASSED" << std::endl;
}

void test_statistics() {
    std::cout << "Testing statistics..." << std::endl;
    
    MatchingEngine engine;
    
    engine.submitOrder(Order(1, "AAPL", Side::Sell, OrderType::Limit, 150.0, 100));
    engine.submitOrder(Order(2, "AAPL", Side::Buy, OrderType::Limit, 150.0, 100));
    engine.submitOrder(Order(3, "AAPL", Side::Buy, OrderType::Limit, 149.0, 50));
    
    assert(engine.totalOrdersProcessed() == 3);
    assert(engine.totalFillsGenerated() == 1);
    
    std::cout << "  PASSED" << std::endl;
}

int main() {
    std::cout << "\n=== Matching Engine Tests ===" << std::endl;
    
    test_submit_limit_order();
    test_limit_order_match();
    test_market_order();
    test_ioc_order();
    test_cancel_order();
    test_multiple_symbols();
    test_callbacks();
    test_with_risk_manager();
    test_statistics();
    
    std::cout << "\n=== All Matching Engine Tests Passed! ===" << std::endl;
    return 0;
}
