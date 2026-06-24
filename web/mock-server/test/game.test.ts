import { describe, it, expect } from 'vitest';
import { Game } from '../src/game.js';
import { ServerEvent } from '../src/types.js';

function makeGame(opts: { autoHuman?: boolean; rng?: () => number } = {}) {
  const events: ServerEvent[] = [];
  const game = new Game({ humanNickname: 'san', emit: (e) => events.push(e), ...opts });
  return { game, events };
}

describe('Game', () => {
  it('start emits roomCreate and gameStarting', () => {
    const { game, events } = makeGame();
    game.start();
    const names = events.map((e) => e.event);
    expect(names).toContain('roomCreate');
    expect(names).toContain('gameStarting');
  });

  it('default bidding prompts the human first (landlordElect client 0)', () => {
    const { game, events } = makeGame();
    game.start();
    const elect = events.find((e) => e.event === 'landlordElect') as
      | Extract<ServerEvent, { event: 'landlordElect' }> | undefined;
    expect(elect).toBeDefined();
    expect(elect!.data.client).toBe(0);
  });

  it('grabbing landlord yields landlordConfirm + showPokers', () => {
    const { game, events } = makeGame();
    game.start();
    game.onHumanLandlordElect(true);
    expect(events.some((e) => e.event === 'landlordConfirm')).toBe(true);
    const pok = events.find((e) => e.event === 'showPokers') as
      | Extract<ServerEvent, { event: 'showPokers' }> | undefined;
    expect(pok).toBeDefined();
    expect(pok!.data.pokers.length).toBe(20); // 17 + 3 bottom for the landlord
  });

  it('passing while leading is rejected with cantPass', () => {
    const { game, events } = makeGame();
    game.start();
    game.onHumanLandlordElect(true); // human is landlord, leads, lastPlay=null
    game.onHumanPass();
    const err = events.find((e) => e.event === 'playError') as
      | Extract<ServerEvent, { event: 'playError' }> | undefined;
    expect(err).toBeDefined();
    expect(err!.data.reason).toBe('cantPass');
  });

  it('every autoHuman game terminates at gameOver (deterministic + 20 random deals)', () => {
    // deterministic deal
    let seed = 1;
    const rng = () => {
      seed = (seed * 1103515245 + 12345) & 0x7fffffff;
      return seed / 0x7fffffff;
    };
    for (let i = 0; i < 20; i++) {
      const { game, events } = makeGame({ autoHuman: true, rng });
      game.start();
      expect(events.some((e) => e.event === 'gameOver')).toBe(true);
    }
  });
});
