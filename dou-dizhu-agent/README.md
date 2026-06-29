# Dou-Dizhu Agent

A dou-dizhu (斗地主) AI agent that plays as a single player. It exposes one HTTP endpoint, `POST /play`, which receives the current game state and returns a legal play decision.

## Architecture

```
外部游戏项目          Agent 服务 (FastAPI)
     │                      │
     │  POST /play          │
     │  (游戏状态 JSON)      │
     │─────────────────────>│
     │                 ┌────┴────┐
     │                 │ API 层   │  校验输入、序列化输出
     │                 └────┬────┘
     │                 ┌────┴────┐
     │                 │ 规则引擎  │  牌型识别、候选生成、大小比较
     │                 └────┬────┘
     │                 ┌────┴────┐
     │                 │ 策略模块  │  LLM 从候选中决策
     │                 │ (可配置)  │
     │                 └────┬────┘
     │                 ┌────┴────┐
     │                 │ 出牌选择  │  把策略映射到具体手牌
     │                 └────┬────┘
     │  返回出牌 JSON       │
     │<─────────────────────│
```

- **API layer** (`api.py`): FastAPI app with `POST /play`.
- **Rule engine** (`rules.py`): Card pattern recognition, comparison, and candidate generation.
- **Card counter** (`counter.py`): Tracks remaining cards from visible information.
- **Strategy module** (`strategy.py`): LLM-based choice from candidates (default MiniMax via OpenAI-compatible API).
- **Play selector** (`selector.py`): Maps LLM choice to concrete cards with fallback.

## Installation

Requires Python 3.12+ and [uv](https://docs.astral.sh/uv/).

```bash
uv sync
```

## Running

```bash
uv run python -m dou_dizhu_agent
```

The server starts on `http://0.0.0.0:8000` by default.

## API Usage

### Request

```bash
curl -X POST http://localhost:8000/play \
  -H "Content-Type: application/json" \
  -d '{
    "player_id": "p1",
    "hand": ["3", "4", "5", "6", "7", "8", "9"],
    "role": "landlord",
    "is_my_turn": true,
    "last_play": ["3", "4", "5", "6", "7"],
    "last_play_player": "p2",
    "other_players_card_count": {"p2": 16, "p3": 16},
    "bottom_cards": ["3", "4", "5"]
  }'
```

### Response

```json
{
  "action": "play",
  "cards": ["4", "5", "6", "7", "8"]
}
```

When the agent chooses to pass:

```json
{
  "action": "pass",
  "cards": []
}
```

## LLM Configuration

By default, the agent uses an OpenAI-compatible endpoint (e.g. MiniMax). Set one of the following environment variables:

```bash
export LLM_API_KEY="your-api-key"
export LLM_BASE_URL="https://api.minimaxi.chat/v1"  # optional
export LLM_MODEL="minimax-text-01"                   # optional
```

If no API key is configured, the agent falls back to a deterministic strategy that always selects the first candidate.

## Testing

```bash
uv run pytest
```

This runs all unit, integration, and end-to-end scenario tests.

## Project Structure

```
src/dou_dizhu_agent/
├── __init__.py
├── __main__.py      # uvicorn entry point
├── api.py           # FastAPI app
├── models.py        # Pydantic request/response models
├── rules.py         # Rule engine
├── strategy.py      # LLM strategy module
├── selector.py      # Play selector with fallback
├── counter.py       # Card counter
└── utils.py         # Utility functions

tests/
├── test_rules.py    # Rule engine tests
├── test_counter.py  # Card counter tests
├── test_models.py   # Pydantic model tests
├── test_strategy.py # Strategy module tests
├── test_selector.py # Selector tests
├── test_api.py      # API integration tests
└── test_games.py    # End-to-end scenario tests
```

## Design Document

See [docs/superpowers/specs/2026-06-29-dou-dizhu-agent-design.md](docs/superpowers/specs/2026-06-29-dou-dizhu-agent-design.md).
