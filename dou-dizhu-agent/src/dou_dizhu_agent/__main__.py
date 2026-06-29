"""Entry point for running the dou-dizhu agent server."""

import os

import uvicorn

if __name__ == "__main__":
    host = os.getenv("HOST", "0.0.0.0")
    port = int(os.getenv("PORT", "8000"))
    uvicorn.run("dou_dizhu_agent.api:app", host=host, port=port, reload=False)
