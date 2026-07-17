#include "traders.hpp"

void MarketMaker::makeDecision() {
    double bestBid = orderBook.getBestBid();
    double bestAsk = orderBook.getBestAsk();
    double midPrice = (bestBid + bestAsk) / 2.0;
    double buyPrice = midPrice - spread / 2.0;
    double sellPrice = midPrice + spread / 2.0;
    double currentBalance = getCash();
    double currentPosition = getPosition();
}

void MarketMaker::runLoop(std::atomic<bool>& running) {
    while (running.load()) {
        makeDecision();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}