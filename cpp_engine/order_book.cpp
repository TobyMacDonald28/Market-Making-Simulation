#include "OrderBook.hpp"
#include <iostream>

OrderBook::OrderBook(double startingPrice) {
    addOrder({0, 0, true, startingPrice, 0});
}


void OrderBook::addOrder(Order order){

    std::lock_guard<std::mutex> lock(bookMutex);

    if (order.isBuy) {

        while (!asks.empty() && asks.begin()->first <= order.price){
            int executionPrice = asks.begin()->first;
            if (asks.begin()->second.front().quantity < order.quantity) {

                int quantity = asks.begin()->second.front().quantity;

                executeTradeBalances(order, asks.begin()->second.front(), executionPrice, quantity);
                
                orderLookup.erase(asks.begin()->second.front().orderId);

                asks[executionPrice].erase(asks[executionPrice].begin());
                if (asks[executionPrice].empty()) {
                    asks.erase(executionPrice);
                }

                order.quantity -= quantity;
                
            } else {
                int quantity = order.quantity;
                executeTradeBalances(order, asks.begin()->second.front(), executionPrice, quantity);
                orderLookup.erase(asks.begin()->second.front().orderId);
                return;
            }
        }

        bids[order.price].push_back(order);
        orderLookup[order.orderId] = --bids[order.price].end();

    } else {

        while (!bids.empty() && bids.rbegin()->first >= order.price){
            int executionPrice = bids.rbegin()->first;
            if (bids.rbegin()->second.front().quantity < order.quantity) {

                int quantity = bids.rbegin()->second.front().quantity;

                executeTradeBalances(bids.rbegin()->second.front(), order, executionPrice, quantity);
                
                orderLookup.erase(bids.rbegin()->second.front().orderId);

                bids[executionPrice].erase(bids[executionPrice].begin());
                if (bids[executionPrice].empty()) {
                    bids.erase(executionPrice);
                }

                order.quantity -= quantity;
            } else {
                int quantity = order.quantity;
                executeTradeBalances(bids.begin()->second.front(), order, executionPrice, quantity);
                orderLookup.erase(bids.begin()->second.front().orderId);
                return;
            }
        }

        asks[order.price].push_back(order);
        orderLookup[order.orderId] = --asks[order.price].end();

    }
}

void OrderBook::cancelOrder(int orderId){

    std::lock_guard<std::mutex> lock(bookMutex);

    auto it = orderLookup.find(orderId);
    if (it != orderLookup.end()) {
        auto orderIt = it->second;
        if (orderIt->isBuy) {
            bids[orderIt->price].erase(orderIt);
            if (bids[orderIt->price].empty()) {
                bids.erase(orderIt->price);
            }
        } else {
            asks[orderIt->price].erase(orderIt);
            if (asks[orderIt->price].empty()) {
                asks.erase(orderIt->price);
            }
        }
        orderLookup.erase(it);
    } else {
        std::cerr << "Order ID " << orderId << " not found for cancellation." << std::endl;
    }
}

void OrderBook::executeTradeBalances(const Order& buyerOrder, const Order& sellerOrder, double executionPrice, int quantity){
    
    traderAccounts[sellerOrder.botID].stockQuantity -= quantity;
    traderAccounts[sellerOrder.botID].balance += quantity * executionPrice;
    
    traderAccounts[buyerOrder.botID].stockQuantity += quantity;
    traderAccounts[buyerOrder.botID].balance -= quantity * executionPrice;
    
}