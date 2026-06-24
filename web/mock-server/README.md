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
