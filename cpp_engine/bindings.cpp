#include <pybind11/pybind11.h>
#include "order_book.hpp" 
namespace py = pybind11;


PYBIND11_MODULE(engine_backend, m) {
    py::class_<MarketState>(m, "MarketState")
        .def(py::init<>())
        .def_readwrite("bestBid", &MarketState::bestBid)
        .def_readwrite("bestAsk", &MarketState::bestAsk)
        .def_readwrite("bestBidQuantity", &MarketState::bestBidQuantity)
        .def_readwrite("bestAskQuantity", &MarketState::bestAskQuantity);
}