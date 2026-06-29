# ratel

#### 介绍
命令行斗地主 C++ 实现，并提供 WebSocket + JSON 网关、React 网页前端，以及基于 LLM 的机器人出牌代理。

#### Web 前端预览

基于 React + Vite + Tailwind，连接 C++ 网关实时对战（真实扑克牌面 + Linear 暗色主题）。

| 大厅 | 抢地主 |
| --- | --- |
| ![Lobby](web/docs/screenshots/lobby.png) | ![Bidding](web/docs/screenshots/bidding.png) |

| 出牌 | 结算 |
| --- | --- |
| ![Playing](web/docs/screenshots/playing.png) | ![Result](web/docs/screenshots/result.png) |

运行方式见 [`web/README.md`](web/README.md)：网关 `./gateway 127.0.0.1 8787`，前端 `cd web && npm run dev`。

#### 安装教程

本项目依赖于 muduo 和 boost

1.  先安装 boost 库
2.  make

#### 项目结构

1.  `landlords-server` / `landlords-client` / `landlords-common`
    - C++ 斗地主核心服务端、客户端与公共代码。
2.  `web`
    - React + Vite + Tailwind 网页前端，连接网关实时对战。
3.  `bot-client`
    - 外部 LLM 机器人客户端，替代 C++ 网关内部机器人；负责 WebSocket 通信，并将出牌决策委托给 `dou-dizhu-agent`。
4.  `dou-dizhu-agent`
    - Python FastAPI 出牌代理，提供 `POST /play` 接口：规则引擎 + LLM 策略 + 出牌选择器。

#### 快速运行

- 网关：`./gateway 127.0.0.1 8787`
- 网页前端：`cd web && npm run dev`
- Python 出牌代理：`cd dou-dizhu-agent && uv sync && uv run python -m dou_dizhu_agent`
- LLM 机器人客户端：`cd bot-client && cp .env.example .env.local && npm run dev`

详见各子目录 `README.md`。
