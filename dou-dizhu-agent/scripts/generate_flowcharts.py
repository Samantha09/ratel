"""Generate flowchart images using matplotlib."""

from __future__ import annotations

import matplotlib
import matplotlib.patches as mpatches
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch

# Configure Chinese font support.
matplotlib.rcParams["font.family"] = ["Arial Unicode MS"]
matplotlib.rcParams["axes.unicode_minus"] = False


def draw_box(ax, x, y, text, width=2.0, height=0.8, color="#e1f5fe", fontsize=10):
    """Draw a rounded rectangle box."""
    box = FancyBboxPatch(
        (x - width / 2, y - height / 2),
        width,
        height,
        boxstyle="round,pad=0.05,rounding_size=0.15",
        facecolor=color,
        edgecolor="#01579b",
        linewidth=1.5,
    )
    ax.add_patch(box)
    ax.text(x, y, text, ha="center", va="center", fontsize=fontsize, wrap=True)
    return box


def draw_diamond(ax, x, y, text, size=1.0, color="#fff9c4", fontsize=9):
    """Draw a diamond shape for decisions."""
    diamond = plt.Polygon(
        [(x, y + size), (x + size, y), (x, y - size), (x - size, y)],
        facecolor=color,
        edgecolor="#f57f17",
        linewidth=1.5,
    )
    ax.add_patch(diamond)
    ax.text(x, y, text, ha="center", va="center", fontsize=fontsize)
    return diamond


def draw_arrow(ax, x1, y1, x2, y2, label=None, color="#424242"):
    """Draw an arrow between two points."""
    arrow = FancyArrowPatch(
        (x1, y1),
        (x2, y2),
        arrowstyle="->",
        mutation_scale=15,
        linewidth=1.5,
        color=color,
    )
    ax.add_patch(arrow)
    if label:
        mid_x = (x1 + x2) / 2
        mid_y = (y1 + y2) / 2
        ax.text(mid_x + 0.15, mid_y + 0.15, label, fontsize=8, color="#424242")


def create_interface_flowchart(output_path: str) -> None:
    """Create interface data flow chart."""
    fig, ax = plt.subplots(figsize=(14, 8))
    ax.set_xlim(0, 14)
    ax.set_ylim(0, 10)
    ax.axis("off")
    ax.set_title("斗地主 Agent 接口数据流转流程图", fontsize=16, fontweight="bold", pad=20)

    # Nodes arranged horizontally.
    nodes = [
        (1.5, 5, "外部游戏项目"),
        (4.0, 5, "FastAPI\nPOST /play"),
        (6.5, 5, "API 层\nPydantic 校验"),
        (9.0, 7, "规则引擎\n生成候选"),
        (9.0, 3, "记牌器\n更新剩余牌"),
        (11.5, 5, "策略模块\nLLM 决策"),
        (13.0, 5, "出牌选择器\n映射手牌"),
    ]

    for x, y, text in nodes:
        draw_box(ax, x, y, text)

    # Arrows.
    draw_arrow(ax, 2.5, 5, 3.0, 5)
    draw_arrow(ax, 5.0, 5, 5.5, 5)
    draw_arrow(ax, 7.5, 5, 8.0, 6.5, "候选")
    draw_arrow(ax, 7.5, 5, 8.0, 3.5, "状态")
    draw_arrow(ax, 10.0, 6.5, 10.5, 5, "候选列表")
    draw_arrow(ax, 10.0, 3.5, 10.5, 5, "摘要")
    draw_arrow(ax, 12.5, 5, 12.6, 5)

    # Return arrow (curved).
    return_arrow = FancyArrowPatch(
        (13.0, 4.2),
        (1.5, 4.2),
        arrowstyle="->",
        mutation_scale=15,
        linewidth=1.5,
        color="#d32f2f",
        connectionstyle="arc3,rad=-0.3",
    )
    ax.add_patch(return_arrow)
    ax.text(7.25, 3.2, "返回 PlayResponse JSON", ha="center", fontsize=10, color="#d32f2f")

    plt.tight_layout()
    plt.savefig(output_path, dpi=150, bbox_inches="tight", facecolor="white")
    plt.close()


def create_agent_workflow_chart(output_path: str) -> None:
    """Create agent internal workflow chart."""
    fig, ax = plt.subplots(figsize=(12, 14))
    ax.set_xlim(0, 12)
    ax.set_ylim(0, 16)
    ax.axis("off")
    ax.set_title("斗地主 Agent 内部工作流程图", fontsize=16, fontweight="bold", pad=20)

    # Vertical workflow.
    y_positions = [
        (15, "开始", False),
        (13.5, "接收\nPlayRequest", False),
        (12, "Pydantic\n输入校验", True),
        (10.5, "更新记牌器\nCardCounter", False),
        (9, "规则引擎\n生成候选", False),
        (7.5, "是否存在\n合法候选?", True),
        (6, "构造 LLM\nPrompt", False),
        (4.5, "调用 LLM\n获取选择", False),
        (3, "选择是否\n合法?", True),
        (1.5, "构造\nPlayResponse", False),
        (0, "结束", False),
    ]

    boxes = {}
    for i, (y, text, is_diamond) in enumerate(y_positions):
        if is_diamond:
            boxes[i] = draw_diamond(ax, 6, y, text, size=0.9)
        else:
            color = "#c8e6c9" if text == "结束" or text == "开始" else "#e1f5fe"
            boxes[i] = draw_box(ax, 6, y, text, color=color)

    # Main vertical arrows.
    for i in range(len(y_positions) - 1):
        y1 = y_positions[i][0]
        y2 = y_positions[i + 1][0]
        if y_positions[i][2]:  # diamond
            draw_arrow(ax, 6, y1 - 0.9, 6, y2 + 0.9, "是")
        else:
            draw_arrow(ax, 6, y1 - 0.4, 6, y2 + 0.4)

    # Validation failure branch.
    draw_arrow(ax, 6.9, 12, 9.5, 12, "否")
    err_box = draw_box(ax, 10.5, 12, "返回 422", color="#ffccbc")
    draw_arrow(ax, 10.5, 11.6, 6, 0.4, "")

    # No candidate branch.
    draw_arrow(ax, 6.9, 7.5, 9.5, 7.5, "无")
    pass_box = draw_box(ax, 10.5, 7.5, "返回 pass", color="#ffccbc")
    draw_arrow(ax, 10.5, 7.1, 6, 1.4, "")

    # Invalid choice branch.
    draw_arrow(ax, 6.9, 3, 9.5, 3, "否")
    fallback_box = draw_box(ax, 10.5, 3, "回退默认策略", color="#fff9c4")
    draw_arrow(ax, 10.5, 2.6, 6, 1.4, "")

    # LLM exception branch.
    ax.text(2.5, 4.5, "超时/异常", fontsize=9, color="#d32f2f")
    draw_arrow(ax, 4.2, 4.5, 3.5, 3, "")
    ax.text(2.5, 3, "记录日志", fontsize=9, color="#d32f2f")
    draw_arrow(ax, 3.5, 3, 5.1, 3, "")

    plt.tight_layout()
    plt.savefig(output_path, dpi=150, bbox_inches="tight", facecolor="white")
    plt.close()


if __name__ == "__main__":
    base = "/usr/local/data/Projects/dou-dizhu-agent/docs/superpowers/flows/images"
    create_interface_flowchart(f"{base}/interface-flow.png")
    create_agent_workflow_chart(f"{base}/agent-workflow.png")
    print("Flowchart images generated successfully.")
