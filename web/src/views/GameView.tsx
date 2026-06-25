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

export function GameView({ state, actions }: GameViewProps) {
  const myTurn = state.clientId != null && state.turnClientId === state.clientId;

  return (
    <div className="relative mx-auto flex h-full max-w-3xl flex-col gap-6 p-6">
      <CardTable state={state} />

      <div className="flex-1" />

      <Hand hand={state.hand} selected={state.selected} onToggle={actions.selectCard} />

      <ActionBar
        myTurn={myTurn}
        canPass={state.lastSell != null && state.lastSell.client !== state.clientId}
        onPlay={() => actions.play()}
        onPass={() => actions.pass()}
      />

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
