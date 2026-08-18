#include "../cpp_engine/order_book.hpp"
#include <iostream>
#include <cassert>

void test_simple_match() {
    OrderBook ob(100.0);

    ob.getAccount(1).stockQuantity = 100;
    ob.getAccount(2).stockQuantity = 0;

    ob.addOrder({101, 1, false, 100.0, 10});
    ob.addOrder({102, 2, true, 100.0, 10});

    std::cout << "Testing simple match..." << std::endl;
    
    assert(ob.getAccount(2).stockQuantity == 10);
    assert(ob.getAccount(1).balance == 10000.0 + (10 * 100.0)); // 10000 base + 1000 trade
    
    std::cout << "Simple match test passed!" << std::endl;
}

void test_partial_fill() {
    OrderBook ob(100.0);

    ob.getAccount(1).stockQuantity = 100;
    ob.getAccount(2).stockQuantity = 0;

    ob.addOrder({101, 1, false, 100.0, 10});

    ob.addOrder({102, 2, true, 100.0, 5});

    std::cout << "Testing partial fill..." << std::endl;
    assert(ob.getAccount(2).stockQuantity == 5);
    assert(ob.getAccount(1).balance == 10000.0 + (5 * 100.0)); // 10000 base + 500 trade
    
    std::cout << "Partial fill test passed!" << std::endl;
}

void test_insufficient_funds() {
    OrderBook ob(100.0);

    ob.getAccount(1).balance = 10000.0;
    ob.getAccount(1).stockQuantity = 100;
    ob.getAccount(2).balance = 50.0; // Not enough to buy 10 stocks at 100 each
    ob.getAccount(2).stockQuantity = 0;

    ob.addOrder({101, 1, false, 100.0, 10});

    std::cout << "Testing insufficient funds..." << std::endl;
    ob.addOrder({102, 2, true, 100.0, 10}); // Trying to buy 10 stocks

    assert(ob.getAccount(2).stockQuantity == 0); // Should not be able to buy any
    assert(ob.getAccount(1).balance == 10000.0); // No trade should have occurred
    
    std::cout << "Insufficient funds test passed!" << std::endl;
}

void test_insufficient_stock() {
    OrderBook ob(100.0);
    ob.getAccount(1).balance = 10000.0;
    ob.getAccount(1).stockQuantity = 5; // Only 5 stocks available
    ob.getAccount(2).stockQuantity = 0;
    ob.getAccount(2).balance = 500.0; // Enough to buy 5 stocks at 100 each


    ob.addOrder({101, 1, false, 100.0, 10}); // Trying to sell 10 stocks

    ob.addOrder({102, 2, true, 100.0, 10}); // Trying to buy 10 stocks

    std::cout << "Testing insufficient stock..." << std::endl;
    assert(ob.getAccount(2).stockQuantity == 5); // Should only be able to buy 5
    assert(ob.getAccount(1).balance == 10000.0 + (5 * 100.0)); // 10000 base + 500 trade
    
    std::cout << "Insufficient stock test passed!" << std::endl;
}

int main() {
    try {
        test_simple_match();
        test_partial_fill();
        test_insufficient_funds();
        test_insufficient_stock();
        std::cout << "All tests passed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}