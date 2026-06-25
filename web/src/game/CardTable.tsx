import { GameState, SeatInfo } from '../state/gameReducer';
import { PlayingCard } from '../components/PlayingCard';

function seatLabel(seat: SeatInfo, me: number | null): string {
  if (seat.id === me) return '我';
  if (seat.nickname) return seat.nickname;
  return seat.id === 1 ? '左家' : '右家';
}

function initials(name: string): string {
  return name.trim().slice(0, 1).toUpperCase() || '?';
}

function OpponentChip({ seat, me, active }: { seat: SeatInfo; me: number | null; active: boolean }) {
  const name = seatLabel(seat, me);
  return (
    <div
      className={[
        'edge-highlight flex min-w-[150px] items-center gap-3 rounded-lg border px-3 py-2.5 transition-colors',
        active ? 'animate-pulse-ring border-primary bg-surface-2' : 'border-hairline bg-surface-1',
      ].join(' ')}
    >
      <div
        className={[
          'flex h-9 w-9 shrink-0 items-center justify-center rounded-full text-sm font-semibold',
          seat.isLandlord ? 'bg-landlord/15 text-landlord' : 'bg-surface-3 text-ink-muted',
        ].join(' ')}
        aria-hidden="true"
      >
        {initials(name)}
      </div>
      <div className="min-w-0 flex-1">
        <div className="flex items-center gap-1.5">
          <span className="truncate text-sm font-medium text-ink">{name}</span>
          {seat.isLandlord && (
            <span className="rounded-pill bg-landlord/15 px-1.5 py-px text-[10px] font-medium text-landlord">
              地主
            </span>
          )}
        </div>
        <div className="mt-0.5 flex items-center gap-1.5">
          <span aria-hidden="true" className="text-xs text-ink-tertiary">🂠</span>
          <span className="font-mono text-xs text-ink-subtle">{seat.cardsLeft} 张</span>
          {active && <span className="ml-1 text-[11px] font-medium text-primary-hover">思考中…</span>}
        </div>
      </div>
    </div>
  );
}

function PlaceholderChip({ name }: { name: string }) {
  return (
    <div className="flex min-w-[150px] items-center gap-3 rounded-lg border border-hairline bg-surface-1/70 px-3 py-2.5">
      <div className="flex h-9 w-9 shrink-0 items-center justify-center rounded-full bg-surface-3 text-sm font-semibold text-ink-subtle">
        {name.slice(-1)}
      </div>
      <div className="min-w-0 flex-1">
        <span className="truncate text-sm font-medium text-ink-muted">{name}</span>
        <div className="mt-0.5 flex items-center gap-1.5">
          <span className="h-1.5 w-1.5 rounded-full bg-success" />
          <span className="text-xs text-success">准备就绪</span>
        </div>
      </div>
    </div>
  );
}

export interface CardTableProps {
  state: GameState;
}

export function CardTable({ state }: CardTableProps) {
  const opponents = state.seats.filter((s) => s.id !== state.clientId);
  const lastSell = state.lastSell;
  const passed = lastSell != null && lastSell.pokers.length === 0;

  return (
    <div className="flex h-full w-full flex-col justify-between gap-4">
      <div className="flex w-full flex-wrap justify-center gap-4">
        {opponents.length > 0 ? (
          opponents.map((s) => (
            <OpponentChip key={s.id} seat={s} me={state.clientId} active={state.turnClientId === s.id} />
          ))
        ) : (
          // PvE always has two robots; the gateway only sends their identities
          // once play starts, so show them as ready until real seats arrive.
          <>
            <PlaceholderChip name="robot_1" />
            <PlaceholderChip name="robot_2" />
          </>
        )}
      </div>

      <div className="grid flex-1 place-items-center">
        <div className="flex min-h-[8.5rem] w-full max-w-xl flex-col items-center justify-center gap-3 rounded-xl border border-hairline/70 bg-canvas/40 px-5 py-4 shadow-[inset_0_2px_12px_rgba(0,0,0,0.45)]">
          {lastSell && !passed ? (
            <>
              <span className="text-[11px] uppercase tracking-[0.18em] text-ink-tertiary">
                {lastSell.nickname || '上家'} 出牌
              </span>
              <div className="flex flex-wrap justify-center gap-1.5">
                {lastSell.pokers.map((card, i) => (
                  <PlayingCard key={`${card.level}:${card.type}:${i}`} card={card} mini />
                ))}
              </div>
            </>
          ) : passed ? (
            <span className="rounded-pill border border-hairline bg-surface-2 px-3 py-1 text-xs text-ink-subtle">
              {lastSell?.nickname || '对手'} 选择不出
            </span>
          ) : (
            <span className="text-sm text-ink-tertiary">等待出牌…</span>
          )}
        </div>
      </div>

      <div className="h-1" aria-hidden="true" />
    </div>
  );
}
