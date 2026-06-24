import { GameState, SeatInfo } from '../state/gameReducer';

function seatLabel(seat: SeatInfo, me: number | null): string {
  if (seat.id === me) return '我';
  if (seat.nickname) return seat.nickname;
  return seat.id === 1 ? '左家' : '右家';
}

export interface CardTableProps {
  state: GameState;
}

export function CardTable({ state }: CardTableProps) {
  const opponents = state.seats.filter((s) => s.id !== state.clientId);
  return (
    <div className="flex flex-col items-center gap-6">
      <div className="flex w-full justify-center gap-8">
        {opponents.map((s) => (
          <div
            key={s.id}
            className={`rounded-lg border px-4 py-2 text-center ${
              state.turnClientId === s.id ? 'border-primary bg-surface-2' : 'border-hairline bg-surface-1'
            }`}
          >
            <div className="text-sm text-ink">
              {seatLabel(s, state.clientId)}
              {s.isLandlord ? ' · 地主' : ''}
            </div>
            <div className="font-mono text-xs text-ink-subtle">剩 {s.cardsLeft} 张</div>
          </div>
        ))}
      </div>

      <div className="min-h-[7rem] rounded-lg border border-hairline bg-surface-1 px-4 py-2">
        {state.lastSell ? (
          <div className="flex items-center gap-2">
            <span className="text-xs text-ink-subtle">{state.lastSell.nickname}：</span>
            <div className="flex gap-1">
              {state.lastSell.pokers.map((card, i) => (
                <span key={i} className="font-mono text-sm text-ink">
                  {cardLabel(card)}
                </span>
              ))}
            </div>
          </div>
        ) : (
          <span className="text-xs text-ink-tertiary">等待出牌…</span>
        )}
      </div>
    </div>
  );
}

const RANK = ['3', '4', '5', '6', '7', '8', '9', '10', 'J', 'Q', 'K', 'A', '2'];
function cardLabel(card: { level: number }): string {
  if (card.level === 13) return '小王';
  if (card.level === 14) return '大王';
  return RANK[card.level];
}
