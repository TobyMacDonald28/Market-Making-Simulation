#include "traders.hpp"
#include "order_book.hpp"

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
    
    if (buyQuantity > 0) {
        orderBook.replaceOrder(prevBidId, newBuyOrder);
        prevBidId = newBuyOrder.orderId;
    }
    if (sellQuantity > 0) {
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

void MomentumTrader::makeDecision() {
    double bestBid = orderBook.getBestBid();
    double bestAsk = orderBook.getBestAsk();
    

    {
        py::gil_scoped_acquire acquire;
        std::string action = py_bot.attr("make_decision")(bestBid, bestAsk).cast<std::string>();
        py::gil_scoped_release release;
        
        double buyPrice = bestBid - 0.5;
        double sellPrice = bestAsk + 0.5;
        int buyQuantity = static_cast<int>(getCash() / buyPrice) / 2;
        int sellQuantity = getPosition() / 2;

        if (action == "BUY") {
            if (buyQuantity <= 0) {
                std::cout << "[INFO] MomentumTrader " << traderID << " cannot afford to buy.\n";
                return;
            }
            Order newBuyOrder = {static_cast<int>(orderBook.generateOrderID()), traderID, true, buyPrice, buyQuantity};
            orderBook.replaceOrder(prevBidId, newBuyOrder);
            prevBidId = newBuyOrder.orderId;
        } else if (action == "SELL") {
            if (sellQuantity <= 0) {
                std::cout << "[INFO] MomentumTrader " << traderID << " has no stock to sell.\n";
                return;
            }
            Order newSellOrder = {static_cast<int>(orderBook.generateOrderID()), traderID, false, sellPrice, sellQuantity};
            orderBook.replaceOrder(prevAskId, newSellOrder);
            prevAskId = newSellOrder.orderId;
        }
    }
}

void MomentumTrader::runLoop(std::atomic<bool>& running) {
    while (running) {
        makeDecision();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
