# ratel LLM Agent

外部 LLM Agent，替代 C++ gateway 内部机器人。

## 运行

```bash
cd llm-agent
# 复制环境变量模板并填入 MiniMax API Key
cp .env.example .env.local
# 编辑 .env.local，填入 MINIMAX_API_KEY

# 启动 gateway（仓库根目录）
cd .. && ./gateway 127.0.0.1 8787

# 启动 LLM agents
cd llm-agent && npm run dev
```

## 测试

```bash
# 单元测试（mock provider，不调用真实 API）
npm test

# 端到端冒烟测试（mock provider）
npm run smoke
```

## 环境变量

| 变量 | 说明 | 默认值 |
|---|---|---|
| `GATEWAY_URL` | gateway WebSocket 地址 | `ws://127.0.0.1:8787` |
| `LLM_PROVIDER` | `minimax` / `anthropic` / `openai` / `ollama` / `mock` | `minimax` |
| `MINIMAX_API_KEY` | MiniMax API Key | - |
| `MINIMAX_BASE_URL` | MiniMax Anthropic-compatible 端点 | `https://api.minimaxi.com/anthropic` |
| `MINIMAX_MODEL` | 模型名 | `MiniMax-M2.7` |
| `LLM_TEMPERATURE` | 采样温度 | `0.3` |
| `AGENT_COUNT` | 机器人数量 | `2` |
| `AGENT_NICKNAME_PREFIX` | 机器人昵称前缀 | `robot` |

## 架构

- `src/rules.ts`：斗地主牌型识别、大小比较、合法出牌枚举。
- `src/prompt.ts`：生成给 LLM 的 prompt。
- `src/decision.ts`：调用 LLM API（MiniMax 等）并做合法性兜底。
- `src/game-client.ts`：WebSocket 连接与事件收发。
- `src/agent.ts`：单个 robot 的状态机。
- `src/main.ts`：入口，按配置启动 N 个 agent。
- `scripts/smoke.ts`：端到端冒烟测试。
