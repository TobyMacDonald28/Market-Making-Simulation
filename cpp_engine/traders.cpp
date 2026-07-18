#include "traders.hpp"
#include "order_book.hpp"
#include <iostream>

void MarketMaker::makeDecision() {
    double bestBid = orderBook.getBestBid();
    double bestAsk = orderBook.getBestAsk();
    double midPrice = (bestBid + bestAsk) / 2.0;
    double buyPrice = midPrice - spread / 2.0;
    double sellPrice = midPrice + spread / 2.0;
    int buyQuantity = static_cast<int>(getCash() / buyPrice) / 2;
    int sellQuantity = getPosition() / 2;
    Order newBuyOrder = {static_cast<int>(orderBook.generateOrderID()), traderID, true, buyPrice, buyQuantity};
    Order newSellOrder = {static_cast<int>(orderBook.generateOrderID()), traderID, false, sellPrice, sellQuantity};
    
    if (getCash() > 0) {
        orderBook.replaceOrder(prevBidId, newBuyOrder);
        prevBidId = newBuyOrder.orderId;
    }
    if (getPosition() > 0) {
        orderBook.replaceOrder(prevAskId, newSellOrder);
        prevAskId = newSellOrder.orderId;
    }
}

void MarketMaker::runLoop(std::atomic<bool>& running) {
    while (running.load()) {
        makeDecision();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
