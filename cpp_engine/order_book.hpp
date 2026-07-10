#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <map>
#include <vector>
#include <string>
#include <unordered_map>

//  Order Data Layout Struct
struct Order {
    int orderId;    
    int botId;      
    bool isBuy;     
    double price;   
    int quantity;   
};



class OrderBook {
private:
    
    // Bids are sorted greatest-to-least using std::greater so the highest buy offer sits at the top
    std::map<double, std::list<Order>, std::greater<double>> bids;
    
    // Asks are sorted least-to-greatest by default so the cheapest sell offer sits at the top
    std::map<double, std::list<Order>> asks;
    
    // Fast O(1) hash map shortcut pointing directly to an order's location inside queues
    std::unordered_map<int, std::list<Order>::iterator> orderLookup;

public:
    OrderBook(double startingPrice);
    void addOrder(Order order);
    void cancelOrder(int orderId);

};

#endif