
class MomentumTrader:
    def __init__(self):
        self.lookback_prices = []
        self.window_size = 5

    def make_decision(self, best_bid, best_ask):
        self.lookback_prices.append(best_bid)
        if len(self.lookback_prices) > self.window_size:
            self.lookback_prices.pop(0)
            avg_price = sum(self.lookback_prices) / len(self.lookback_prices)
            if best_bid > avg_price:
                return "BUY"
            else:
                return "SELL"
        
        return "HOLD"
    