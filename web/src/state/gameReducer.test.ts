import { describe, it, expect } from 'vitest';
import { reducer, initialState } from './gameReducer';
import { ServerEvent, Card } from '../types';

const c = (level: number, type = 1): Card => ({ type: type as Card['type'], level });

function apply(events: ServerEvent[]) {
  return events.reduce((state, event) => reducer(state, { type: 'server', event }), { ...initialState, clientId: 0 });
}

describe('reducer', () => {
  it('connected + idSet sets connection state', () => {
    const s = apply([{ event: 'connected', data: {} }, { event: 'idSet', data: { clientId: 0 } }]);
    expect(s.connected).toBe(true);
    expect(s.clientId).toBe(0);
    expect(s.phase).toBe('lobby');
  });

  it('roomCreate stores roomId', () => {
    const s = apply([{ event: 'connected', data: {} }, { event: 'idSet', data: { clientId: 0 } },
      { event: 'roomCreate', data: { roomId: 1234, owner: 'san', clientCount: 3, type: 1 } }]);
    expect(s.roomId).toBe(1234);
  });

  it('gameStarting + landlordConfirm move to playing and set landlord', () => {
    const s = apply([
      { event: 'gameStarting', data: {} },
      { event: 'landlordConfirm', data: { landlord: 0, nickname: 'san' } },
    ]);
    expect(s.phase).toBe('playing');
    expect(s.landlord).toBe(0);
    expect(s.seats[0].isLandlord).toBe(true);
  });

  it('showPokers sets the hand', () => {
    const s = apply([{ event: 'showPokers', data: { pokers: [c(0), c(1), c(2)] } }]);
    expect(s.hand.length).toBe(3);
  });

  it('landlordElect addressed to me prompts bidding', () => {
    const s = apply([{ event: 'landlordElect', data: { client: 0, nickname: 'san' } }]);
    expect(s.promptBid).toBe(true);
  });

  it('playRedirect by me removes those cards from my hand and clears selection', () => {
    let s = apply([{ event: 'showPokers', data: { pokers: [c(0), c(1), c(2)] } }]);
    s = reducer(s, { type: 'select', index: 0 });
    expect(s.selected).toEqual([0]);
    s = reducer(s, {
      type: 'server',
      event: { event: 'playRedirect', data: { sellClient: 0, sellNickname: 'san', sellPokers: [c(0)], sellType: 3, nextClient: 1 } },
    });
    expect(s.hand.length).toBe(2);
    expect(s.selected).toEqual([]);
    expect(s.turnClientId).toBe(1);
  });

  it('playRedirect by an opponent decrements their cardsLeft', () => {
    let s = apply([{ event: 'landlordConfirm', data: { landlord: 1, nickname: 'Bot' } }]);
    const before = s.seats[1].cardsLeft; // 20 (landlord)
    s = reducer(s, {
      type: 'server',
      event: { event: 'playRedirect', data: { sellClient: 1, sellNickname: 'Bot', sellPokers: [c(0), c(1)], sellType: 3, nextClient: 2 } },
    });
    expect(s.seats[1].cardsLeft).toBe(before - 2);
  });

  it('playError sets error and keeps selection', () => {
    let s = apply([{ event: 'showPokers', data: { pokers: [c(0)] } }]);
    s = reducer(s, { type: 'select', index: 0 });
    s = reducer(s, { type: 'server', event: { event: 'playError', data: { reason: 'less' } } });
    expect(s.error).toBe('less');
    expect(s.selected).toEqual([0]);
  });

  it('gameOver sets result and phase', () => {
    const s = apply([{ event: 'gameOver', data: { winner: 0, landlord: 0, nickname: 'san' } }]);
    expect(s.phase).toBe('over');
    expect(s.result?.winner).toBe(0);
  });

  it('select toggles an index', () => {
    let s = apply([{ event: 'showPokers', data: { pokers: [c(0), c(1)] } }]);
    s = reducer(s, { type: 'select', index: 1 });
    expect(s.selected).toEqual([1]);
    s = reducer(s, { type: 'select', index: 1 });
    expect(s.selected).toEqual([]);
  });
});
