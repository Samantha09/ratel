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

function TopBar({ state, myTurn }: { state: GameState; myTurn: boolean }) {
  const isLandlord = state.myType === 'LANDLORD';
  return (
    <header className="flex items-center justify-between">
      <div className="flex items-center gap-2.5">
        <div className="flex h-7 w-7 items-center justify-center rounded-md bg-primary text-sm font-bold text-on-primary">
          R
        </div>
        <span className="display-tracking font-display text-sm font-semibold text-ink">ratel</span>
        {state.roomId != null && (
          <span className="rounded-pill border border-hairline bg-surface-1 px-2 py-0.5 font-mono text-[11px] text-ink-subtle">
            #{state.roomId}
          </span>
        )}
      </div>

      <div className="flex items-center gap-2">
        {state.myType && (
          <span
            className={[
              'rounded-pill px-2.5 py-0.5 text-[11px] font-medium',
              isLandlord ? 'bg-landlord/15 text-landlord' : 'bg-surface-2 text-ink-muted',
            ].join(' ')}
          >
            {isLandlord ? '我是地主' : '我是农民'}
          </span>
        )}
        <span
          className={[
            'flex items-center gap-1.5 rounded-pill px-2.5 py-0.5 text-[11px] font-medium',
            myTurn ? 'bg-primary/15 text-primary-hover' : 'bg-surface-1 text-ink-subtle',
          ].join(' ')}
        >
          <span className={`h-1.5 w-1.5 rounded-full ${myTurn ? 'bg-primary-hover' : 'bg-ink-tertiary'}`} />
          {myTurn ? '你的回合' : '等待中'}
        </span>
      </div>
    </header>
  );
}

export function GameView({ state, actions }: GameViewProps) {
  const myTurn = state.clientId != null && state.turnClientId === state.clientId;

  return (
    <div className="relative mx-auto flex h-full w-full max-w-4xl flex-col gap-4 p-4 sm:p-6">
      <TopBar state={state} myTurn={myTurn} />

      <main className="table-surface edge-highlight relative flex flex-1 overflow-hidden rounded-3xl border border-hairline-strong p-5">
        <CardTable state={state} />
      </main>

      <section
        aria-label="我的手牌"
        className="flex flex-col gap-3 rounded-2xl border border-hairline bg-surface-1/50 px-3 py-4"
      >
        <Hand hand={state.hand} selected={state.selected} onToggle={actions.selectCard} />

        <ActionBar
          myTurn={myTurn}
          canPass={state.lastSell != null && state.lastSell.client !== state.clientId}
          selectedCount={state.selected.length}
          onPlay={() => actions.play()}
          onPass={() => actions.pass()}
        />
      </section>

      {state.error && <Toast text={ERROR_TEXT[state.error] ?? '出牌错误'} />}
      {state.promptBid && <BiddingOverlay onGrab={actions.grab} />}
      {state.phase === 'over' && state.result && (
        <ResultOverlay
          winnerNickname={state.result.winnerNickname}
          winnerType={state.result.winnerType}
          myType={state.myType}
          onAgain={() => actions.reset()}
        />
      )}
    </div>
  );
}
