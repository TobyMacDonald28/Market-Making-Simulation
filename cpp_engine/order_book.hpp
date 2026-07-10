#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <map>
#include <vector>
#include <string>
#include <unordered_map>

// Order Data Layout Struct
struct Order {
    int orderId;    
    int botId;      
    bool isBuy;     
    double price;   
    int quantity;   
};

struct TraderAccount {
    int botId;
    double balance = 10000;
    int stockQuantity = 0;
};

class OrderBook {
private:
    
    // Bids are sorted greatest to least using std::greater so the highest buy offer sits at the top
    std::map<double, std::list<Order>, std::greater<double>> bids;
    
    // Asks are sorted least to greatest by default so the cheapest sell offer sits at the top
    std::map<double, std::list<Order>> asks;
    
    // Fast O(1) hash map shortcut pointing directly to an order's location inside queues
    std::unordered_map<int, std::list<Order>::iterator> orderLookup;

    // All trader accounts mapped by botID encapsulating their accounts
    std::unordered_map<int, TraderAccount> traderAccounts;

    // helper function to execute trades 
    void executeTradeBalances(const Order& buyerOrder, const Order& sellerOrder, double executionPrice, int quantity);


public:
    OrderBook(double startingPrice);
    void addOrder(Order order);
    void cancelOrder(int orderId);

};

#endif