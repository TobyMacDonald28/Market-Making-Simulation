#ifndef TRADERS_HPP
#define TRADERS_HPP
#include <thread>
#include <chrono>
#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "order_book.hpp"
namespace py = pybind11;


class Trader {
protected:
    int traderID;
    OrderBook& orderBook; 
    int prevBidId = -1;
    int prevAskId = -1;

public:
    Trader(int id, OrderBook& ob) : traderID(id), orderBook(ob) {}

    virtual ~Trader() = default;

    virtual void makeDecision() = 0;

    virtual void runLoop(std::atomic<bool>& running) = 0;

    void tick() {
        makeDecision();
    }

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
    double spread = 0.5;
    MarketMaker(int id, OrderBook& ob, double spread) : Trader(id, ob), spread(spread) {}
    void makeDecision() override;
    void runLoop(std::atomic<bool>& running) override;
};

class MomentumTrader : public Trader {
    py::object py_bot; 
public:
    MomentumTrader(int id, OrderBook& ob) : Trader(id, ob) {
        py::module_ sys = py::module_::import("sys");
        sys.attr("path").attr("append")("../python_brain");
        py::module_ mod = py::module_::import("traders");        
        py_bot = mod.attr("MomentumTrader")();
    }
    void makeDecision() override;
    void runLoop(std::atomic<bool>& running) override;
};

#endif