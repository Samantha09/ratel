# ratel

#### 介绍
命令行斗地主 C++ 实现，并提供 WebSocket + JSON 网关与 React 网页前端。

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

#### 使用说明

1.  landlords-server
    - 服务端代码
2.  landlords-client
    - 客户端代码
3. landlords-common
    - 公共代码
