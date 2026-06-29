import { Card } from '../types';

/**
 * Client-side poker-type evaluator, ported from the index(1) prototype and
 * adapted to the project's `Card { type, level }` model.
 *
 * Two consumers:
 *  - multiplier detection in `gameReducer` (BOMB / KING_BOMB -> ×2)
 *  - the 提示 (hint) button in `GameView` (cycle legal plays)
 *
 * NOTE: this is advisory client logic. The gateway remains the source of truth
 * — a play suggested here may still come back as `playError`.
 */

export type SellKind =
  | 'single'
  | 'pair'
  | 'triple'
  | 'trio_single'
  | 'trio_pair'
  | 'straight'
  | 'pairs_run'
  | 'airplane'
  | 'airplane_s'
  | 'airplane_p'
  | 'bomb'
  | 'rocket';

export interface Sell {
  kind: SellKind;
  /** max ordinal in the combination (tie-breaker within same kind/len). */
  top: number;
  /** run length in distinct ranks (straights / runs / airplanes); 1 otherwise. */
  len: number;
}

// level: 0..12 = 3,4,...,J,Q,K,A,2 ; 13 = 小王 ; 14 = 大王
export function ordinal(card: Card): number {
  const l = card.level;
  if (l <= 12) return l + 3; // 3=3 ... A=14, 2=15
  if (l === 13) return 16; // 小王
  return 17; // 大王
}

function isConsec(arr: number[]): boolean {
  for (let i = 1; i < arr.length; i++) if (arr[i] - arr[i - 1] !== 1) return false;
  return true;
}

function counts(cards: Card[]): Map<number, number> {
  const m = new Map<number, number>();
  for (const card of cards) {
    const o = ordinal(card);
    m.set(o, (m.get(o) ?? 0) + 1);
  }
  return m;
}

export function evaluate(cards: Card[]): Sell | null {
  const n = cards.length;
  if (!n) return null;
  const m = counts(cards);
  const uniq = [...m.keys()].sort((a, b) => a - b);
  const ct = uniq.map((v) => m.get(v)!);

  if (n === 2 && m.has(16) && m.has(17)) return { kind: 'rocket', top: 17, len: 1 };
  if (n === 1) return { kind: 'single', top: uniq[0], len: 1 };
  if (n === 4 && uniq.length === 1) return { kind: 'bomb', top: uniq[0], len: 1 };
  if (n === 2 && uniq.length === 1) return { kind: 'pair', top: uniq[0], len: 1 };
  if (n === 3 && uniq.length === 1) return { kind: 'triple', top: uniq[0], len: 1 };
  if (n === 4 && uniq.length === 2) {
    const t = uniq.find((v) => m.get(v) === 3);
    if (t !== undefined) return { kind: 'trio_single', top: t, len: 1 };
  }
  if (n === 5 && uniq.length === 2) {
    const t = uniq.find((v) => m.get(v) === 3);
    const p = uniq.find((v) => m.get(v) === 2);
    if (t !== undefined && p !== undefined) return { kind: 'trio_pair', top: t, len: 1 };
  }
  // straight: >=5 distinct consecutive singles, top <= A (14)
  if (n >= 5 && uniq.length === n && uniq[n - 1] <= 14 && isConsec(uniq)) {
    return { kind: 'straight', top: uniq[n - 1], len: n };
  }
  // consecutive pairs (>=3 pairs)
  if (n >= 6 && n % 2 === 0 && ct.every((x) => x === 2) && uniq[uniq.length - 1] <= 14 && isConsec(uniq)) {
    return { kind: 'pairs_run', top: uniq[uniq.length - 1], len: uniq.length };
  }
  const ap = evalAirplane(m, uniq, n);
  if (ap) return ap;
  return null;
}

function evalAirplane(m: Map<number, number>, uniq: number[], n: number): Sell | null {
  const trips = uniq.filter((v) => (m.get(v) ?? 0) >= 3 && v <= 14).sort((a, b) => a - b);
  for (let i = 0; i < trips.length; i++) {
    for (let j = i + 2; j <= trips.length; j++) {
      const seq = trips.slice(i, j);
      if (!isConsec(seq)) break;
      const k = seq.length; // number of consecutive triples (>= 2)
      const body = 3 * k;
      const rem = n - body;
      const leftover = new Map<number, number>();
      for (const v of uniq) leftover.set(v, m.get(v)!);
      for (const v of seq) leftover.set(v, leftover.get(v)! - 3);
      const lvals = uniq.filter((v) => (leftover.get(v) ?? 0) > 0);
      const lcount = lvals.reduce((s, v) => s + (leftover.get(v) ?? 0), 0);
      if (rem === 0 && lcount === 0) return { kind: 'airplane', top: seq[k - 1], len: k };
      if (rem === k && lcount === k) return { kind: 'airplane_s', top: seq[k - 1], len: k };
      if (rem === 2 * k && lcount === 2 * k && lvals.every((v) => (leftover.get(v) ?? 0) === 2)) {
        return { kind: 'airplane_p', top: seq[k - 1], len: k };
      }
    }
  }
  return null;
}

export function beats(a: Sell, b: Sell | null): boolean {
  if (!b) return true;
  if (a.kind === 'rocket') return true;
  if (b.kind === 'rocket') return false;
  if (a.kind === 'bomb' && b.kind !== 'bomb') return true;
  if (a.kind !== 'bomb' && b.kind === 'bomb') return false;
  if (a.kind === 'bomb' && b.kind === 'bomb') return a.top > b.top;
  return a.kind === b.kind && a.len === b.len && a.top > b.top;
}

export function isBomb(s: Sell): boolean {
  return s.kind === 'bomb';
}
export function isKingBomb(s: Sell): boolean {
  return s.kind === 'rocket';
}

/* ---------------- legal-play generation (for the 提示 button) ---------------- */

function group(hand: Card[]): Map<number, Card[]> {
  const g = new Map<number, Card[]>();
  for (const card of hand) {
    const o = ordinal(card);
    if (!g.has(o)) g.set(o, []);
    g.get(o)!.push(card);
  }
  return g;
}

function findRun(g: Map<number, Card[]>, vals: number[], len: number, need: number, minTop: number): Card[] | null {
  const ok = vals.filter((v) => (g.get(v)?.length ?? 0) >= need && v <= 14).sort((a, b) => a - b);
  for (let i = 0; i + len <= ok.length; i++) {
    const win = ok.slice(i, i + len);
    if (isConsec(win) && win[len - 1] > minTop) {
      const out: Card[] = [];
      for (const v of win) out.push(...g.get(v)!.slice(0, need));
      return out;
    }
  }
  return null;
}

function spareSingle(g: Map<number, Card[]>, vals: number[], exclude: number[]): Card[] | null {
  const cand = vals
    .filter((v) => !exclude.includes(v))
    .sort((a, b) => g.get(a)!.length - g.get(b)!.length || a - b);
  return cand.length ? [g.get(cand[0])![0]] : null;
}

function sparePair(g: Map<number, Card[]>, vals: number[], exclude: number[]): Card[] | null {
  const cand = vals
    .filter((v) => !exclude.includes(v) && (g.get(v)?.length ?? 0) >= 2)
    .sort((a, b) => a - b);
  return cand.length ? g.get(cand[0])!.slice(0, 2) : null;
}

function rankOf(s: Sell): number {
  const order: Record<SellKind, number> = {
    single: 0,
    pair: 1,
    triple: 2,
    trio_single: 3,
    trio_pair: 4,
    straight: 5,
    pairs_run: 6,
    airplane: 7,
    airplane_s: 8,
    airplane_p: 9,
    bomb: 90,
    rocket: 99,
  };
  return order[s.kind] * 100 + s.top;
}

/** All plays from `hand` that beat `last` (or a spread of options when leading). */
export function legalPlays(hand: Card[], last: Sell | null): Card[][] {
  const g = group(hand);
  const vals = [...g.keys()].sort((a, b) => a - b);
  const out: Card[][] = [];
  const push = (cs: Card[]) => {
    const e = evaluate(cs);
    if (e && beats(e, last)) out.push(cs);
  };

  if (!last) {
    vals.forEach((v) => push([g.get(v)![0]]));
    vals.forEach((v) => {
      if (g.get(v)!.length >= 2) push(g.get(v)!.slice(0, 2));
    });
    vals.forEach((v) => {
      if (g.get(v)!.length >= 3) push(g.get(v)!.slice(0, 3));
    });
    for (let L = 5; L <= 12; L++) {
      const r = findRun(g, vals, L, 1, -1);
      if (r) push(r);
    }
  } else {
    const t = last.kind;
    if (t === 'single') vals.forEach((v) => { if (v > last.top) push([g.get(v)![0]]); });
    else if (t === 'pair') vals.forEach((v) => { if (v > last.top && g.get(v)!.length >= 2) push(g.get(v)!.slice(0, 2)); });
    else if (t === 'triple') vals.forEach((v) => { if (v > last.top && g.get(v)!.length >= 3) push(g.get(v)!.slice(0, 3)); });
    else if (t === 'trio_single')
      vals.forEach((v) => {
        if (v > last.top && g.get(v)!.length >= 3) {
          const s = spareSingle(g, vals, [v]);
          if (s) push([...g.get(v)!.slice(0, 3), ...s]);
        }
      });
    else if (t === 'trio_pair')
      vals.forEach((v) => {
        if (v > last.top && g.get(v)!.length >= 3) {
          const p = sparePair(g, vals, [v]);
          if (p) push([...g.get(v)!.slice(0, 3), ...p]);
        }
      });
    else if (t === 'straight') {
      const r = findRun(g, vals, last.len, 1, last.top);
      if (r) push(r);
    } else if (t === 'pairs_run') {
      const r = findRun(g, vals, last.len, 2, last.top);
      if (r) push(r);
    }

    // bomb / rocket overrides
    if (t !== 'bomb' && t !== 'rocket') {
      vals.forEach((v) => { if (g.get(v)!.length >= 4) push(g.get(v)!.slice(0, 4)); });
    } else if (t === 'bomb') {
      vals.forEach((v) => { if (g.get(v)!.length >= 4 && v > last.top) push(g.get(v)!.slice(0, 4)); });
    }
    if (g.has(16) && g.has(17)) push([g.get(16)![0], g.get(17)![0]]);
  }

  out.sort((a, b) => rankOf(evaluate(a)!) - rankOf(evaluate(b)!));
  return out;
}
