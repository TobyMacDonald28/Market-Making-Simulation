#include "OrderBook.hpp"
#include <iostream>

OrderBook::OrderBook(double startingPrice) {
    addOrder({0, 0, true, startingPrice, 0});
}


void OrderBook::addOrder(Order order){
    if (order.isBuy) {
        bids[order.price].push_back(order);
        orderLookup[order.orderId] = --bids[order.price].end();
    } else {
        asks[order.price].push_back(order);
        orderLookup[order.orderId] = --asks[order.price].end();
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