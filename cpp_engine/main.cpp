#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <memory>

#include <pybind11/embed.h>

#include "order_book.hpp"
#include "traders.hpp"

namespace py = pybind11;

int main() {
    py::scoped_interpreter guard{};  // starts embedded Python and holds the GIL on this thread

    OrderBook centralExchange(100.0);
    std::atomic<bool> running(true);
    std::vector<std::thread> traderThreads;
    std::vector<std::unique_ptr<Trader>> bots;

    for (int i = 0; i < 3; ++i) {
        bots.push_back(std::make_unique<MarketMaker>(i + 1, centralExchange, 0.05));
    }
    for (int i = 3; i < 9; ++i) {
        bots.push_back(std::make_unique<MomentumTrader>(i + 1, centralExchange));
    }
    for (int i = 9; i < 40; ++i) {
        bots.push_back(std::make_unique<RandomTrader>(i + 1, centralExchange));
    }


    constexpr int NUM_AGENTIC_BOTS = 1;
    std::vector<AgenticTrader*> agenticBots;  
    for (int i = 40; i < 40 + NUM_AGENTIC_BOTS; ++i) {
        auto bot = std::make_unique<AgenticTrader>(i, centralExchange);
        agenticBots.push_back(bot.get());
        bots.push_back(std::move(bot));
    }

    py::gil_scoped_release release; 

    for (auto& bot : bots) {
        traderThreads.emplace_back([&bot, &running]() {
            bot->runLoop(std::ref(running));
        });
    }

    while (running) {
        std::string command;
        std::cout << "\n[p] Print Book, [a] Agentic bot status, [exit] Quit: ";
        std::cin >> command;

        if (command == "p") {
            centralExchange.displayBook();
        } else if (command == "a") {
            for (auto* bot : agenticBots) {
                std::cout << bot->lastStatus() << std::endl;
            }
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
}