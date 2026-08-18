#include "order_book.hpp"
#include <iostream>

OrderBook::OrderBook(double startingPrice) {
    addOrder({0, 0, true, startingPrice, 0});
}

void OrderBook::internalCancelOrder(int orderId){

    auto it = orderLookup.find(orderId);
    if (it != orderLookup.end()) {
        auto orderIt = it->second;
        auto orderprice = orderIt->price;
        if (orderIt->isBuy) {
            bids[orderprice].erase(orderIt);
            if (bids[orderprice].empty()) {
                bids.erase(orderprice);
            }
        } else {
            asks[orderprice].erase(orderIt);
            if (asks[orderprice].empty()) {
                asks.erase(orderprice);
            }
        }
        orderLookup.erase(it);
    }
}


void OrderBook::internalAddOrder(Order order){

    if (order.isBuy) {

        while (!asks.empty() && asks.begin()->first <= order.price && order.quantity > 0) {
            double executionPrice = asks.begin()->first;
            
                
            Order& orderIt = asks.begin()->second.front();

            double maxAffordable = (executionPrice > 0) ? (traderAccounts[order.botId].balance / executionPrice) : 0;

            int quantity = std::min({order.quantity, orderIt.quantity, (int)maxAffordable});
            
            if (quantity <= 0) {
                return;
            } 
            quantity = std::min({quantity, traderAccounts[orderIt.botId].stockQuantity});

            if (quantity <= 0) {
                internalCancelOrder(orderIt.orderId);
                continue;
            } 

            executeTradeBalances(order, asks.begin()->second.front(), executionPrice, quantity);
            
            orderIt.quantity -= quantity;
            order.quantity -= quantity;

            if (orderIt.quantity == 0) {
                internalCancelOrder(orderIt.orderId);
            }
        }
    
        if (order.quantity > 0) {
            bids[order.price].push_back(order);
            orderLookup[order.orderId] = --bids[order.price].end();
        }

    } else {

        while (!bids.empty() && bids.begin()->first >= order.price && order.quantity > 0) {
            double executionPrice = bids.begin()->first;
            
                
            Order& orderIt = bids.begin()->second.front();

            double maxAffordable = (executionPrice > 0) ? (traderAccounts[orderIt.botId].balance / executionPrice) : 0;

            int quantity = std::min({order.quantity, orderIt.quantity, (int)maxAffordable});

            if (quantity <= 0) {
                internalCancelOrder(orderIt.orderId);
                continue;
            }

            quantity = std::min({quantity, traderAccounts[order.botId].stockQuantity});
            
            if (quantity <= 0) {
                return;
            } 

            executeTradeBalances(order, bids.begin()->second.front(), executionPrice, quantity);
            
            orderIt.quantity -= quantity;
            order.quantity -= quantity;

            if (orderIt.quantity == 0) {
                internalCancelOrder(orderIt.orderId);
            }
        
        }

        if (order.quantity > 0) {
            asks[order.price].push_back(order);
            orderLookup[order.orderId] = --asks[order.price].end();
        }
    }
}

void OrderBook::addOrder(Order order){
    std::lock_guard<std::mutex> lock(bookMutex);
    internalAddOrder(order);
}

void OrderBook::cancelOrder(int orderId){
    std::lock_guard<std::mutex> lock(bookMutex);
    internalCancelOrder(orderId);
}

void OrderBook::executeTradeBalances(const Order& buyerOrder, const Order& sellerOrder, double executionPrice, int quantity){

    traderAccounts[sellerOrder.botId].stockQuantity -= quantity;
    traderAccounts[sellerOrder.botId].balance += quantity * executionPrice;
    
    traderAccounts[buyerOrder.botId].stockQuantity += quantity;
    traderAccounts[buyerOrder.botId].balance -= quantity * executionPrice;
    
}

double OrderBook::getBestBid() { 
    std::lock_guard<std::mutex> lock(bookMutex);
    return bids.empty() ? 0.0 : bids.begin()->first; 
}

double OrderBook::getBestAsk() { 
    std::lock_guard<std::mutex> lock(bookMutex);
    return asks.empty() ? 999999.0 : asks.begin()->first; 
}

double OrderBook::getBestBidQuantity() { 
    std::lock_guard<std::mutex> lock(bookMutex);
    return bids.empty() ? 0 : bids.begin()->second.front().quantity; 
}

double OrderBook::getBestAskQuantity() { 
    std::lock_guard<std::mutex> lock(bookMutex);
    return asks.empty() ? 0 : asks.begin()->second.front().quantity; 
}

void OrderBook::replaceOrder(int orderId, Order newOrder) {
    std::lock_guard<std::mutex> lock(bookMutex);
    internalCancelOrder(orderId);
    internalAddOrder(newOrder);
}

void OrderBook::displayBook() {
    std::lock_guard<std::mutex> lock(bookMutex);
    std::cout << "Order Book:\n";
    std::cout << "Bids:\n";
    for (const auto& [price, orders] : bids) {
        for (const auto& order : orders) {
            std::cout << "Price: " << price << ", Quantity: " << order.quantity << ", Bot ID: " << order.botId << "\n";
        }
    }
    std::cout << "Asks:\n";
    for (const auto& [price, orders] : asks) {
        for (const auto& order : orders) {
            std::cout << "Price: " << price << ", Quantity: " << order.quantity << ", Bot ID: " << order.botId << "\n";
        }
    }
}