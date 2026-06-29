import { createRoot } from 'react-dom/client';
import './styles/index.css';
import { Card } from './types';
import { GameState } from './state/gameReducer';
import { LobbyView } from './views/LobbyView';
import { GameView, GameActions } from './views/GameView';

// A static gallery that renders the real views with crafted state so we can
// capture deterministic screenshots (no live gateway needed). Pick a scene with
// ?scene=lobby|bidding|playing|result.

const card = (level: number, type: Card['type'] = 1): Card => ({ type, level });
const noop = () => {};
const actions: GameActions = {
  selectCard: noop,
  play: noop,
  pass: noop,
  grab: noop,
  reset: noop,
};

// 11-card hand, sorted, with a trio + pair selected for a lively shot.
const hand: Card[] = [
  card(0, 4), card(1, 1), card(2, 3), card(4, 2), card(4, 4), card(4, 1),
  card(6, 3), card(8, 1), card(10, 4), card(11, 2), card(14, 0),
];

const base: GameState = {
  phase: 'playing',
  connected: true,
  clientId: 1,
  nickname: '我',
  roomId: 7,
  hand,
  selected: [3, 4, 5],
  seats: [
    { id: 1, nickname: '我', cardsLeft: 11, isLandlord: false },
    { id: -1, nickname: 'robot_1', cardsLeft: 6, isLandlord: true },
    { id: -2, nickname: 'robot_2', cardsLeft: 9, isLandlord: false },
  ],
  turnClientId: 1,
  lastSell: {
    client: -1,
    nickname: 'robot_1',
    pokers: [card(7, 3), card(7, 1)],
    type: 0,
  },
  landlord: -1,
  myType: 'PEASANT',
  result: null,
  error: null,
  lobbyError: null,
  promptBid: false,
};

const biddingState: GameState = {
  ...base,
  phase: 'bidding',
  seats: [],
  hand: [...hand, card(5, 2), card(7, 4), card(9, 1), card(12, 3), card(13, 0), card(3, 1)],
  selected: [],
  lastSell: null,
  turnClientId: 1,
  promptBid: true,
};

const resultState: GameState = {
  ...base,
  phase: 'over',
  myType: 'LANDLORD',
  seats: base.seats.map((s) => ({ ...s, isLandlord: s.id === 1 })),
  result: { winnerNickname: '我', winnerType: 'LANDLORD' },
};

const scene = new URLSearchParams(location.search).get('scene') ?? 'playing';

function Scene() {
  if (scene === 'lobby') return <LobbyView connecting={false} onCreate={noop} />;
  if (scene === 'bidding') return <GameView state={biddingState} actions={actions} />;
  if (scene === 'result') return <GameView state={resultState} actions={actions} />;
  return <GameView state={base} actions={actions} />;
}

createRoot(document.getElementById('root')!).render(<Scene />);
