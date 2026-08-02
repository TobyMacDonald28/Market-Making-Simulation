from collections import deque
from dotenv import load_dotenv
import os
import pandas as pd
from groq import Groq
from pydantic import BaseModel, ConfigDict, Field
from typing import Literal
import numpy as np
import json
import engine_backend
 
load_dotenv()
 
client = Groq(
    api_key=os.environ["GROQ_API_KEY"]
)
 
 
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
 
 
class Decision(BaseModel):
    model_config = ConfigDict(extra="forbid")
    decision: Literal["BUY", "SELL", "HOLD"]
    quantity: int = Field(ge=0)
 
 
class AgenticTrader:
    def __init__(self):
        self.lookback_window = 10
        self.tick_history = deque(maxlen=self.lookback_window)
 
    def make_decision(self, market_state, current_position, current_cash):
        self.tick_history.append(market_state)
 
        if len(self.tick_history) < self.lookback_window:
            return Decision(decision="HOLD", quantity=0)
 
        df = pd.DataFrame(self.tick_history)
 
        avgbidvol = df['bid_volume'].mean()
        avgaskvol = df['ask_volume'].mean()
 
        stdbid = df['best_bid'].std()
        stdask = df['best_ask'].std()
 
        avgask = df['best_ask'].mean()
        avgbid = df['best_bid'].mean()
 
        current_features = df.iloc[-1:]
 
        current_bid = float(current_features['best_bid'].values[0])
        current_ask = float(current_features['best_ask'].values[0])
 
        context = {
            # numpy scalar types (np.float64 etc.) aren't JSON serializable,
            # so every value here is cast to a plain python float first.
            'avg_bid_volume': float(avgbidvol),
            'avg_ask_volume': float(avgaskvol),
            'std_bid': float(stdbid) if pd.notna(stdbid) else 0.0,
            'std_ask': float(stdask) if pd.notna(stdask) else 0.0,
            'avg_ask': float(avgask),
            'avg_bid': float(avgbid),
            'current_bid': current_bid,
            'current_ask': current_ask,
            'current_bid_volume': float(current_features['bid_volume'].values[0]),
            'current_ask_volume': float(current_features['ask_volume'].values[0]),
            'current_position': current_position,
            'current_cash': current_cash,
        }
 
        decision = self.predict(context)
        return decision
 
    def predict(self, context):
        response = client.chat.completions.create(
            model="openai/gpt-oss-20b",
            messages=[
                {"role": "system", "content": "You are a trading agent that makes decisions based on market data. You will be provided with a context containing market statistics and the current state of the market. Your task is to analyze this information and make a decision to either BUY, SELL, or HOLD. Additionally, you must specify the quantity of the asset to trade, which should be a non-negative integer and MUST be less than or equal to the amount available. Please ensure that your response adheres strictly to the specified JSON schema."},
                {"role": "user", "content": json.dumps(context)},
            ],
            response_format={
                "type": "json_schema",
                "json_schema": {
                    "name": "decision",
                    "schema": Decision.model_json_schema(),
                    "strict": True,
                },
            },
            reasoning_effort="low",
            max_completion_tokens=1024,
            temperature=0.2,
        )
        decision = Decision.model_validate_json(response.choices[0].message.content)
        return decision
 