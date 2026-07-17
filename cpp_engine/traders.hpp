#ifndef TRADERS_HPP
#define TRADERS_HPP

#include "OrderBook.hpp" 

class Trader {
private:
    int traderID;
    OrderBook& orderBook; 

public:
    virtual ~Trader() = default;

    virtual void makeDecision() = 0;

    int getID() const { return traderID; }

    double getCash() const { 
        return orderBook.getAccountCopy(traderID).balance; 
    }

    bool canAfford(double amount) const { 
        return getCash() >= amount; 
    }
    
    int getPosition() const { 
        return orderBook.getAccountCopy(traderID).stockQuantity; 
    }
};

class MarketMaker : public Trader {
public:
    MarketMaker(int id, OrderBook& ob) : Trader(id, ob) {}
    void makeDecision() override;
};

class MomentumTrader : public Trader {
public:
    MomentumTrader(int id, OrderBook& ob) : Trader(id, ob) {}
    void makeDecision() override;
};

class MeanReversionTrader : public Trader {
public:
    MeanReversionTrader(int id, OrderBook& ob) : Trader(id, ob) {}
    void makeDecision() override;
};

#endif