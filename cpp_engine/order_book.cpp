#include "OrderBook.hpp"
#include <iostream>

OrderBook::OrderBook(double startingPrice) {
    addOrder({0, 0, true, startingPrice, 0});
}


void OrderBook::addOrder(Order order){
    if (order.isBuy) {

        while (asks.begin()->price <= order.price){
            int executionPrice = asks.begin()->price;
            if (asks.begin()->quantity < order.quantity) {
                int quantity = asks.begin()->quantity;
                executeTradeBalances(order, *asks.begin(), executionPrice, quantity);
                order.quantity -= quantity;
            } else {
                int quantity = order.quantity;
                executeTradeBalances(order, *asks.begin(), executionPrice, quantity);
                break;
            }
        }

        bids[order.price].push_back(order);
        orderLookup[order.orderId] = bids[order.price].end();

    } else {

        while (bids.begin()->price >= order.price){
            int executionPrice = bids.begin()->price;
            if (bids.begin()->quantity < order.quantity) {
                int quantity = bids.begin()->quantity;
                executeTradeBalances(*bids.begin(), order, executionPrice, quantity);
                order.quantity -= quantity;
            } else {
                int quantity = order.quantity;
                executeTradeBalances(*bids.begin(), order, executionPrice, quantity);
                // TODO: Need to free order ?
                break;
            }
        }
        asks[order.price].push_back(order);
        orderLookup[order.orderId] = asks[order.price].end();

    }
}

void OrderBook::cancelOrder(int orderId){
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
    
    traderAccounts[sellerOrder->botID].stockQuantity -= quantity
    traderAccounts[sellerOrder->botID].balance += quantity * executionPrice
    
    traderAccounts[buyerOrder->botID].stockQuantity += quantity
    traderAccounts[buyerOrder->botID].balance -= quantity * executionPrice
    
}