import { useEffect, useState } from 'react';
import { GameState } from '../state/gameReducer';
import { CardTable } from '../game/CardTable';
import { Hand } from '../game/Hand';
import { ActionBar } from '../game/ActionBar';
import { BiddingOverlay } from '../game/BiddingOverlay';
import { ResultOverlay } from '../game/ResultOverlay';
import { Toast } from '../components/Toast';

export interface GameActions {
  selectCard: (i: number) => void;
  play: () => void;
  pass: () => void;
  grab: (g: boolean) => void;
  reset: () => void;
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

function TopBar({ roomId }: { roomId: number | null }) {
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
          <span className="dot" />底分 <b>3</b>
        </div>
        <div className="chip">
          倍数 <b>1</b>
        </div>
        <div className="chip">
          第 <b>1</b> 局
        </div>
      </div>
    </header>
  );
}

export function GameView({ state, actions }: GameViewProps) {
  const myTurn = state.clientId != null && state.turnClientId === state.clientId;
  const canPass = state.lastSell != null && state.lastSell.client !== state.clientId;
  const myRole = state.myType;

  // 提示按钮为装饰(无客户端合法牌引擎),点击给一条短暂 toast,不改动游戏逻辑。
  const [hint, setHint] = useState<string | null>(null);
  useEffect(() => {
    if (!hint) return;
    const t = setTimeout(() => setHint(null), 1500);
    return () => clearTimeout(t);
  }, [hint]);

  return (
    <>
      <TopBar roomId={state.roomId} />

      <main className="table">
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
            onHint={() => setHint('提示功能暂未开放')}
          />
        </section>
      </main>

      {state.error && <Toast text={ERROR_TEXT[state.error] ?? '出牌错误'} />}
      {!state.error && hint && <Toast text={hint} />}
      {state.promptBid && <BiddingOverlay onGrab={actions.grab} />}
      {state.phase === 'over' && state.result && (
        <ResultOverlay
          winnerNickname={state.result.winnerNickname}
          winnerType={state.result.winnerType}
          myType={state.myType}
          onAgain={() => actions.reset()}
        />
      )}
    </>
  );
}
