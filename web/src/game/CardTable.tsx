import { GameState, SeatInfo, SeatPlay } from '../state/gameReducer';
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

/** One column of the 3-column play zone: a seat's most recent play. */
function PlayedColumn({
  play,
  isLeader,
}: {
  play: SeatPlay | undefined;
  isLeader: boolean;
}) {
  if (play?.passed) {
    return <div className="pass-bubble">不 出</div>;
  }
  if (play && play.pokers.length > 0) {
    return (
      <>
        {isLeader && <div className="lead-tag">▲ 领出</div>}
        <div className="mini-row">
          {play.pokers.map((card, i) => (
            <PlayingCard
              key={`${card.level}:${card.type}:${i}`}
              card={card}
              variant="mini"
              style={{ animationDelay: `${i * 0.04}s` }}
            />
          ))}
        </div>
      </>
    );
  }
  return null;
}

export interface CardTableProps {
  state: GameState;
}

export function CardTable({ state }: CardTableProps) {
  const me = state.clientId;
  const opponents = state.seats.filter((s) => s.id !== me);

  // Gateway position: "up" = upstream/previous player (left in counter-clockwise layout),
  // "down" = downstream/next player (right). Empty = me.
  const left = opponents.find((s) => s.position === 'up') ?? opponents[0] ?? null;
  const right = opponents.find((s) => s.position === 'down') ?? opponents[1] ?? null;
  const landlordKnown = state.landlord != null;
  const leaderId = state.lastSell?.client ?? null;

  const { playsBySeat } = state;

  return (
    <>
      {/* 两侧出牌区: 只显示两个对手的最近一手; 自己的出牌显示在 player-area 自己这边 */}
      <div className="play-zone">
        <div className="played" id="play-left">
          <PlayedColumn play={left ? playsBySeat[left.id] : undefined} isLeader={left?.id === leaderId} />
        </div>
        <div className="played" id="play-center">
          <div className="center-emblem">
            <div className="ring">♠</div>
            <span style={{ fontSize: 11, letterSpacing: 2 }}>出牌区</span>
          </div>
        </div>
        <div className="played" id="play-right">
          <PlayedColumn play={right ? playsBySeat[right.id] : undefined} isLeader={right?.id === leaderId} />
        </div>
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
