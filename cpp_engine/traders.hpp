#ifndef TRADERS_HPP
#define TRADERS_HPP
#include <thread>
#include <chrono>
#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "order_book.hpp"
#include <random>
namespace py = pybind11;

PYBIND11_MODULE(engine_backend, m) {
    
    py::class_<MarketState>(m, "MarketState")
        .def(py::init<>())
        .def_readwrite("bestBid", &MarketState::bestBid)
        .def_readwrite("bestAsk", &MarketState::bestAsk)
        .def_readwrite("bestBidQuantity", &MarketState::bestBidQuantity)
        .def_readwrite("bestAskQuantity", &MarketState::bestAskQuantity);
}

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

    virtual void runLoop(std::atomic<bool>& running) {
        while (running.load()) {
            makeDecision();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

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
};

class RandomTrader : public Trader {
private:
    std::mt19937 gen;
    
    std::uniform_int_distribution<> actionDist;
    
    std::uniform_real_distribution<> priceOffsetDist;
    
    std::uniform_real_distribution<> sizeDist; 
    
    int prevOrderId; 

public:
    RandomTrader(int id, OrderBook& ob): Trader(id, ob), gen(std::random_device{}()), actionDist(0, 1), priceOffsetDist(0.1, 5.0), sizeDist(0.1, 10.0), prevOrderId(-1) {};
    void makeDecision() override;
    
};

#endif