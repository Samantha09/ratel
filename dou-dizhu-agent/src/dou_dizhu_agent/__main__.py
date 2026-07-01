"""Entry point for running the dou-dizhu agent server."""

import logging
import os

import uvicorn

# 让应用层的 INFO 日志(如每回合 raw= 决策)能在 uvicorn 默认配置下显示。
logging.getLogger("dou_dizhu_agent").setLevel(logging.INFO)

if __name__ == "__main__":
    host = os.getenv("HOST", "0.0.0.0")
    port = int(os.getenv("PORT", "8000"))
    uvicorn.run("dou_dizhu_agent.api:app", host=host, port=port, reload=False)
