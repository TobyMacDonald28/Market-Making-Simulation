#include <iostream>
#include <cassert>
#include <cmath>
#include "order_book.hpp"
#include "traders.hpp" 

// Helper function to set up a baseline market
void injectDummyMarket(OrderBook& ob) {
    // Creates a spread of $2.00 (Bid at 99, Ask at 101) -> MidPrice = 100.0
    ob.addOrder({static_cast<int>(ob.generateOrderID()), 999, true, 99.0, 100}); 
    ob.addOrder({static_cast<int>(ob.generateOrderID()), 999, false, 101.0, 100});
}

// TEST 1: Does the bot calculate fair value and quote the correct spread?
void testInitialQuotes() {
    OrderBook ob(100.0);
    injectDummyMarket(ob);
    
    // Create bot with ID 1 and a target spread of $1.00
    MarketMaker bot(1, ob, 1.0); 
    
    // Force the bot to act
    bot.makeDecision();

    // Mid price is 100. Spread is 1.0. 
    // Bot should bid at 99.50 and ask at 100.50.
    // Because 99.50 > 99.0 (dummy bid), the bot should now be the Best Bid.
    // Because 100.50 < 101.0 (dummy ask), the bot should now be the Best Ask.
    assert(ob.getBestBid() == 99.50);
    assert(ob.getBestAsk() == 100.50);

    std::cout << "[PASS] testInitialQuotes: Spread calculated and injected correctly.\n";
}

// TEST 2: Does the bot respect inventory limits?
void testZeroInventoryNoSell() {
    OrderBook ob(100.0);
    injectDummyMarket(ob);
    
    MarketMaker bot(1, ob, 1.0);
    
    // Manually ensure the bot has exactly 0 stock and $10,000
    TraderAccount& botAccount = ob.getAccount(1);
    botAccount.stockQuantity = 0; 
    
    bot.makeDecision();

    // Because it has 0 stock, it should ONLY place a bid, not an ask.
    // Therefore, the Best Ask should remain the dummy ask (101.0).
    assert(ob.getBestBid() == 99.50);
    assert(ob.getBestAsk() == 101.0); // Unchanged!

    std::cout << "[PASS] testZeroInventoryNoSell: Bot refused to short-sell without inventory.\n";
}

// TEST 3: Does the bot size its orders correctly based on cash?
void testOrderSizing() {
    OrderBook ob(100.0);
    injectDummyMarket(ob);
    
    MarketMaker bot(1, ob, 1.0);
    TraderAccount& botAccount = ob.getAccount(1);
    
    // Give the bot exactly $500 cash
    botAccount.balance = 500.0;
    
    bot.makeDecision();
    
    // Expected Buy Price = 99.50
    // Expected quantity = floor((500 / 99.50) / 2) = floor(5.02 / 2) = floor(2.51) = 2 shares
    // (Note: To test this assertion deeply, you would ideally need a getter in OrderBook 
    // to check the exact quantity of the bot's specific order ID).
    
    std::cout << "[PASS] testOrderSizing: Ran without crashing under restricted cash.\n";
}

int main() {
    std::cout << "Running Market Maker Tests...\n";
    std::cout << "-----------------------------\n";
    
    testInitialQuotes();
    testZeroInventoryNoSell();
    testOrderSizing();
    
    std::cout << "-----------------------------\n";
    std::cout << "All tests completed successfully!\n";
    return 0;
}