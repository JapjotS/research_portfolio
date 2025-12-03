#include "../include/order_book.hpp"
#include <iostream>
#include <cassert>

using namespace trading;

void test_add_order() {
    std::cout << "Testing addOrder..." << std::endl;
    
    OrderBook book("AAPL");
    
    // Add buy order
    Order buy_order(1, "AAPL", Side::Buy, OrderType::Limit, 150.0, 100);
    assert(book.addOrder(buy_order));
    assert(book.bidOrderCount() == 1);
    assert(book.askOrderCount() == 0);
    
    // Add sell order
    Order sell_order(2, "AAPL", Side::Sell, OrderType::Limit, 151.0, 50);
    assert(book.addOrder(sell_order));
    assert(book.bidOrderCount() == 1);
    assert(book.askOrderCount() == 1);
    
    // Check best bid/ask
    auto best_bid = book.getBestBid();
    assert(best_bid.has_value());
    assert(best_bid->first == 150.0);
    assert(best_bid->second == 100);
    
    auto best_ask = book.getBestAsk();
    assert(best_ask.has_value());
    assert(best_ask->first == 151.0);
    assert(best_ask->second == 50);
    
    // Check spread
    auto spread = book.getSpread();
    assert(spread.has_value());
    assert(spread.value() == 1.0);
    
    std::cout << "  PASSED" << std::endl;
}

void test_cancel_order() {
    std::cout << "Testing cancelOrder..." << std::endl;
    
    OrderBook book("AAPL");
    
    // Add orders
    Order order1(1, "AAPL", Side::Buy, OrderType::Limit, 150.0, 100);
    Order order2(2, "AAPL", Side::Buy, OrderType::Limit, 149.0, 200);
    book.addOrder(order1);
    book.addOrder(order2);
    
    assert(book.totalOrderCount() == 2);
    
    // Cancel first order
    assert(book.cancelOrder(1));
    assert(book.totalOrderCount() == 1);
    
    // Try to cancel non-existent order
    assert(!book.cancelOrder(999));
    
    // Best bid should now be 149.0
    auto best_bid = book.getBestBid();
    assert(best_bid.has_value());
    assert(best_bid->first == 149.0);
    
    std::cout << "  PASSED" << std::endl;
}

void test_multiple_price_levels() {
    std::cout << "Testing multiple price levels..." << std::endl;
    
    OrderBook book("AAPL");
    
    // Add orders at different price levels
    book.addOrder(Order(1, "AAPL", Side::Buy, OrderType::Limit, 150.0, 100));
    book.addOrder(Order(2, "AAPL", Side::Buy, OrderType::Limit, 149.5, 200));
    book.addOrder(Order(3, "AAPL", Side::Buy, OrderType::Limit, 149.0, 150));
    book.addOrder(Order(4, "AAPL", Side::Buy, OrderType::Limit, 150.0, 50));  // Same level as order 1
    
    // Get bid levels
    auto levels = book.getBidLevels(5);
    assert(levels.size() == 3);
    
    // Check ordering (highest price first)
    assert(levels[0].price == 150.0);
    assert(levels[0].total_quantity == 150);  // 100 + 50
    assert(levels[1].price == 149.5);
    assert(levels[1].total_quantity == 200);
    assert(levels[2].price == 149.0);
    
    std::cout << "  PASSED" << std::endl;
}

void test_execute_fill() {
    std::cout << "Testing executeFill..." << std::endl;
    
    OrderBook book("AAPL");
    
    // Add resting sell orders
    book.addOrder(Order(1, "AAPL", Side::Sell, OrderType::Limit, 150.0, 100));
    book.addOrder(Order(2, "AAPL", Side::Sell, OrderType::Limit, 150.5, 200));
    book.addOrder(Order(3, "AAPL", Side::Sell, OrderType::Limit, 151.0, 150));
    
    // Execute buy order that matches first two levels
    auto fills = book.executeFill(Side::Buy, 250, 151.0, 100);
    
    assert(fills.size() == 2);  // Matched two orders
    assert(fills[0].price == 150.0);
    assert(fills[0].quantity == 100);
    assert(fills[1].price == 150.5);
    assert(fills[1].quantity == 150);
    
    // Check remaining order book state
    auto best_ask = book.getBestAsk();
    assert(best_ask.has_value());
    assert(best_ask->first == 150.5);
    assert(best_ask->second == 50);  // 200 - 150 filled
    
    std::cout << "  PASSED" << std::endl;
}

void test_order_lookup() {
    std::cout << "Testing getOrder..." << std::endl;
    
    OrderBook book("AAPL");
    
    Order order(1, "AAPL", Side::Buy, OrderType::Limit, 150.0, 100);
    book.addOrder(order);
    
    // Lookup existing order
    const Order* found = book.getOrder(1);
    assert(found != nullptr);
    assert(found->id == 1);
    assert(found->price == 150.0);
    
    // Lookup non-existent order
    const Order* not_found = book.getOrder(999);
    assert(not_found == nullptr);
    
    std::cout << "  PASSED" << std::endl;
}

void test_modify_order() {
    std::cout << "Testing modifyOrder..." << std::endl;
    
    OrderBook book("AAPL");
    
    Order order(1, "AAPL", Side::Buy, OrderType::Limit, 150.0, 100);
    book.addOrder(order);
    
    // Modify quantity only
    assert(book.modifyOrder(1, 0, 200));
    
    const Order* modified = book.getOrder(1);
    assert(modified != nullptr);
    assert(modified->quantity == 200);
    assert(modified->price == 150.0);
    
    // Modify price (will re-add order)
    assert(book.modifyOrder(1, 151.0, 0));
    
    auto best_bid = book.getBestBid();
    assert(best_bid->first == 151.0);
    
    std::cout << "  PASSED" << std::endl;
}

void test_mid_price() {
    std::cout << "Testing getMidPrice..." << std::endl;
    
    OrderBook book("AAPL");
    
    // No orders - no mid price
    assert(!book.getMidPrice().has_value());
    
    // Only bid - no mid price
    book.addOrder(Order(1, "AAPL", Side::Buy, OrderType::Limit, 150.0, 100));
    assert(!book.getMidPrice().has_value());
    
    // Both sides - has mid price
    book.addOrder(Order(2, "AAPL", Side::Sell, OrderType::Limit, 152.0, 100));
    
    auto mid = book.getMidPrice();
    assert(mid.has_value());
    assert(mid.value() == 151.0);  // (150 + 152) / 2
    
    std::cout << "  PASSED" << std::endl;
}

int main() {
    std::cout << "\n=== Order Book Tests ===" << std::endl;
    
    test_add_order();
    test_cancel_order();
    test_multiple_price_levels();
    test_execute_fill();
    test_order_lookup();
    test_modify_order();
    test_mid_price();
    
    std::cout << "\n=== All Order Book Tests Passed! ===" << std::endl;
    return 0;
}
