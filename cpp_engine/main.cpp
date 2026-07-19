#include <iostream>

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include "order_book.hpp" 
#include "traders.hpp"    

int main() {

    py::scoped_interpreter guard{};

    PyEval_SaveThread();   
    
    OrderBook centralExchange(100.0);
    std::atomic<bool> running(true);
    std::vector<std::thread> traderThreads;
    std::vector<std::unique_ptr<Trader>> bots;

    for (int i = 0; i < 3; ++i) {
        bots.push_back(std::make_unique<MarketMaker>(i + 1, centralExchange, 0.05));
    }
    for (int i = 3; i < 6; ++i) {
        bots.push_back(std::make_unique<MomentumTrader>(i + 1, centralExchange));
    }


    for (auto& bot : bots) {
        traderThreads.emplace_back([&bot, &running]() {
            bot->runLoop(std::ref(running));
        });
    }

    while (running) {
        std::string command;
        std::cout << "\n[p] Print Book, [exit] Quit: ";
        std::cin >> command;

        if (command == "p") {
             centralExchange.displayBook();
        } else if (command == "exit") {
            running = false; 
        }
    }

    std::cout << "Shutting down trader threads..." << std::endl;
    for (auto& thread : traderThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    std::cout << "All threads joined. Exiting." << std::endl;
    return 0;
/*

    OrderBook centralExchange(100.0);
    std::vector<std::unique_ptr<Trader>> bots;

    bots.push_back(std::make_unique<MarketMaker>(1, centralExchange, 0.05));
    bots.push_back(std::make_unique<MomentumTrader>(2, centralExchange));

    std::string command;
    while (true) {
        std::cout << "\n[t] Tick, [exit] Quit: ";
        std::cin >> command;

        if (command == "t") {
            for (auto& bot : bots) {
                bot->tick();
            }
            std::cout << "Current Order Book:\n";
            centralExchange.displayBook();
            std::cout << "Simulation ticked." << std::endl;
        } else if (command == "exit") {
            break;
        }
    }
    return 0;
    */

}