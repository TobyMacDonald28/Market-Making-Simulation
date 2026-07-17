#include <iostream>

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include "OrderBook.h" 
#include "Trader.h"    

int main() {
    OrderBook centralExchange;
    std::atomic<bool> running(true);
    std::vector<std::thread> traderThreads;

    for (int i = 0; i < 5; ++i) {
        traderThreads.emplace_back(MarketMakerLoop, std::ref(centralExchange), std::ref(running), i);
    }

    while (running) {
        std::cout << "\n--- Trading Engine Running ---" << std::endl;
        std::cout << "Type 'exit' to shut down: ";
        std::string command;
        std::cin >> command;

        if (command == "exit") {
            running = false; 
        }
    }

    for (auto& t : traderThreads) {
        if (t.joinable()) {
            t.join();
        }
    }

    std::cout << "Engine shut down cleanly." << std::endl;
    return 0;
}