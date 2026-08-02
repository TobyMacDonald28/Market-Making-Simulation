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

void MomentumTrader::makeDecision() {
    double bestBid = orderBook.getBestBid();
    double bestAsk = orderBook.getBestAsk();
    
    std::string action;
    try {
        {
            py::gil_scoped_acquire gil;

            action = py_bot.attr("make_decision")(bestBid, bestAsk)
                         .cast<std::string>();
        }
    }
    catch (const py::error_already_set& e) {
        std::cerr << "Python error in trader "
                  << traderID << ": "
                  << e.what() << std::endl;
        return;
    }
        
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

void RandomTrader::makeDecision() {
    double bestBid = orderBook.getBestBid();
    double bestAsk = orderBook.getBestAsk();
    if (bestBid <= 0 || bestAsk <= 0 || bestBid >= bestAsk) return; 

    int action       = actionDist(gen);
    double offset    = priceOffsetDist(gen);
    double sizeRatio = sizeDist(gen); 

    double midPrice = (bestBid + bestAsk) / 2.0;

    if (action == 0) { 
        double buyPrice = midPrice + offset;
        
        int maxAffordable = static_cast<int>(getCash() / buyPrice);
        int buyQuantity = static_cast<int>(maxAffordable * sizeRatio);
        
        if (buyQuantity > 0) {
            Order newOrder = {static_cast<int>(orderBook.generateOrderID()), traderID, true, buyPrice, buyQuantity};
            if (prevBidId == -1) orderBook.addOrder(newOrder);
            else orderBook.replaceOrder(prevBidId, newOrder);
            prevBidId = newOrder.orderId;
        }

    } else { 
        double sellPrice = midPrice - offset;
        
        int sellQuantity = static_cast<int>(getPosition() * sizeRatio); 
        
        if (sellQuantity > 0) { 
            Order newOrder = {static_cast<int>(orderBook.generateOrderID()), traderID, false, sellPrice, sellQuantity};
            if (prevAskId == -1) orderBook.addOrder(newOrder);
            else orderBook.replaceOrder(prevAskId, newOrder);
            prevAskId = newOrder.orderId; 
        }
    }
}

void AgenticTrader::makeDecision() {
    MarketState marketState = orderBook.getMarketState(traderID);
    double cashBalance = getCash();
    int currentPosition = getPosition();
    
    std::string action;
    try {
        {
            py::gil_scoped_acquire gil;

            action = py_bot.attr("make_decision")(marketState, cashBalance, currentPosition);
            std::string action = py_signal.attr("decision").cast<std::string>();
            int quantity = py_signal.attr("quantity").cast<int>();
        }
    }
    catch (const py::error_already_set& e) {
        std::cerr << "Python error in trader "
                  << traderID << ": "
                  << e.what() << std::endl;
        return;
    }
        

        if (action == "BUY") {
            if (quantity <= 0) {
                std::cout << "[INFO] AgenticTrader " << traderID << " cannot afford to buy.\n";
                return;
            }
            Order newBuyOrder = {static_cast<int>(orderBook.generateOrderID()), traderID, true, buyPrice, buyQuantity};
            orderBook.replaceOrder(prevBidId, newBuyOrder);
            prevBidId = newBuyOrder.orderId;
        } else if (action == "SELL") {
            if (quantity <= 0) {
                std::cout << "[INFO] AgenticTrader " << traderID << " has no stock to sell.\n";
                return;
            }
            Order newSellOrder = {static_cast<int>(orderBook.generateOrderID()), traderID, false, sellPrice, sellQuantity};
            orderBook.replaceOrder(prevAskId, newSellOrder);
            prevAskId = newSellOrder.orderId;
        }
    
}

string AgenticTraderBot::lastStatus() {
    MarketState marketState = orderBook.getMarketState(traderID);
    double cashBalance = getCash();
    int currentPosition = getPosition();
    string status;
    status += "Bot ID: " + std::to_string(traderID) + "\n";
    status += "Cash Balance: " + std::to_string(cashBalance) + "\n";
    status += "Current Position: " + std::to_string(currentPosition) + "\n";
    status += "Previous Bid: " + std::to_string(getOrder(prevBidId).price) + " (Quantity: " + std::to_string(getOrder(prevBidId).quantity) + ")\n";
    status += "Previous Ask: " + std::to_string(getOrder(prevAskId).price) + " (Quantity: " + std::to_string(getOrder(prevAskId).quantity) + ")\n";

    return status;
}