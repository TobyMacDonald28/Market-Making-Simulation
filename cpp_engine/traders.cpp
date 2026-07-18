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

MomentumTrader::MomentumTrader(int id, OrderBook& ob) : Trader(id, ob) {
    py::module_ mod = py::module_::import("momentum_strategy");
    py_bot = mod.attr("MomentumTrader")();
}

void MomentumTrader::makeDecision() {
    double current_price = orderBook.getBestBid();

    {
        py::gil_scoped_acquire acquire;
        std::string action = py_bot.attr("make_decision")(current_price).cast<std::string>();

        if (action == "BUY") {
            double buyPrice = current_price - 0.5; // Example logic
            int buyQuantity = static_cast<int>(getCash() / buyPrice) / 2;
            Order newBuyOrder = {static_cast<int>(orderBook.generateOrderID()), traderID, true, buyPrice, buyQuantity};
            orderBook.replaceOrder(prevBidId, newBuyOrder);
            prevBidId = newBuyOrder.orderId;
        } else if (action == "SELL") {
            double sellPrice = current_price + 0.5; // Example logic
            int sellQuantity = getPosition() / 2;
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
