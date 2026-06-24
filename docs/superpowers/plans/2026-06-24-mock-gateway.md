# Mock Gateway Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a standalone TypeScript WebSocket server that simulates a full 斗地主 PVE game (you vs 2 bots) and speaks the exact JSON contract from the spec (§6) — so the future React frontend can be developed and demoed end-to-end now, and the later C++ gateway can be swapped in behind the same contract.

**Architecture:** A Node process runs a WebSocket server (`ws`). Each browser connection owns one `Game` (3 seats: 1 human + 2 bots). A pure rules engine (`deck` / `cardtype` / `compare` / `bot`) handles cards; `Game` is a state machine that emits `ServerEvent`s through a callback; `server.ts` emits `connected`/`idSet` on connect, starts a `Game` on `createRoomPve`, wires the `Game` emitter to the WebSocket, and translates inbound `ClientEvent` JSON into `Game` method calls. The contract types in `types.ts` are the single source of truth shared (by copy) with the frontend plan.

**Tech Stack:** TypeScript 5, Node 20+, `ws` (WebSocket), Vitest (tests). No native deps, no Boost.

## Global Constraints

- **JSON contract is the spec of record** (spec §6). Every event/message shape below must match `types.ts` exactly; the later C++ gateway and the frontend must conform to the same shapes.
- **Card model:** `Card = { type: 0|1|2|3|4; level: 0..14 }`. `type`: BLANK=0, DIAMOND=1(♦), CLUB=2(♣), SPADE=3(♠), HEART=4(♥). `level`: 3=0,4=1,5=2,6=3,7=4,8=5,9=6,10=7,J=8,Q=9,K=10,A=11,2=12,SMALL_KING=13(S),BIG_KING=14(X).
- **`SellType` mirrors the C++ enum values** (`landlords-common/enums/SellType.h`): ILLEGAL=0, BOMB=1, KING_BOMB=2, SINGLE=3, DOUBLE=4, THREE=5, THREE_ZONES_SINGLE=6, THREE_ZONES_DOUBLE=7, SINGLE_STRAIGHT=10, DOUBLE_STRAIGHT=11, VOID_SELL=18.
- **Mock rules subset (documented limitation):** supported card types are ROCKET(KING_BOMB), BOMB, SINGLE, DOUBLE, THREE, THREE+1 (THREE_ZONES_SINGLE), THREE+2 (THREE_ZONES_DOUBLE), SINGLE_STRAIGHT(≥5, ranks 3..A), DOUBLE_STRAIGHT(≥3 pairs, ranks 3..A). All other types (planes, 4-with-attachments, 4-straights) are detected as **ILLEGAL** in the mock. Ranks 2 (level 12) and jokers (13/14) never participate in straights.
- **Bots are intentionally simple** (greedy: lead smallest single; follow with smallest beating combo of same type, else use a bomb/rocket if available, else pass). Documented; not a strength target.
- **Determinism:** the default bidding order is fixed `[0,1,2]` (human first) so flows are reproducible; tests may inject an `rng` to vary the deal/order. A redeal cap (3) guarantees termination even if no one bids.
- **No external services.** Single Node process, listens on `127.0.0.1:8787`. Logging to stdout.
- **TDD:** every logic module is test-first. Frequent commits, one logical change per commit.

---

## File Structure

All under `web/mock-server/`:

- `package.json`, `tsconfig.json`, `vitest.config.ts` — tooling.
- `src/types.ts` — JSON contract types (source of truth; copied to frontend in Plan 2).
- `src/deck.ts` — 54-card deck, shuffle, deal into 17/17/17 + 3 bottom.
- `src/cardtype.ts` — `detectSell(cards): Sell | null` (the rules core).
- `src/compare.ts` — `canBeat(candidate, last): boolean`.
- `src/bot.ts` — `botPlay(hand, last): BotAction` (greedy).
- `src/game.ts` — `Game` state machine: dealing, landlord bidding, turn rotation, win; emits `ServerEvent`s via callback.
- `src/server.ts` — WebSocket server: `connected`/`idSet` on connect; starts `Game` on `createRoomPve`; maps inbound `ClientEvent` → `Game` calls; `Game` emit → outbound JSON.
- `src/index.ts` — entrypoint: start server on `127.0.0.1:8787`.
- `test/deck.test.ts`, `test/cardtype.test.ts`, `test/compare.test.ts`, `test/bot.test.ts`, `test/game.test.ts`, `test/server.test.ts`.

---

## Task 1: Project scaffold + sanity test

**Files:**
- Create: `web/mock-server/package.json`
- Create: `web/mock-server/tsconfig.json`
- Create: `web/mock-server/vitest.config.ts`
- Create: `web/mock-server/test/sanity.test.ts`

**Interfaces:**
- Produces: a runnable Vitest setup (`npm test`) and a TS build (`npm run build`). Later tasks add modules under `src/`.

- [ ] **Step 1: Create `package.json`**

```json
{
  "name": "ratel-mock-server",
  "version": "0.1.0",
  "private": true,
  "type": "module",
  "scripts": {
    "build": "tsc",
    "start": "node dist/index.js",
    "dev": "tsx src/index.ts",
    "test": "vitest run"
  },
  "dependencies": {
    "ws": "^8.18.0"
  },
  "devDependencies": {
    "@types/node": "^20.11.0",
    "@types/ws": "^8.5.10",
    "tsx": "^4.7.0",
    "typescript": "^5.4.0",
    "vitest": "^1.4.0"
  }
}
```

- [ ] **Step 2: Create `tsconfig.json`**

```json
{
  "compilerOptions": {
    "target": "ES2022",
    "module": "ESNext",
    "moduleResolution": "Bundler",
    "strict": true,
    "esModuleInterop": true,
    "skipLibCheck": true,
    "outDir": "dist",
    "rootDir": "src",
    "types": ["node"]
  },
  "include": ["src"]
}
```

- [ ] **Step 3: Create `vitest.config.ts`**

```ts
import { defineConfig } from 'vitest/config';

export default defineConfig({
  test: {
    include: ['test/**/*.test.ts'],
    environment: 'node',
  },
});
```

- [ ] **Step 4: Create the sanity test `test/sanity.test.ts`**

```ts
import { describe, it, expect } from 'vitest';

describe('sanity', () => {
  it('runs', () => {
    expect(1 + 1).toBe(2);
  });
});
```

- [ ] **Step 5: Install deps and run the test**

Run: `cd web/mock-server && npm install && npm test`
Expected: `1 passed`.

- [ ] **Step 6: Commit**

```bash
git add web/mock-server
git commit -m "feat(mock): scaffold TypeScript + Vitest project"
```

---

## Task 2: JSON contract types + deck

**Files:**
- Create: `web/mock-server/src/types.ts`
- Create: `web/mock-server/src/deck.ts`
- Create: `web/mock-server/test/deck.test.ts`

**Interfaces:**
- Produces: `Card`, `SellType`, `ClientEvent`, `ServerEvent`, `PlayErrorReason`, `Emitter` (in `types.ts`); `buildDeck()`, `shuffle(arr)`, `deal(deck)` (in `deck.ts`). Consumed by every later task.

- [ ] **Step 1: Write `src/types.ts` (the contract of record)**

```ts
export type PokerType = 0 | 1 | 2 | 3 | 4; // BLANK, DIAMOND, CLUB, SPADE, HEART
export type PokerLevel = number;             // 0..14 (see Global Constraints)
export interface Card { type: PokerType; level: PokerLevel; }

export const SellType = {
  ILLEGAL: 0, BOMB: 1, KING_BOMB: 2, SINGLE: 3, DOUBLE: 4, THREE: 5,
  THREE_ZONES_SINGLE: 6, THREE_ZONES_DOUBLE: 7,
  SINGLE_STRAIGHT: 10, DOUBLE_STRAIGHT: 11, VOID_SELL: 18,
} as const;
export type SellTypeValue = (typeof SellType)[keyof typeof SellType];

export type PlayErrorReason = 'mismatch' | 'less' | 'invalid' | 'order' | 'cantPass';

export type ClientEvent =
  | { event: 'setNickname'; data: { nickname: string } }
  | { event: 'createRoomPve'; data: Record<string, never> }
  | { event: 'joinRoom'; data: { roomId: number } }
  | { event: 'landlordElect'; data: { grab: boolean } }
  | { event: 'play'; data: { pokers: Card[] } }
  | { event: 'pass'; data: Record<string, never> }
  | { event: 'exit'; data: Record<string, never> };

export type ServerEvent =
  | { event: 'idSet'; data: { clientId: number } }
  | { event: 'connected'; data: Record<string, never> }
  | { event: 'roomCreate'; data: { roomId: number; owner: string; clientCount: number; type: number } }
  | { event: 'roomJoin'; data: { roomId: number; owner: string; clientCount: number; type: number } }
  | { event: 'roomJoinFail'; data: { reason: 'full' | 'inexist' } }
  | { event: 'gameStarting'; data: Record<string, never> }
  | { event: 'landlordElect'; data: { client: number; nickname: string } }
  | { event: 'landlordConfirm'; data: { landlord: number; nickname: string } }
  | { event: 'showPokers'; data: { pokers: Card[] } }
  | { event: 'playTurn'; data: Record<string, never> }
  | { event: 'playRedirect'; data: { sellClient: number; sellNickname: string; sellPokers: Card[]; sellType: number; nextClient: number } }
  | { event: 'playPass'; data: { client: number; nextClient: number } }
  | { event: 'playError'; data: { reason: PlayErrorReason } }
  | { event: 'gameOver'; data: { winner: number; landlord: number; nickname: string } };

export type Emitter = (e: ServerEvent) => void;
```

- [ ] **Step 2: Write the failing test `test/deck.test.ts`**

```ts
import { describe, it, expect } from 'vitest';
import { buildDeck, shuffle, deal } from '../src/deck.js';

describe('deck', () => {
  it('builds a 54-card deck', () => {
    expect(buildDeck().length).toBe(54);
  });

  it('has 4 of each rank 3..2 and two jokers', () => {
    const d = buildDeck();
    for (let lvl = 0; lvl <= 12; lvl++) {
      expect(d.filter((c) => c.level === lvl).length).toBe(4);
    }
    expect(d.filter((c) => c.level === 13).length).toBe(1); // small joker
    expect(d.filter((c) => c.level === 14).length).toBe(1); // big joker
  });

  it('shuffle preserves contents and length', () => {
    const d = buildDeck();
    const s = shuffle(d);
    expect(s.length).toBe(54);
    expect([...s].sort((a, b) => a.level - b.level || a.type - b.type))
      .toEqual([...d].sort((a, b) => a.level - b.level || a.type - b.type));
  });

  it('deal gives three 17-card hands and a 3-card bottom', () => {
    const { hands, bottom } = deal(buildDeck());
    expect(hands.length).toBe(3);
    expect(hands.every((h) => h.length === 17)).toBe(true);
    expect(bottom.length).toBe(3);
  });
});
```

- [ ] **Step 3: Run the test to verify it fails**

Run: `cd web/mock-server && npx vitest run test/deck.test.ts`
Expected: FAIL — cannot find module `../src/deck.js`.

- [ ] **Step 4: Implement `src/deck.ts`**

```ts
import { Card } from './types.js';

export function buildDeck(): Card[] {
  const cards: Card[] = [];
  for (let level = 0; level <= 12; level++) {
    for (let type = 1; type <= 4; type++) {
      cards.push({ type: type as Card['type'], level });
    }
  }
  cards.push({ type: 0, level: 13 }); // small joker
  cards.push({ type: 0, level: 14 }); // big joker
  return cards;
}

export function shuffle<T>(arr: T[], rng: () => number = Math.random): T[] {
  const a = [...arr];
  for (let i = a.length - 1; i > 0; i--) {
    const j = Math.floor(rng() * (i + 1));
    [a[i], a[j]] = [a[j], a[i]];
  }
  return a;
}

export function deal(deck: Card[], rng: () => number = Math.random): { hands: Card[][]; bottom: Card[] } {
  const shuffled = shuffle(deck, rng);
  const hands: Card[][] = [[], [], []];
  for (let i = 0; i < 51; i++) hands[i % 3].push(shuffled[i]);
  const bottom = shuffled.slice(51, 54);
  const sortFn = (a: Card, b: Card) => a.level - b.level || a.type - b.type;
  hands.forEach((h) => h.sort(sortFn));
  bottom.sort(sortFn);
  return { hands, bottom };
}
```

- [ ] **Step 5: Run the test to verify it passes**

Run: `cd web/mock-server && npx vitest run test/deck.test.ts`
Expected: `4 passed`.

- [ ] **Step 6: Commit**

```bash
git add web/mock-server/src/types.ts web/mock-server/src/deck.ts web/mock-server/test/deck.test.ts
git commit -m "feat(mock): JSON contract types and deck/deal"
```

---

## Task 3: Card-type detection (`detectSell`)

**Files:**
- Create: `web/mock-server/src/cardtype.ts`
- Create: `web/mock-server/test/cardtype.test.ts`

**Interfaces:**
- Consumes: `Card`, `SellType` from `types.ts`.
- Produces: `Sell { type: SellTypeValue; coreLevel: number; length: number; cards: Card[] }` and `detectSell(cards: Card[]): Sell | null`. Consumed by `compare.ts`, `bot.ts`, `game.ts`.

- [ ] **Step 1: Write the failing test `test/cardtype.test.ts`**

```ts
import { describe, it, expect } from 'vitest';
import { detectSell } from '../src/cardtype.js';
import { SellType, Card } from '../src/types.js';

const c = (level: number, type = 1): Card => ({ type: type as Card['type'], level });

describe('detectSell', () => {
  it('rejects empty', () => {
    expect(detectSell([])).toBeNull();
  });

  it('detects single', () => {
    const s = detectSell([c(3)])!;
    expect(s.type).toBe(SellType.SINGLE);
    expect(s.coreLevel).toBe(3);
  });

  it('detects pair', () => {
    const s = detectSell([c(5, 1), c(5, 2)])!;
    expect(s.type).toBe(SellType.DOUBLE);
    expect(s.coreLevel).toBe(5);
  });

  it('detects triple', () => {
    const s = detectSell([c(7, 1), c(7, 2), c(7, 3)])!;
    expect(s.type).toBe(SellType.THREE);
    expect(s.coreLevel).toBe(7);
  });

  it('detects bomb (4 same)', () => {
    const s = detectSell([c(9, 1), c(9, 2), c(9, 3), c(9, 4)])!;
    expect(s.type).toBe(SellType.BOMB);
    expect(s.coreLevel).toBe(9);
  });

  it('detects rocket (both jokers)', () => {
    const s = detectSell([c(13, 0), c(14, 0)])!;
    expect(s.type).toBe(SellType.KING_BOMB);
  });

  it('detects triple + single', () => {
    const s = detectSell([c(6, 1), c(6, 2), c(6, 3), c(11, 1)])!;
    expect(s.type).toBe(SellType.THREE_ZONES_SINGLE);
    expect(s.coreLevel).toBe(6);
  });

  it('detects triple + pair', () => {
    const s = detectSell([c(6, 1), c(6, 2), c(6, 3), c(8, 1), c(8, 2)])!;
    expect(s.type).toBe(SellType.THREE_ZONES_DOUBLE);
    expect(s.coreLevel).toBe(6);
  });

  it('detects single straight (len 5, 3..7)', () => {
    const s = detectSell([c(0), c(1), c(2), c(3), c(4)])!;
    expect(s.type).toBe(SellType.SINGLE_STRAIGHT);
    expect(s.coreLevel).toBe(0);
    expect(s.length).toBe(5);
  });

  it('rejects 4-card straight (min is 5)', () => {
    expect(detectSell([c(0), c(1), c(2), c(3)])).toBeNull();
  });

  it('rejects straight containing a 2 (level 12) or joker', () => {
    expect(detectSell([c(9), c(10), c(11), c(12), c(13)])).toBeNull();
  });

  it('detects pair straight (3 pairs, 3..5)', () => {
    const s = detectSell([c(0, 1), c(0, 2), c(1, 1), c(1, 2), c(2, 1), c(2, 2)])!;
    expect(s.type).toBe(SellType.DOUBLE_STRAIGHT);
    expect(s.length).toBe(3);
  });

  it('returns null for illegal mixes', () => {
    expect(detectSell([c(0), c(2)])).toBeNull(); // two different singles
    expect(detectSell([c(0), c(1), c(3)])).toBeNull(); // not consecutive
  });
});
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `cd web/mock-server && npx vitest run test/cardtype.test.ts`
Expected: FAIL — cannot find module `../src/cardtype.js`.

- [ ] **Step 3: Implement `src/cardtype.ts`**

```ts
import { Card, SellType, SellTypeValue } from './types.js';

export interface Sell {
  type: SellTypeValue;
  coreLevel: number; // comparison key (level of the main body)
  length: number;    // number of cards for non-straights; unit count for straights
  cards: Card[];
}

function countByLevel(cards: Card[]): Map<number, number> {
  const m = new Map<number, number>();
  for (const card of cards) m.set(card.level, (m.get(card.level) ?? 0) + 1);
  return m;
}

function isConsecutive(levels: number[]): boolean {
  for (let i = 1; i < levels.length; i++) {
    if (levels[i] !== levels[i - 1] + 1) return false;
  }
  return true;
}

export function detectSell(cards: Card[]): Sell | null {
  const n = cards.length;
  if (n === 0) return null;

  const counts = countByLevel(cards);
  const distinct = [...counts.keys()].sort((a, b) => a - b);
  const levels = cards.map((c) => c.level).sort((a, b) => a - b);
  const mk = (type: SellTypeValue, coreLevel: number, length: number): Sell => ({ type, coreLevel, length, cards });

  // Rocket
  if (n === 2 && counts.has(13) && counts.has(14)) return mk(SellType.KING_BOMB, 100, 2);
  // Bomb
  if (n === 4 && distinct.length === 1) return mk(SellType.BOMB, distinct[0], 4);
  // Single
  if (n === 1) return mk(SellType.SINGLE, levels[0], 1);
  // Pair
  if (n === 2 && distinct.length === 1) return mk(SellType.DOUBLE, distinct[0], 2);
  // Triple
  if (n === 3 && distinct.length === 1) return mk(SellType.THREE, distinct[0], 3);
  // Triple + single
  if (n === 4 && distinct.length === 2) {
    const triple = [...counts.entries()].find(([, cnt]) => cnt === 3);
    if (triple) return mk(SellType.THREE_ZONES_SINGLE, triple[0], 4);
  }
  // Triple + pair
  if (n === 5 && distinct.length === 2) {
    const vals = [...counts.values()];
    if (vals.includes(3) && vals.includes(2)) {
      const triple = [...counts.entries()].find(([, cnt]) => cnt === 3)!;
      return mk(SellType.THREE_ZONES_DOUBLE, triple[0], 5);
    }
  }
  // Single straight: >=5 distinct consecutive, all <= 11 (A)
  if (n >= 5 && distinct.length === n && distinct.every((l) => l <= 11) && isConsecutive(distinct)) {
    return mk(SellType.SINGLE_STRAIGHT, distinct[0], n);
  }
  // Pair straight: >=3 pairs (>=6 cards), all counts 2, consecutive, all <= 11
  if (n >= 6 && n % 2 === 0 && [...counts.values()].every((cnt) => cnt === 2)
      && distinct.every((l) => l <= 11) && isConsecutive(distinct)) {
    return mk(SellType.DOUBLE_STRAIGHT, distinct[0], distinct.length);
  }
  return null;
}
```

- [ ] **Step 4: Run the test to verify it passes**

Run: `cd web/mock-server && npx vitest run test/cardtype.test.ts`
Expected: all `cardtype` tests pass.

- [ ] **Step 5: Commit**

```bash
git add web/mock-server/src/cardtype.ts web/mock-server/test/cardtype.test.ts
git commit -m "feat(mock): card-type detection (rules core)"
```

---

## Task 4: Comparison (`canBeat`)

**Files:**
- Create: `web/mock-server/src/compare.ts`
- Create: `web/mock-server/test/compare.test.ts`

**Interfaces:**
- Consumes: `Sell`, `SellType` (`cardtype.ts`, `types.ts`).
- Produces: `canBeat(candidate: Sell, last: Sell): boolean`. Consumed by `bot.ts`, `game.ts`.

- [ ] **Step 1: Write the failing test `test/compare.test.ts`**

```ts
import { describe, it, expect } from 'vitest';
import { detectSell } from '../src/cardtype.js';
import { canBeat } from '../src/compare.js';

const sell = (cards: [number, number][]) =>
  detectSell(cards.map(([l, t]) => ({ type: t as 0 | 1 | 2 | 3 | 4, level: l })))!;

describe('canBeat', () => {
  it('higher single beats lower single', () => {
    expect(canBeat(sell([[5, 1]]), sell([[3, 1]]))).toBe(true);
    expect(canBeat(sell([[3, 1]]), sell([[5, 1]]))).toBe(false);
  });

  it('requires same type + length for normal plays', () => {
    expect(canBeat(sell([[5, 1], [5, 2]]), sell([[3, 1]]))).toBe(false); // pair vs single
    expect(canBeat(sell([[5, 1]]), sell([[3, 1], [3, 2]]))).toBe(false); // single vs pair
  });

  it('straight must match length', () => {
    const s5 = sell([[0, 1], [1, 1], [2, 1], [3, 1], [4, 1]]);
    const s5b = sell([[1, 1], [2, 1], [3, 1], [4, 1], [5, 1]]);
    expect(canBeat(s5b, s5)).toBe(true);
  });

  it('bomb beats any non-bomb', () => {
    expect(canBeat(sell([[7, 1], [7, 2], [7, 3], [7, 4]]), sell([[11, 1]]))).toBe(true);
  });

  it('higher bomb beats lower bomb', () => {
    const low = sell([[3, 1], [3, 2], [3, 3], [3, 4]]);
    const high = sell([[9, 1], [9, 2], [9, 3], [9, 4]]);
    expect(canBeat(high, low)).toBe(true);
    expect(canBeat(low, high)).toBe(false);
  });

  it('rocket beats everything, nothing beats rocket', () => {
    const rocket = sell([[13, 0], [14, 0]]);
    const bomb = sell([[12, 1], [12, 2], [12, 3], [12, 4]]);
    expect(canBeat(rocket, bomb)).toBe(true);
    expect(canBeat(bomb, rocket)).toBe(false);
  });
});
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `cd web/mock-server && npx vitest run test/compare.test.ts`
Expected: FAIL — cannot find module `../src/compare.js`.

- [ ] **Step 3: Implement `src/compare.ts`**

```ts
import { Sell } from './cardtype.js';
import { SellType } from './types.js';

export function canBeat(candidate: Sell, last: Sell): boolean {
  if (candidate.type === SellType.KING_BOMB) return true;
  if (last.type === SellType.KING_BOMB) return false;
  if (candidate.type === SellType.BOMB && last.type !== SellType.BOMB) return true;
  if (last.type === SellType.BOMB && candidate.type !== SellType.BOMB) return false;
  if (candidate.type === SellType.BOMB && last.type === SellType.BOMB) {
    return candidate.coreLevel > last.coreLevel;
  }
  if (candidate.type === last.type && candidate.length === last.length) {
    return candidate.coreLevel > last.coreLevel;
  }
  return false;
}
```

- [ ] **Step 4: Run the test to verify it passes**

Run: `cd web/mock-server && npx vitest run test/compare.test.ts`
Expected: all `compare` tests pass.

- [ ] **Step 5: Commit**

```bash
git add web/mock-server/src/compare.ts web/mock-server/test/compare.test.ts
git commit -m "feat(mock): play comparison rules"
```

---

## Task 5: Greedy bot AI

**Files:**
- Create: `web/mock-server/src/bot.ts`
- Create: `web/mock-server/test/bot.test.ts`

**Interfaces:**
- Consumes: `Card`, `SellType` from `types.ts`; `Sell`, `detectSell` from `cardtype.ts`; `canBeat` from `compare.ts`.
- Produces: `BotAction = { kind: 'play'; cards: Card[] } | { kind: 'pass' }` and `botPlay(hand, last): BotAction`. Consumed by `game.ts`.

- [ ] **Step 1: Write the failing test `test/bot.test.ts`**

```ts
import { describe, it, expect } from 'vitest';
import { botPlay } from '../src/bot.js';
import { detectSell } from '../src/cardtype.js';
import { Card } from '../src/types.js';

const c = (level: number, type = 1): Card => ({ type: type as Card['type'], level });

describe('botPlay', () => {
  it('leads with the smallest single when no last play', () => {
    const hand = [c(9), c(4), c(7)];
    const action = botPlay(hand, null);
    expect(action.kind).toBe('play');
    if (action.kind === 'play') expect(action.cards[0].level).toBe(4);
  });

  it('beats a single with the smallest higher single', () => {
    const hand = [c(5), c(8), c(11)];
    const last = detectSell([c(4)])!;
    const action = botPlay(hand, last);
    expect(action.kind).toBe('play');
    if (action.kind === 'play') expect(action.cards[0].level).toBe(5);
  });

  it('passes when it cannot beat', () => {
    const hand = [c(2), c(3)];
    const last = detectSell([c(11)])!; // Ace
    expect(botPlay(hand, last).kind).toBe('pass');
  });

  it('uses a bomb when it cannot beat otherwise', () => {
    const hand = [c(2), c(3), c(7, 1), c(7, 2), c(7, 3), c(7, 4)];
    const last = detectSell([c(11)])!; // Ace single
    const action = botPlay(hand, last);
    expect(action.kind).toBe('play');
    if (action.kind === 'play') expect(action.cards.length).toBe(4);
  });
});
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `cd web/mock-server && npx vitest run test/bot.test.ts`
Expected: FAIL — cannot find module `../src/bot.js`.

- [ ] **Step 3: Implement `src/bot.ts`**

```ts
import { Card, SellType } from './types.js';
import { Sell, detectSell } from './cardtype.js';
import { canBeat } from './compare.js';

export type BotAction = { kind: 'play'; cards: Card[] } | { kind: 'pass' };

function groupByLevel(hand: Card[]): Map<number, Card[]> {
  const m = new Map<number, Card[]>();
  for (const card of hand) {
    if (!m.has(card.level)) m.set(card.level, []);
    m.get(card.level)!.push(card);
  }
  return m;
}

function findSmallestBeat(hand: Card[], last: Sell): Sell | null {
  const grouped = groupByLevel(hand);
  const levelsAsc = [...grouped.keys()].sort((a, b) => a - b);

  switch (last.type) {
    case SellType.SINGLE:
      for (const lvl of levelsAsc) if (lvl > last.coreLevel) return detectSell([grouped.get(lvl)![0]]);
      break;
    case SellType.DOUBLE:
      for (const lvl of levelsAsc) if (lvl > last.coreLevel && grouped.get(lvl)!.length >= 2)
        return detectSell(grouped.get(lvl)!.slice(0, 2));
      break;
    case SellType.THREE:
      for (const lvl of levelsAsc) if (lvl > last.coreLevel && grouped.get(lvl)!.length >= 3)
        return detectSell(grouped.get(lvl)!.slice(0, 3));
      break;
    case SellType.SINGLE_STRAIGHT: {
      const len = last.length;
      for (let start = last.coreLevel + 1; start + len - 1 <= 11; start++) {
        let ok = true;
        for (let i = 0; i < len; i++) if (!grouped.has(start + i)) { ok = false; break; }
        if (ok) {
          const cards: Card[] = [];
          for (let i = 0; i < len; i++) cards.push(grouped.get(start + i)![0]);
          return detectSell(cards);
        }
      }
      break;
    }
    case SellType.DOUBLE_STRAIGHT: {
      const len = last.length; // pair count
      for (let start = last.coreLevel + 1; start + len - 1 <= 11; start++) {
        let ok = true;
        for (let i = 0; i < len; i++) if ((grouped.get(start + i)?.length ?? 0) < 2) { ok = false; break; }
        if (ok) {
          const cards: Card[] = [];
          for (let i = 0; i < len; i++) cards.push(...grouped.get(start + i)!.slice(0, 2));
          return detectSell(cards);
        }
      }
      break;
    }
    default:
      // attachment types (3+1, 3+2) — bot passes on these in the mock
      break;
  }
  return null;
}

function findBomb(hand: Card[]): Sell | null {
  const grouped = groupByLevel(hand);
  for (const [, cards] of grouped) if (cards.length === 4) return detectSell(cards);
  if (grouped.has(13) && grouped.has(14)) return detectSell([grouped.get(13)![0], grouped.get(14)![0]]);
  return null;
}

export function botPlay(hand: Card[], last: Sell | null): BotAction {
  if (hand.length === 0) return { kind: 'pass' };

  if (!last) {
    const sorted = [...hand].sort((a, b) => a.level - b.level || a.type - b.type);
    return { kind: 'play', cards: [sorted[0]] };
  }

  const beat = findSmallestBeat(hand, last);
  if (beat) return { kind: 'play', cards: beat.cards };

  const bomb = findBomb(hand);
  if (bomb && canBeat(bomb, last)) return { kind: 'play', cards: bomb.cards };

  return { kind: 'pass' };
}
```

- [ ] **Step 4: Run the test to verify it passes**

Run: `cd web/mock-server && npx vitest run test/bot.test.ts`
Expected: all `bot` tests pass.

- [ ] **Step 5: Commit**

```bash
git add web/mock-server/src/bot.ts web/mock-server/test/bot.test.ts
git commit -m "feat(mock): greedy bot AI"
```

---

## Task 6: Game state machine (dealing, bidding, turns, win)

**Files:**
- Create: `web/mock-server/src/game.ts`
- Create: `web/mock-server/test/game.test.ts`

**Interfaces:**
- Consumes: `Card`, `ServerEvent`, `Emitter` (`types.ts`); `buildDeck`, `deal` (`deck.ts`); `detectSell`, `Sell` (`cardtype.ts`); `botPlay`, `BotAction` (`bot.ts`).
- Produces: `class Game` with `start()`, `setNickname(name)`, `onHumanLandlordElect(grab)`, `onHumanPlay(cards)`, `onHumanPass()`, `suggestPlay()`, `suggestBid()`, and an `autoHuman` test mode. Emits `ServerEvent`s via the injected `Emitter`. Consumed by `server.ts`.

**Design notes (read before implementing):**
- 3 seats: seat 0 = human, seats 1 & 2 = bots. The human's nickname comes from the constructor.
- `Game` emits only gameplay events (`roomCreate`, `gameStarting`, bidding/play/over). The server emits `connected` + `idSet` on connect (not the `Game`).
- **Bidding:** default order `[0,1,2]` (human first); an injected `rng` shuffles it. Each seat gets one chance per round; first to grab becomes landlord. Bots decide instantly (`botWantsLandlord`: hand has a bomb or a joker). On the human's turn, emit `landlordElect` and stop until `onHumanLandlordElect`. If nobody grabs in a round, redeal and restart; after **3** redeals, force `biddingOrder[0]` as landlord (termination guarantee).
- **After landlord chosen:** landlord takes the 3 bottom cards; emit `landlordConfirm` + `showPokers` (human's full hand); turn starts at landlord with `lastPlay = null`.
- **Turn model (correctness-critical):** `recordPlay`/`recordPass` mutate state and emit but do **not** advance the turn. A single `afterAction(seatId, wasPass)` decides the next turn: two consecutive passes after a play return the turn to the leader (the player who made that play) and clear `lastPlay`; otherwise next is `(seatId + 1) % 3`. You may not pass while leading (`playError: cantPass`).
- **autoHuman (test/demo mode):** when true, the human seat resolves automatically via `botPlay`/`botWantsLandlord` exactly like a bot, so `start()` runs a whole game synchronously to `gameOver` with no external input.
- **Win:** when a seat empties its hand, emit `gameOver { winner, landlord, nickname }` and set phase `'over'`.

- [ ] **Step 1: Write the failing test `test/game.test.ts`**

```ts
import { describe, it, expect } from 'vitest';
import { Game } from '../src/game.js';
import { ServerEvent } from '../src/types.js';

function makeGame(opts: { autoHuman?: boolean; rng?: () => number } = {}) {
  const events: ServerEvent[] = [];
  const game = new Game({ humanNickname: 'san', emit: (e) => events.push(e), ...opts });
  return { game, events };
}

describe('Game', () => {
  it('start emits roomCreate and gameStarting', () => {
    const { game, events } = makeGame();
    game.start();
    const names = events.map((e) => e.event);
    expect(names).toContain('roomCreate');
    expect(names).toContain('gameStarting');
  });

  it('default bidding prompts the human first (landlordElect client 0)', () => {
    const { game, events } = makeGame();
    game.start();
    const elect = events.find((e) => e.event === 'landlordElect') as
      | Extract<ServerEvent, { event: 'landlordElect' }> | undefined;
    expect(elect).toBeDefined();
    expect(elect!.data.client).toBe(0);
  });

  it('grabbing landlord yields landlordConfirm + showPokers', () => {
    const { game, events } = makeGame();
    game.start();
    game.onHumanLandlordElect(true);
    expect(events.some((e) => e.event === 'landlordConfirm')).toBe(true);
    const pok = events.find((e) => e.event === 'showPokers') as
      | Extract<ServerEvent, { event: 'showPokers' }> | undefined;
    expect(pok).toBeDefined();
    expect(pok!.data.pokers.length).toBe(20); // 17 + 3 bottom for the landlord
  });

  it('passing while leading is rejected with cantPass', () => {
    const { game, events } = makeGame();
    game.start();
    game.onHumanLandlordElect(true); // human is landlord, leads, lastPlay=null
    game.onHumanPass();
    const err = events.find((e) => e.event === 'playError') as
      | Extract<ServerEvent, { event: 'playError' }> | undefined;
    expect(err).toBeDefined();
    expect(err!.data.reason).toBe('cantPass');
  });

  it('every autoHuman game terminates at gameOver (deterministic + 20 random deals)', () => {
    // deterministic deal
    let seed = 1;
    const rng = () => {
      seed = (seed * 1103515245 + 12345) & 0x7fffffff;
      return seed / 0x7fffffff;
    };
    for (let i = 0; i < 20; i++) {
      const { game, events } = makeGame({ autoHuman: true, rng });
      game.start();
      expect(events.some((e) => e.event === 'gameOver')).toBe(true);
    }
  });
});
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `cd web/mock-server && npx vitest run test/game.test.ts`
Expected: FAIL — cannot find module `../src/game.js`.

- [ ] **Step 3: Implement `src/game.ts`**

```ts
import { Card, ServerEvent, Emitter } from './types.js';
import { buildDeck, deal } from './deck.js';
import { Sell, detectSell } from './cardtype.js';
import { canBeat } from './compare.js';
import { botPlay, BotAction } from './bot.js';

interface Seat {
  id: number;
  nickname: string;
  isBot: boolean;
  hand: Card[];
}

interface LastPlay {
  sell: Sell;
  sellClientId: number;
  nickname: string;
}

type Phase = 'bidding' | 'playing' | 'over';

export interface GameOpts {
  humanNickname: string;
  emit: Emitter;
  rng?: () => number;
  autoHuman?: boolean;
}

const REDEAL_CAP = 3;

export class Game {
  private emit: Emitter;
  private rng: () => number;
  private autoHuman: boolean;
  private humanNickname: string;
  private seats: Seat[] = [];
  private bottom: Card[] = [];
  private landlord = -1;
  private turn = 0;
  private lastPlay: LastPlay | null = null;
  private consecutivePasses = 0;
  private phase: Phase = 'bidding';
  private biddingOrder: number[] = [0, 1, 2];
  private biddingIndex = 0;
  private redealCount = 0;
  private readonly humanSeat = 0;
  private roomId: number;

  constructor(opts: GameOpts) {
    this.emit = opts.emit;
    this.rng = opts.rng ?? Math.random;
    this.autoHuman = opts.autoHuman ?? false;
    this.humanNickname = opts.humanNickname;
    this.roomId = 1000 + Math.floor(this.rng() * 9000);
  }

  setNickname(name: string): void {
    this.humanNickname = name;
    if (this.seats[0]) this.seats[0] = { ...this.seats[0], nickname: name };
  }

  start(): void {
    this.seats = [
      { id: 0, nickname: this.humanNickname, isBot: false, hand: [] },
      { id: 1, nickname: 'Bot Alpha', isBot: true, hand: [] },
      { id: 2, nickname: 'Bot Beta', isBot: true, hand: [] },
    ];
    this.dealAndBeginBidding();
    this.emit({
      event: 'roomCreate',
      data: { roomId: this.roomId, owner: this.seats[0].nickname, clientCount: 3, type: 1 },
    });
    this.emit({ event: 'gameStarting', data: {} });
    this.advanceBidding();
  }

  private dealAndBeginBidding(): void {
    const { hands, bottom } = deal(buildDeck(), this.rng);
    this.seats.forEach((s, i) => (s.hand = hands[i]));
    this.bottom = bottom;
    this.phase = 'bidding';
    this.biddingOrder = [0, 1, 2]; // fixed order: human (seat 0) bids first — deterministic
    this.biddingIndex = 0;
    this.lastPlay = null;
    this.consecutivePasses = 0;
    this.landlord = -1;
  }

  private advanceBidding(): void {
    if (this.phase === 'over') return;
    while (this.biddingIndex < this.biddingOrder.length) {
      const seatId = this.biddingOrder[this.biddingIndex];
      const seat = this.seats[seatId];
      if (!seat.isBot && !this.autoHuman) {
        this.emit({ event: 'landlordElect', data: { client: seat.id, nickname: seat.nickname } });
        return; // wait for onHumanLandlordElect
      }
      // bot or autoHuman: decide instantly
      if (this.botWantsLandlord(seat)) {
        this.confirmLandlord(seatId);
        return;
      }
      this.biddingIndex++;
    }
    // nobody grabbed this round
    if (this.redealCount >= REDEAL_CAP) {
      this.confirmLandlord(this.biddingOrder[0]); // force someone to keep the game moving
      return;
    }
    this.redealCount++;
    this.dealAndBeginBidding();
    this.advanceBidding();
  }

  private botWantsLandlord(seat: Seat): boolean {
    const hasBomb = seat.hand.some((c) => seat.hand.filter((x) => x.level === c.level).length === 4);
    const hasJoker = seat.hand.some((c) => c.level === 13 || c.level === 14);
    return hasBomb || hasJoker;
  }

  onHumanLandlordElect(grab: boolean): void {
    if (this.phase !== 'bidding') return;
    const seatId = this.biddingOrder[this.biddingIndex];
    if (grab) {
      this.confirmLandlord(seatId);
    } else {
      this.biddingIndex++;
      this.advanceBidding();
    }
  }

  private confirmLandlord(seatId: number): void {
    this.landlord = seatId;
    this.seats[seatId].hand.push(...this.bottom);
    this.seats[seatId].hand.sort((a, b) => a.level - b.level || a.type - b.type);
    this.emit({ event: 'landlordConfirm', data: { landlord: seatId, nickname: this.seats[seatId].nickname } });
    this.emit({ event: 'showPokers', data: { pokers: this.seats[this.humanSeat].hand } });
    this.phase = 'playing';
    this.turn = seatId;
    this.lastPlay = null;
    this.consecutivePasses = 0;
    this.advancePlay();
  }

  private advancePlay(): void {
    if (this.phase === 'over') return;
    const seat = this.seats[this.turn];
    const needsHumanWait = !seat.isBot && !this.autoHuman;
    if (needsHumanWait) {
      this.emit({ event: 'playTurn', data: {} });
      return;
    }
    const wasPass = this.resolveSeat(seat);
    if (this.phase === 'over') return;
    this.turn = this.afterAction(seat.id, wasPass);
    this.advancePlay();
  }

  private resolveSeat(seat: Seat): boolean {
    const action: BotAction = botPlay(seat.hand, this.lastPlay?.sell ?? null);
    if (action.kind === 'pass') {
      this.recordPass(seat);
      return true;
    }
    this.recordPlay(seat, action.cards);
    return false;
  }

  onHumanPlay(cards: Card[]): void {
    if (this.phase !== 'playing' || this.turn !== this.humanSeat) {
      this.emit({ event: 'playError', data: { reason: 'order' } });
      return;
    }
    const seat = this.seats[this.humanSeat];
    const sell = detectSell(cards);
    if (!sell || !this.cardsInHand(seat.hand, cards)) {
      this.emit({ event: 'playError', data: { reason: 'invalid' } });
      return;
    }
    if (this.lastPlay && !canBeat(sell, this.lastPlay.sell)) {
      this.emit({ event: 'playError', data: { reason: 'less' } });
      return;
    }
    this.recordPlay(seat, cards);
    if (this.phase === 'over') return;
    this.turn = this.afterAction(seat.id, false);
    this.advancePlay();
  }

  onHumanPass(): void {
    if (this.phase !== 'playing' || this.turn !== this.humanSeat) {
      this.emit({ event: 'playError', data: { reason: 'order' } });
      return;
    }
    if (!this.lastPlay) {
      this.emit({ event: 'playError', data: { reason: 'cantPass' } });
      return;
    }
    this.recordPass(this.seats[this.humanSeat]);
    if (this.phase === 'over') return;
    this.turn = this.afterAction(seatId(this.humanSeat), true);
    this.advancePlay();
  }

  /** Next-turn decision; clears lastPlay when both followers pass. Does not mutate turn itself. */
  private afterAction(seatId: number, wasPass: boolean): number {
    if (wasPass && this.consecutivePasses >= 2 && this.lastPlay) {
      const leader = this.lastPlay.sellClientId;
      this.lastPlay = null;
      this.consecutivePasses = 0;
      return leader;
    }
    return (seatId + 1) % 3;
  }

  private recordPlay(seat: Seat, cards: Card[]): void {
    const sell = detectSell(cards)!;
    for (const card of cards) {
      const idx = seat.hand.findIndex((h) => h.level === card.level && h.type === card.type);
      if (idx >= 0) seat.hand.splice(idx, 1);
    }
    this.lastPlay = { sell, sellClientId: seat.id, nickname: seat.nickname };
    this.consecutivePasses = 0;
    this.emit({
      event: 'playRedirect',
      data: {
        sellClient: seat.id,
        sellNickname: seat.nickname,
        sellPokers: cards,
        sellType: sell.type,
        nextClient: (seat.id + 1) % 3,
      },
    });
    if (seat.hand.length === 0) this.finishGame(seat.id);
  }

  private recordPass(seat: Seat): void {
    this.consecutivePasses++;
    this.emit({ event: 'playPass', data: { client: seat.id, nextClient: (seat.id + 1) % 3 } });
  }

  private finishGame(winnerId: number): void {
    this.phase = 'over';
    this.emit({
      event: 'gameOver',
      data: { winner: winnerId, landlord: this.landlord, nickname: this.seats[winnerId].nickname },
    });
  }

  private cardsInHand(hand: Card[], cards: Card[]): boolean {
    const pool = hand.map((c) => `${c.level}:${c.type}`);
    for (const card of cards) {
      const idx = pool.indexOf(`${card.level}:${card.type}`);
      if (idx < 0) return false;
      pool.splice(idx, 1);
    }
    return true;
  }

  // ---- idle-resolution hints for the server demo (human as bot) ----
  suggestBid(): boolean {
    return this.botWantsLandlord(this.seats[this.humanSeat]);
  }

  suggestPlay(): BotAction {
    return botPlay(this.seats[this.humanSeat].hand, this.lastPlay?.sell ?? null);
  }
}

function seatId(i: number): number {
  return i;
}
```

> Note the trivial `seatId()` helper keeps `onHumanPass` readable (`afterAction(seatId(0), true)`). If your linter flags it, inline as `this.afterAction(0, true)` — seat 0 is always the human.

- [ ] **Step 4: Run the test to verify it passes**

Run: `cd web/mock-server && npx vitest run test/game.test.ts`
Expected: all `game` tests pass, including the 20-deal `autoHuman` termination test. If termination flakes, check that `afterAction` resets `lastPlay`/`consecutivePasses` exactly on the second consecutive pass and that `REDEAL_CAP` forces a landlord after 3 empty rounds.

- [ ] **Step 5: Commit**

```bash
git add web/mock-server/src/game.ts web/mock-server/test/game.test.ts
git commit -m "feat(mock): game state machine (deal/bid/play/win)"
```

---

## Task 7: WebSocket server wiring

**Files:**
- Create: `web/mock-server/src/server.ts`
- Create: `web/mock-server/src/index.ts`
- Create: `web/mock-server/test/server.test.ts`

**Interfaces:**
- Consumes: `Game` (`game.ts`); `ClientEvent`, `ServerEvent`, `Emitter` (`types.ts`).
- Produces: `startServer(opts?: { port?: number; host?: string; idleMs?: number }): WebSocketServer`. `index.ts` starts it on `127.0.0.1:8787`.

**Design notes:**
- On connect: emit `connected` then `idSet { clientId: 0 }`. Do **not** start the game yet.
- On `setNickname`: store the nickname for the not-yet-created game.
- On `createRoomPve` / `joinRoom`: create a `Game` (with the stored nickname) and call `start()`. (V1 simplified lobby.)
- **Idle auto-resolution:** the server arms a timer (default `idleMs = 1200ms`, env `IDLE_MS` overrides) whenever it emits a human-wait event (`landlordElect`, `playTurn`). If the human hasn't responded when it fires, the server resolves the human's turn using `game.suggestBid()` / `game.suggestPlay()` — so a connected client that does nothing still watches a complete self-playing game (satisfies the demo's "runs to gameOver without input"). Any inbound human message clears the pending timer first.
- Timers are cleared on `gameOver` and on socket close.

- [ ] **Step 1: Write the failing test `test/server.test.ts`**

```ts
import { describe, it, expect, afterEach, beforeAll } from 'vitest';
import { WebSocket } from 'ws';
import { startServer } from '../src/server.js';
import type { WebSocketServer } from 'ws';

// Keep idle auto-resolution from racing the test's active input.
beforeAll(() => {
  process.env.IDLE_MS = '100000';
});

let server: WebSocketServer;
afterEach(() => server?.close());

function connect(port: number): Promise<WebSocket> {
  return new Promise((resolve) => {
    const ws = new WebSocket(`ws://127.0.0.1:${port}`);
    ws.on('open', () => resolve(ws));
  });
}

function recv(ws: WebSocket, predicate: (e: any) => boolean, timeoutMs = 2000): Promise<any> {
  return new Promise((resolve, reject) => {
    const timer = setTimeout(() => reject(new Error('timeout waiting for event')), timeoutMs);
    ws.on('message', (raw) => {
      const e = JSON.parse(raw.toString());
      if (predicate(e)) {
        clearTimeout(timer);
        resolve(e);
      }
    });
  });
}

describe('server', () => {
  it('answers connect with connected + idSet(0)', async () => {
    server = startServer({ port: 0 });
    const port = (server.address() as any).port;
    const ws = await connect(port);
    const id = await recv(ws, (e) => e.event === 'idSet');
    expect(id.data.clientId).toBe(0);
    ws.close();
  });

  it('createRoomPve then landlordElect(grab) yields landlordConfirm + showPokers', async () => {
    server = startServer({ port: 0 });
    const port = (server.address() as any).port;
    const ws = await connect(port);

    ws.send(JSON.stringify({ event: 'setNickname', data: { nickname: 'san' } }));
    ws.send(JSON.stringify({ event: 'createRoomPve', data: {} }));

    // default bidding prompts human (client 0) first
    await recv(ws, (e) => e.event === 'landlordElect' && e.data.client === 0);
    ws.send(JSON.stringify({ event: 'landlordElect', data: { grab: true } }));

    const confirm = await recv(ws, (e) => e.event === 'landlordConfirm');
    expect(confirm.data.landlord).toBe(0);
    const pok = await recv(ws, (e) => e.event === 'showPokers');
    expect(pok.data.pokers.length).toBe(20);
    ws.close();
  });
});
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `cd web/mock-server && npx vitest run test/server.test.ts`
Expected: FAIL — cannot find module `../src/server.js`.

- [ ] **Step 3: Implement `src/server.ts`**

```ts
import { WebSocketServer, WebSocket } from 'ws';
import { Game } from './game.js';
import { ClientEvent, ServerEvent } from './types.js';

export interface ServerOpts {
  port?: number;
  host?: string;
  idleMs?: number;
}

export function startServer(opts: ServerOpts = {}): WebSocketServer {
  const idleMs = opts.idleMs ?? Number(process.env.IDLE_MS ?? 1200);
  const wss = new WebSocketServer({ port: opts.port ?? 0, host: opts.host ?? '127.0.0.1' });

  wss.on('connection', (ws: WebSocket) => {
    let game: Game | null = null;
    let nickname = 'san';
    let idleTimer: NodeJS.Timeout | null = null;

    const send = (e: ServerEvent) => {
      if (ws.readyState === WebSocket.OPEN) ws.send(JSON.stringify(e));
    };

    const clearIdle = () => {
      if (idleTimer) { clearTimeout(idleTimer); idleTimer = null; }
    };

    const armIdle = (onFire: () => void) => {
      clearIdle();
      idleTimer = setTimeout(() => {
        idleTimer = null;
        if (game) onFire();
      }, idleMs);
    };

    const emit: EmitterLike = (e) => {
      send(e);
      if (e.event === 'landlordElect') {
        armIdle(() => game!.onHumanLandlordElect(game!.suggestBid()));
      } else if (e.event === 'playTurn') {
        armIdle(() => {
          const a = game!.suggestPlay();
          if (a.kind === 'play') game!.onHumanPlay(a.cards);
          else game!.onHumanPass();
        });
      } else if (e.event === 'gameOver') {
        clearIdle();
      }
    };

    // connect handshake
    send({ event: 'connected', data: {} });
    send({ event: 'idSet', data: { clientId: 0 } });

    const startGame = () => {
      game = new Game({ humanNickname: nickname, emit });
      game.start();
    };

    ws.on('message', (raw) => {
      let msg: ClientEvent;
      try {
        msg = JSON.parse(raw.toString());
      } catch {
        return;
      }
      if (!game) {
        if (msg.event === 'setNickname') nickname = msg.data.nickname;
        if (msg.event === 'createRoomPve' || msg.event === 'joinRoom') startGame();
        return;
      }
      switch (msg.event) {
        case 'setNickname':
          nickname = msg.data.nickname;
          game.setNickname(nickname);
          break;
        case 'createRoomPve':
        case 'joinRoom':
          // game already started; accept as no-op
          break;
        case 'landlordElect':
          clearIdle();
          game.onHumanLandlordElect(msg.data.grab);
          break;
        case 'play':
          clearIdle();
          game.onHumanPlay(msg.data.pokers);
          break;
        case 'pass':
          clearIdle();
          game.onHumanPass();
          break;
        case 'exit':
          ws.close();
          break;
      }
    });

    ws.on('close', () => clearIdle());
  });

  return wss;
}

type EmitterLike = (e: ServerEvent) => void;
```

- [ ] **Step 4: Create `src/index.ts`**

```ts
import { startServer } from './server.js';

const port = Number(process.env.PORT ?? 8787);
const host = process.env.HOST ?? '127.0.0.1';
const wss = startServer({ port, host });
wss.on('listening', () => {
  console.log(`[ratel-mock] WebSocket server listening on ws://${host}:${port}`);
});
```

- [ ] **Step 5: Run the full suite**

Run: `cd web/mock-server && npm test`
Expected: all suites pass (`sanity`, `deck`, `cardtype`, `compare`, `bot`, `game`, `server`).

- [ ] **Step 6: Commit**

```bash
git add web/mock-server/src/server.ts web/mock-server/src/index.ts web/mock-server/test/server.test.ts
git commit -m "feat(mock): WebSocket server speaking the JSON contract"
```

---

## Task 8: Run instructions + manual smoke check

**Files:**
- Create: `web/mock-server/README.md`

- [ ] **Step 1: Write `README.md`**

````markdown
# ratel mock gateway

A stand-in backend for the ratel web game. Speaks the JSON contract defined in
`src/types.ts` (mirrors `docs/superpowers/specs/2026-06-24-web-game-design.md` §6)
over WebSocket. Simulates a full 斗地主 PVE game (you + 2 bots).

## Run

```bash
npm install
npm run dev      # tsx, listens on ws://127.0.0.1:8787
# or
npm run build && npm start
```

Set `IDLE_MS` (ms) to change how long the server waits for a human move before
auto-resolving it (default 1200). Set `PORT` / `HOST` to change the bind address.

## Test

```bash
npm test
```

## Contract

See `src/types.ts` for `ClientEvent` / `ServerEvent` shapes. Supported card types:
single, pair, triple, triple+1, triple+2, straight (≥5), pair-straight (≥3 pairs),
bomb, rocket. The bot AI is intentionally greedy/simple. A connected client that
sends nothing will watch a complete self-playing game (idle auto-resolution).

## Smoke test with `wscat`

```bash
npx wscat -c ws://127.0.0.1:8787
# on connect you get `connected` + `idSet`; send {"event":"createRoomPve","data":{}}
# to start a game. Send {"event":"landlordElect","data":{"grab":true}} when prompted.
```
````

- [ ] **Step 2: Run the server and confirm it listens**

Run: `cd web/mock-server && npm run dev & ; sleep 2 ; lsof -i :8787 | head ; pkill -f 'tsx src/index.ts'`
Expected: a node process shown listening on `8787`, then terminated.

- [ ] **Step 3: Final full test run**

Run: `cd web/mock-server && npm test`
Expected: all suites green.

- [ ] **Step 4: Commit**

```bash
git add web/mock-server/README.md
git commit -m "docs(mock): run instructions and smoke test"
```

---

## Definition of Done

- `npm test` is green across all 7 suites.
- `npm run dev` serves `ws://127.0.0.1:8787`; a connecting client receives `connected` + `idSet`, and after `createRoomPve` a full game runs to `gameOver` (actively driven, or self-played via idle auto-resolution if the client sends nothing).
- `src/types.ts` is the locked JSON contract for the frontend plan and the future C++ gateway.
- All commits land on the `feature/web-game` branch.
