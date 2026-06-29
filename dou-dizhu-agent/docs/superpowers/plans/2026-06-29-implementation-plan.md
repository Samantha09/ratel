# 斗地主陪玩 AI Agent 实施计划

## 项目信息

- **项目名称**：dou-dizhu-agent
- **设计文档**：`docs/superpowers/specs/2026-06-29-dou-dizhu-agent-design.md`
- **工作分支**：`worktree-dou-dizhu-agent`
- **技术栈**：Python 3.12+, uv, FastAPI, Pydantic, LangChain, pytest

## 全局约束

1. **出牌必须合法**：所有返回给外部项目的出牌必须经规则引擎校验为合法牌型，且能管住上家（首家除外）。
2. **LLM 只做策略决策**：LLM 不直接生成原始牌型，只能从候选列表中选择索引或 PASS。
3. **模型可配置**：策略模块必须支持配置不同 LLM 提供商，默认使用 MiniMax（通过 OpenAI 兼容接口）。
4. **错误回退**：LLM 返回非法选择或调用失败时，必须回退到规则引擎默认策略（如 PASS 或出最小能管的牌）。
5. **测试驱动**：每个组件必须附带 pytest 单元测试，核心规则引擎需要充分覆盖边界情况。
6. **代码结构**：源代码位于 `src/dou_dizhu_agent/` 包下，按模块组织；测试位于 `tests/`。
7. **牌表示**：`3, 4, 5, 6, 7, 8, 9, 10, J, Q, K, A, 2, 小王, 大王`。
8. **接口契约**：`POST /play` 的请求和响应字段必须严格匹配设计文档。
9. **无占位逻辑**：不允许使用未实现的 `pass` 或 `TODO` 作为最终交付；每个任务完成后对应功能可运行。
10. **保持现有文件**：初始提交包含 `.gitignore`, `.python-version`, `README.md`, `pyproject.toml`, `uv.lock`, `main.py`, `docs/`，可以修改但不应破坏其基本用途。

## Task 1：项目基础设置

### 目标
搭建项目基础结构，安装依赖，确保 pytest 能正常运行。

### 要求
1. 修改 `pyproject.toml`：
   - 添加描述 `"A dou-dizhu AI agent that plays as one player"`。
   - 添加依赖：`fastapi`, `uvicorn[standard]`, `pydantic>=2.0`, `langchain`, `langchain-openai>=0.1.0`, `pytest`, `httpx`（用于 FastAPI TestClient）。
2. 创建目录结构：
   - `src/dou_dizhu_agent/`
   - `src/dou_dizhu_agent/__init__.py`
   - `src/dou_dizhu_agent/api.py`（FastAPI 应用入口，后续任务填充）
   - `src/dou_dizhu_agent/models.py`（Pydantic 模型，后续任务填充）
   - `src/dou_dizhu_agent/rules.py`（规则引擎，后续任务填充）
   - `src/dou_dizhu_agent/strategy.py`（策略模块，后续任务填充）
   - `src/dou_dizhu_agent/selector.py`（出牌选择器，后续任务填充）
   - `src/dou_dizhu_agent/counter.py`（记牌器，后续任务填充）
   - `src/dou_dizhu_agent/utils.py`（工具函数，后续任务填充）
   - `tests/__init__.py`
   - `tests/conftest.py`（pytest 共享 fixture）
3. 运行 `uv sync`（或等效命令）安装依赖。
4. 运行 `uv run pytest`，确认无报错（此时可能没有测试文件，pytest 应报告 0 个测试通过）。
5. 提交变更。

### 验收标准
- `uv run pytest` 成功退出。
- 目录结构符合上述要求。
- `pyproject.toml` 包含所有必需的依赖。

## Task 2：规则引擎

### 目标
实现斗地主规则引擎，包括牌型识别、大小比较和候选生成。

### 要求
1. 在 `src/dou_dizhu_agent/rules.py` 中实现：
   - 牌的点数和花色常量；牌表示为字符串列表。
   - 有效的点数顺序：`3 < 4 < 5 < 6 < 7 < 8 < 9 < 10 < J < Q < K < A < 2 < 小王 < 大王`。
   - `HandPattern` 类（或等效结构）表示牌型，包含类型、主点数、长度/副牌信息。
   - `parse_pattern(cards: list[str]) -> HandPattern | None`：识别以下牌型：
     - 单张
     - 对子
     - 三张
     - 三带一
     - 三带二
     - 顺子（至少 5 张连续点数）
     - 连对（至少 3 对连续点数）
     - 飞机（至少两个连续三张，可带单张或对子）
     - 炸弹（四张同点数）
     - 王炸（小王 + 大王）
   - `can_beat(a: HandPattern, b: HandPattern) -> bool`：判断 a 是否能管住 b。同牌型按点数比较；炸弹大于所有普通牌型；王炸最大。
   - `generate_candidates(hand: list[str], last_play: list[str] | None) -> list[list[str]]`：
     - 如果 `last_play` 为空或 `None`，返回所有合法的首出候选。
     - 否则，返回所有能管住 `last_play` 的合法出牌。
     - 若无牌可管，返回 `[[]]`（表示 PASS，用空列表表示）。
2. 在 `tests/test_rules.py` 中实现单元测试，覆盖：
   - 各种牌型的识别（边界情况如无效牌型、非法输入）。
   - 大小比较（同牌型、炸弹压制普通牌型、王炸最大）。
   - 候选生成（首家出牌、能管住、不能管住、炸弹压制）。
3. 提交变更。

### 验收标准
- 所有规则引擎单元测试通过。
- 候选生成覆盖设计文档中提及的所有牌型。
- 代码通过 `ruff check`（如已配置）或无明显风格问题。

## Task 3：记牌器

### 目标
实现记牌器，根据可见信息维护剩余牌分布。

### 要求
1. 在 `src/dou_dizhu_agent/counter.py` 中实现 `CardCounter` 类：
   - 构造时接收初始参数：我的手牌、底牌、历史出牌（可选）。
   - 维护 `remaining` 字典，记录每种牌在外部牌堆中的剩余数量。
   - 提供 `update(request: PlayRequest)` 方法，根据请求中的手牌、场上已出牌、历史等更新剩余分布。
   - 提供 `summary()` 方法，返回一个适合放入 LLM prompt 的字符串摘要（例如各点数剩余数量）。
   - 如果 `history` 未提供，仅基于我的手牌、底牌和 `last_play` 统计可见牌；其余牌默认按斗地主标准总数推断（每人 17 张，地主 20 张，底牌 3 张）。
2. 在 `tests/test_counter.py` 中实现单元测试。
3. 提交变更。

### 验收标准
- 单元测试覆盖常见场景（含 history、不含 history、更新后状态正确）。
- `summary()` 输出简洁且包含所有点数。

## Task 4：API 数据模型

### 目标
定义 `POST /play` 的请求和响应 Pydantic 模型。

### 要求
1. 在 `src/dou_dizhu_agent/models.py` 中实现：
   - `PlayRequest`：字段包括 `player_id`, `hand`, `role`, `is_my_turn`, `last_play`, `last_play_player`, `other_players_card_count`, `bottom_cards`, `history`（可选）。
   - `role` 必须是 `"landlord"` 或 `"peasant"`。
   - `hand` 中每张牌必须是合法牌字符串。
   - `last_play` 默认为 `[]`。
   - `other_players_card_count` 为字典，值为非负整数。
   - `PlayResponse`：字段包括 `action`（`"play"` 或 `"pass"`）和 `cards`（列表）。
   - 当 `action="pass"` 时，`cards` 应为空列表。
2. 在 `tests/test_models.py` 中实现模型校验测试（有效输入、无效 role、非法牌、缺少必填字段等）。
3. 提交变更。

### 验收标准
- 所有模型测试通过。
- 模型字段与接口契约完全一致。

## Task 5：LLM 策略模块

### 目标
实现策略模块，构造 prompt 并调用 LLM 选择候选。

### 要求
1. 在 `src/dou_dizhu_agent/strategy.py` 中实现 `StrategyModule` 类：
   - 构造时接收模型配置（例如 `provider`, `api_key`, `model`, `base_url`, `temperature`），默认使用 MiniMax。
   - 提供 `choose(candidates: list[list[str]], request: PlayRequest, card_counter_summary: str) -> int | str` 方法：
     - 构造 prompt，包含我的手牌、角色、场上牌、另外两家剩余牌数、底牌、候选列表、记牌器摘要。
     - 调用 LLM，要求 LLM 返回候选索引（从 0 开始）或 `"pass"`。
     - 解析 LLM 输出，返回整数索引或 `"pass"`。
   - 支持环境变量或构造参数配置 LLM 凭证；如果未配置，方法不应在导入时失败，而是在调用时抛出可捕获异常或返回回退标记。
   - 提供 `MockStrategyModule` 或类似测试替身，便于测试。
2. 在 `tests/test_strategy.py` 中实现单元测试（使用 mock LLM 验证 prompt 构造和选择解析）。
3. 提交变更。

### 验收标准
- 策略模块单元测试通过。
- Prompt 包含所有必要上下文信息。
- LLM 输出解析健壮（能处理引号、多余空格等）。

## Task 6：出牌选择器

### 目标
将 LLM 的策略选择映射为具体手牌，并提供回退策略。

### 要求
1. 在 `src/dou_dizhu_agent/selector.py` 中实现 `PlaySelector` 类：
   - 提供 `select(candidates: list[list[str]], choice: int | str, request: PlayRequest) -> tuple[str, list[str]]` 方法。
   - 如果 `choice` 是合法候选索引，返回 `("play", candidates[choice])`。
   - 如果 `choice` 是 `"pass"` 或候选为空/仅含 `[]`，返回 `("pass", [])`。
   - 如果 `choice` 非法（越界、非预期类型），回退到默认策略：
     - 若有候选可出牌，选择最小能管的牌（首家则出最小合法首出）。
     - 若无牌可管，返回 `("pass", [])`。
   - 默认策略逻辑可委托给规则引擎的辅助函数。
2. 在 `tests/test_selector.py` 中实现单元测试，覆盖正常选择、PASS、非法选择回退等情况。
3. 提交变更。

### 验收标准
- 选择器单元测试通过。
- 非法选择时正确回退，不抛出未处理异常。

## Task 7：FastAPI 应用与 /play 接口

### 目标
将所有组件组装为 FastAPI 应用，暴露 `POST /play` 接口。

### 要求
1. 在 `src/dou_dizhu_agent/api.py` 中实现：
   - 创建 FastAPI app。
   - 实现 `POST /play` 路由：
     - 接收 `PlayRequest`。
     - 验证输入（Pydantic 自动处理）。
     - 更新记牌器。
     - 生成候选出牌。
     - 调用策略模块获取选择。
     - 使用出牌选择器映射为具体牌型。
     - 返回 `PlayResponse`。
   - 输入非法时返回 `422`，错误信息明确。
   - LLM 调用失败/超时时记录日志并回退。
   - 应用启动时可以从环境变量读取 LLM 配置。
2. 在 `src/dou_dizhu_agent/__main__.py` 中实现 `uvicorn` 启动入口。
3. 在 `tests/test_api.py` 中实现集成测试，使用 `TestClient` 和 mock LLM 测试完整流程。
4. 提交变更。

### 验收标准
- FastAPI 应用可启动，`/play` 接口返回正确 JSON。
- 集成测试覆盖合法请求、首家出牌、必须 PASS、LLM 失败回退等场景。

## Task 8：端到端对局测试

### 目标
模拟几局已知牌局，验证 Agent 不犯规则错误。

### 要求
1. 在 `tests/test_games.py` 中实现端到端测试：
   - 构造几个已知手牌和场上状态的序列。
   - 对每个状态调用 `/play`（通过 `TestClient`）。
   - 验证返回的出牌：
     - 是合法牌型。
     - 能管住上家（或首家出牌合法）。
     - 返回的牌确实在请求手牌中。
     - 当必须 PASS 时返回 PASS。
2. 使用 mock LLM 控制策略选择，以便复现和断言具体行为。
3. 提交变更。

### 验收标准
- 端到端测试通过。
- 至少覆盖 3 个不同场景（ landlord 首出、peasant 跟牌、炸弹回退）。

## Task 9：README 与最终整理

### 目标
更新 README，清理项目，确保项目可直接运行。

### 要求
1. 更新 `README.md`：
   - 项目简介和架构图引用。
   - 安装和运行命令（`uv sync`, `uv run python -m dou_dizhu_agent`）。
   - API 使用示例（curl 或 Python）。
   - 测试命令。
2. 如果 `main.py` 与项目入口重复或不再需要，可以删除或重定向到 `__main__.py`。
3. 运行完整测试套件 `uv run pytest`，确保全部通过。
4. 提交变更。

### 验收标准
- README 信息完整、准确。
- 完整测试套件通过。
- 项目可在新环境中按 README 步骤运行。

## 任务依赖

```
Task 1 → Task 2 → Task 3 → Task 5 → Task 6 → Task 7 → Task 8 → Task 9
Task 1 → Task 4 ────────────────────────────────↗
```
