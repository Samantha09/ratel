import { useEffect, useMemo, useState } from 'react';
import { Card } from '../types';
import { GameState } from '../state/gameReducer';
import { legalPlays, evaluate } from '../game/pokerEngine';
import { CardTable } from '../game/CardTable';
import { BottomStrip } from '../game/BottomStrip';
import { Hand } from '../game/Hand';
import { ActionBar } from '../game/ActionBar';
import { BiddingOverlay } from '../game/BiddingOverlay';
import { ResultOverlay } from '../game/ResultOverlay';
import { Toast } from '../components/Toast';

export interface GameActions {
  selectCard: (i: number) => void;
  setSelection: (indices: number[]) => void;
  play: () => void;
  pass: () => void;
  grab: (g: boolean) => void;
  reset: () => void;
  playAgain: () => void;
}

export interface GameViewProps {
  state: GameState;
  actions: GameActions;
}

const ERROR_TEXT: Record<string, string> = {
  mismatch: '牌型不匹配',
  less: '管不上，出牌太小',
  invalid: '无效的牌型',
  order: '还没轮到你',
  cantPass: '轮到你出牌，不能不出',
};

/** Map a list of cards to distinct indices in `hand` (consuming each match). */
function indicesFor(hand: Card[], cards: Card[]): number[] {
  const used = new Set<number>();
  const out: number[] = [];
  for (const c of cards) {
    const i = hand.findIndex((h, idx) => !used.has(idx) && h.level === c.level && h.type === c.type);
    if (i >= 0) {
      used.add(i);
      out.push(i);
    }
  }
  return out;
}

function TopBar({
  roomId,
  baseScore,
  multiplier,
  round,
}: {
  roomId: number | null;
  baseScore: number;
  multiplier: number;
  round: number;
}) {
  return (
    <header className="topbar">
      <div className="brand">
        <div className="mark">斗</div>
        <div>
          <h1>斗 地 主</h1>
          <small>DOU DI ZHU</small>
        </div>
      </div>
      <div className="stat">
        <div className={`chip room-chip ${roomId != null ? 'show' : ''}`}>
          🚪 房间 <b>{roomId ?? '—'}</b>
        </div>
        <div className="chip">
          <span className="dot" />底分 <b>{baseScore}</b>
        </div>
        <div className="chip">
          倍数 <b>{multiplier}</b>
        </div>
        <div className="chip">
          第 <b>{round}</b> 局
        </div>
      </div>
    </header>
  );
}

export function GameView({ state, actions }: GameViewProps) {
  const myTurn = state.clientId != null && state.turnClientId === state.clientId;
  const canPass = state.lastSell != null && state.lastSell.client !== state.clientId;
  const myRole = state.myType;

  // 提示按钮:用客户端牌型引擎循环给出一组能压上家的合法牌。
  const [hintMsg, setHintMsg] = useState<string | null>(null);
  const [hintIdx, setHintIdx] = useState(0);
  useEffect(() => {
    if (!hintMsg) return;
    const t = setTimeout(() => setHintMsg(null), 1500);
    return () => clearTimeout(t);
  }, [hintMsg]);

  const hintList = useMemo<Card[][]>(() => {
    if (!myTurn) return [];
    const leading = !state.lastSell || state.lastSell.client === state.clientId;
    const last = leading ? null : evaluate(state.lastSell!.pokers);
    return legalPlays(state.hand, last);
  }, [myTurn, state.hand, state.lastSell, state.clientId]);

  const onHint = () => {
    if (!hintList.length) {
      setHintMsg('没有能压上的牌，可以不出');
      return;
    }
    const play = hintList[hintIdx % hintList.length];
    setHintIdx((i) => i + 1);
    actions.setSelection(indicesFor(state.hand, play));
  };

  return (
    <>
      <TopBar
        roomId={state.roomId}
        baseScore={state.baseScore}
        multiplier={state.multiplier}
        round={state.round}
      />

      <main className="table">
        <BottomStrip cards={state.bottomCards} />
        <CardTable state={state} />

        <section className={`player-area ${myTurn ? 'active' : ''}`}>
          {myTurn && <div className="turn-hint">{canPass ? '轮到你，请大过上家或不出' : '你来出牌 · 你是本轮的领出者'}</div>}

          <div className="player-meta">
            <div className="avatar">🧑‍✈️</div>
            <div className="who">
              <h3>你</h3>
              <div className="role">
                {myRole ? <span className={`role-badge ${myRole === 'LANDLORD' ? '' : 'farmer'}`}>{myRole === 'LANDLORD' ? '👑 地主' : '农民'}</span> : ''}
              </div>
            </div>
            <div className="count-pill">
              余牌 <b>{state.hand.length}</b>
            </div>
          </div>

          <Hand hand={state.hand} selected={state.selected} onToggle={actions.selectCard} />

          <ActionBar
            myTurn={myTurn}
            canPass={canPass}
            selectedCount={state.selected.length}
            onPlay={() => actions.play()}
            onPass={() => actions.pass()}
            onHint={onHint}
          />
        </section>
      </main>

      {state.error && <Toast text={ERROR_TEXT[state.error] ?? '出牌错误'} />}
      {!state.error && hintMsg && <Toast text={hintMsg} />}
      {state.promptBid && <BiddingOverlay onGrab={actions.grab} />}
      {state.phase === 'over' && state.result && (
        <ResultOverlay
          winnerNickname={state.result.winnerNickname}
          winnerType={state.result.winnerType}
          myType={state.myType}
          bottomCards={state.bottomCards}
          baseScore={state.baseScore}
          multiplier={state.multiplier}
          onAgain={() => actions.playAgain()}
        />
      )}
    </>
  );
}
