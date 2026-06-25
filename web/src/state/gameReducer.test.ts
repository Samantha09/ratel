import { describe, it, expect } from 'vitest';
import { gameReducer, initialState, GameState } from './gameReducer';
import { ServerEvent, Card, ClientInfoMsg } from '../types';

const c = (level: number, type = 1): Card => ({ type: type as Card['type'], level });
const seat = (clientId: number, type: number, cardsLeft: number, nickname = 'x'): ClientInfoMsg => ({
  clientId,
  nickname,
  type,
  cardsLeft,
  position: '',
});

function apply(events: ServerEvent[], myId = 0): GameState {
  const start: GameState = { ...initialState, clientId: myId };
  return events.reduce((state, event) => gameReducer(state, { type: 'server', event }), start);
}

describe('gameReducer (real gateway contract)', () => {
  it('connected + idSet sets connection state', () => {
    const s = apply([{ event: 'connected', data: {} }, { event: 'idSet', data: { clientId: 7 } }], 7);
    expect(s.connected).toBe(true);
    expect(s.clientId).toBe(7);
    expect(s.phase).toBe('lobby');
  });

  it('gameStarting deals the hand, enters bidding, records first bidder', () => {
    const s = apply([
      { event: 'gameStarting', data: { clientId: 0, nextClientId: -2, pokers: [c(0), c(1), c(2)] } },
    ]);
    expect(s.phase).toBe('bidding');
    expect(s.hand.length).toBe(3);
    expect(s.turnClientId).toBe(-2);
  });

  it('landlordElect addressed to me prompts bidding; otherwise it does not', () => {
    expect(apply([{ event: 'landlordElect', data: { nextClientId: 0 } }]).promptBid).toBe(true);
    expect(apply([{ event: 'landlordElect', data: { nextClientId: -2 } }]).promptBid).toBe(false);
  });

  it('landlordConfirm: opponent landlord -> I am a peasant, landlord plays first', () => {
    const s = apply([
      { event: 'gameStarting', data: { pokers: [c(0), c(1)] } },
      { event: 'landlordConfirm', data: { landlordId: -2, landlordNickname: 'robot', additionalPokers: [c(5)] } },
    ]);
    expect(s.phase).toBe('playing');
    expect(s.landlord).toBe(-2);
    expect(s.myType).toBe('PEASANT');
    expect(s.turnClientId).toBe(-2);
    expect(s.hand.length).toBe(2); // not the landlord -> no bottom cards added
  });

  it('landlordConfirm: I am the landlord -> I get the 3 bottom cards', () => {
    const s = apply([
      { event: 'gameStarting', data: { pokers: [c(0), c(1)] } },
      { event: 'landlordConfirm', data: { landlordId: 0, additionalPokers: [c(5), c(6), c(7)] } },
    ]);
    expect(s.myType).toBe('LANDLORD');
    expect(s.hand.length).toBe(5);
  });

  it('showPokers from me removes those cards and ends my turn', () => {
    let s = apply([{ event: 'gameStarting', data: { pokers: [c(0), c(1), c(2)] } }]);
    s = gameReducer(s, { type: 'server', event: { event: 'playRedirect', data: { pokers: [c(0), c(1), c(2)], sellClientId: 0, clientInfos: [seat(0, 1, 3)] } } });
    expect(s.turnClientId).toBe(0);
    s = gameReducer(s, { type: 'server', event: { event: 'showPokers', data: { clientId: 0, pokers: [c(0)] } } });
    expect(s.hand.length).toBe(2);
    expect(s.turnClientId).toBeNull();
    expect(s.lastSell?.pokers.length).toBe(1);
  });

  it('showPokers from an opponent decrements their cardsLeft and shows the play', () => {
    let s = apply([
      { event: 'playRedirect', data: { pokers: [c(9)], sellClientId: 0, clientInfos: [seat(0, 1, 1), seat(-2, 0, 17, 'robot')] } },
    ]);
    s = gameReducer(s, { type: 'server', event: { event: 'showPokers', data: { clientId: -2, clientNickname: 'robot', pokers: [c(3), c(3, 2)] } } });
    const opp = s.seats.find((x) => x.id === -2)!;
    expect(opp.cardsLeft).toBe(15);
    expect(s.lastSell?.client).toBe(-2);
  });

  it('playRedirect makes it my turn, rebuilds seats and sets my hand', () => {
    const s = apply([
      {
        event: 'playRedirect',
        data: {
          pokers: [c(4), c(5)],
          lastSellPokers: [c(3)],
          lastSellClientId: -2,
          sellClientId: 0,
          clientInfos: [seat(0, 1, 2, 'me'), seat(-2, 0, 16, 'robot')],
        },
      },
    ]);
    expect(s.turnClientId).toBe(0);
    expect(s.hand.length).toBe(2);
    expect(s.seats).toHaveLength(2);
    expect(s.seats.find((x) => x.id === -2)?.isLandlord).toBe(true);
    expect(s.lastSell?.client).toBe(-2);
    expect(s.myType).toBe('PEASANT');
  });

  it('playError uses the `code` field', () => {
    const s = apply([{ event: 'playError', data: { code: 'less' } }]);
    expect(s.error).toBe('less');
  });

  it('playPass advances the turn to nextClientId (so a human leading after passes can act)', () => {
    const s = apply([{ event: 'playPass', data: { clientId: -2, nextClientId: 0 } }]);
    expect(s.turnClientId).toBe(0);
  });

  it('gameOver records winner nickname and type', () => {
    const s = apply([{ event: 'gameOver', data: { winnerNickname: 'robot', winnerType: 'LANDLORD' } }]);
    expect(s.phase).toBe('over');
    expect(s.result?.winnerType).toBe('LANDLORD');
    expect(s.result?.winnerNickname).toBe('robot');
  });

  it('select toggles an index', () => {
    let s = apply([{ event: 'gameStarting', data: { pokers: [c(0), c(1)] } }]);
    s = gameReducer(s, { type: 'select', index: 1 });
    expect(s.selected).toEqual([1]);
    s = gameReducer(s, { type: 'select', index: 1 });
    expect(s.selected).toEqual([]);
  });

  it('reset clears to a fresh connecting state (useGame then reconnects the socket)', () => {
    let s = apply([{ event: 'connected', data: {} }, { event: 'idSet', data: { clientId: 7 } }], 7);
    s = gameReducer(s, { type: 'server', event: { event: 'gameOver', data: { winnerNickname: 'x', winnerType: 'PEASANT' } } });
    s = gameReducer(s, { type: 'reset' });
    expect(s.phase).toBe('connecting');
    expect(s.connected).toBe(false);
    expect(s.clientId).toBeNull();
    expect(s.result).toBeNull();
    // the fresh socket's `connected` event brings us back to the lobby
    s = gameReducer(s, { type: 'server', event: { event: 'connected', data: {} } });
    expect(s.phase).toBe('lobby');
  });
});
