import { GameState, SeatInfo } from '../state/gameReducer';
import { PlayingCard } from '../components/PlayingCard';

function seatLabel(seat: SeatInfo, me: number | null, fallback: string): string {
  if (seat.id === me) return '我';
  if (seat.nickname) return seat.nickname;
  return fallback;
}

function Seat({
  seat,
  side,
  emoji,
  fallbackName,
  landlordKnown,
  active,
}: {
  seat: SeatInfo | null;
  side: 'left' | 'right';
  emoji: string;
  fallbackName: string;
  landlordKnown: boolean;
  active: boolean;
}) {
  const name = seat ? seatLabel(seat, null, fallbackName) : fallbackName;
  const count = seat?.cardsLeft ?? 17;
  const isLandlord = seat?.isLandlord ?? false;
  const fanCount = Math.min(count, 20);

  return (
    <section className={`seat seat-${side} ${active ? 'active' : ''}`}>
      <div className="avatar-row">
        <div className="avatar">{emoji}</div>
        <div className="who">
          <h3>{name}</h3>
          <div className="role">
            {landlordKnown ? (
              <span className={`role-badge ${isLandlord ? '' : 'farmer'}`}>{isLandlord ? '👑 地主' : '农民'}</span>
            ) : (
              ''
            )}
          </div>
        </div>
      </div>
      <div className="count-pill">
        余牌 <b>{count}</b>
      </div>
      <div className="back-fan">
        {Array.from({ length: fanCount }, (_, i) => (
          <div key={i} className="cb" />
        ))}
      </div>
      <div className="thinking">思考中…</div>
    </section>
  );
}

export interface CardTableProps {
  state: GameState;
}

export function CardTable({ state }: CardTableProps) {
  const opponents = state.seats.filter((s) => s.id !== state.clientId);
  const left = opponents[0] ?? null;
  const right = opponents[1] ?? null;
  const landlordKnown = state.landlord != null;

  const lastSell = state.lastSell;
  const passed = lastSell != null && lastSell.pokers.length === 0;
  const hasPlay = lastSell != null && lastSell.pokers.length > 0;
  const leaderName = lastSell?.nickname || '上家';

  return (
    <>
      {/* 出牌区:三列布局,中列显示最近一手(沿用原有 lastSell 语义) */}
      <div className="play-zone">
        <div className="played" id="play-left" />
        <div className="played" id="play-center">
          {hasPlay ? (
            <>
              <div className="lead-tag">▲ {leaderName}</div>
              <div className="mini-row">
                {lastSell!.pokers.map((card, i) => (
                  <PlayingCard
                    key={`${card.level}:${card.type}:${i}`}
                    card={card}
                    variant="mini"
                    style={{ animationDelay: `${i * 0.04}s` }}
                  />
                ))}
              </div>
            </>
          ) : passed ? (
            <div className="pass-bubble">不 出</div>
          ) : (
            <div className="center-emblem">
              <div className="ring">♠</div>
              <span style={{ fontSize: 11, letterSpacing: 2 }}>出牌区</span>
            </div>
          )}
        </div>
        <div className="played" id="play-right" />
      </div>

      <Seat
        seat={left}
        side="left"
        emoji="🤖"
        fallbackName="阿尔法"
        landlordKnown={landlordKnown}
        active={left != null && state.turnClientId === left.id}
      />
      <Seat
        seat={right}
        side="right"
        emoji="👾"
        fallbackName="贝塔"
        landlordKnown={landlordKnown}
        active={right != null && state.turnClientId === right.id}
      />
    </>
  );
}
