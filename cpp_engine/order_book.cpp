#include "order_book.hpp"
#include <iostream>

OrderBook::OrderBook(double startingPrice) {
    addOrder({0, 0, true, startingPrice, 0});
}

void OrderBook::internalCancelOrder(int orderId){

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


void OrderBook::addOrder(Order order){

    std::lock_guard<std::mutex> lock(bookMutex);

    if (order.isBuy) {

        while (!asks.empty() && asks.begin()->first <= order.price && order.quantity > 0) {
            double executionPrice = asks.begin()->first;
            
                
            Order& orderIt = asks.begin()->second.front();

            double maxAffordable = (executionPrice > 0) ? (traderAccounts[order.botId].balance / executionPrice) : 0;

            int quantity = std::min({order.quantity, orderIt.quantity, (int)maxAffordable});
            
            if (quantity <= 0) {
                std::cerr << "Insufficient funds for botId " << order.botId << " to execute trade at price " << executionPrice << std::endl;
                return;
            } 
            quantity = std::min({quantity, traderAccounts[orderIt.botId].stockQuantity});

            if (quantity <= 0) {
                std::cerr << "Insufficient Stock for botId " << orderIt.botId << " to execute trade " << std::endl;
                return;
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
            
                
            auto orderIt = bids.begin()->second.front();

            double maxAffordable = (executionPrice > 0) ? (traderAccounts[orderIt.botId].balance / executionPrice) : 0;

            int quantity = std::min({order.quantity, orderIt.quantity, (int)maxAffordable});

            if (quantity <= 0) {
                std::cerr << "Insufficient funds for botId " << orderIt.botId << " to execute trade at price " << executionPrice << std::endl;
                return;
            }

            quantity = std::min({quantity, traderAccounts[order.botId].stockQuantity});
            
            if (quantity <= 0) {
                std::cerr << "Insufficient Stock for botId " << order.botId << " to execute trade " << std::endl;
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