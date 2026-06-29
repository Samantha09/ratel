import { useEffect, useReducer, useRef, useState } from 'react';
import { connectSocket, SocketHandle } from '../net/socket';
import { gameReducer, initialState, LobbyScreen } from '../state/gameReducer';
import { Card, ClientEvent } from '../types';

export function useGame() {
  const [state, dispatch] = useReducer(gameReducer, initialState);
  const socketRef = useRef<SocketHandle | null>(null);
  // Bumping `gen` tears down the old socket and opens a fresh one. The gateway
  // shuts the WS down after gameOver (CODE_CLIENT_EXIT), so replaying a game
  // requires a brand-new connection — same effect as a manual page refresh.
  const [gen, setGen] = useState(0);
  // When set, the next `connected` auto-resends setNickname + createRoomPve so
  // "再来一局" skips the lobby and deals straight into the next round.
  const pendingReplayRef = useRef<string | null>(null);

  useEffect(() => {
    const url = import.meta.env.VITE_WS_URL ?? 'ws://127.0.0.1:8787';
    const handle = connectSocket(url, (event) => dispatch({ type: 'server', event }));
    socketRef.current = handle;
    return () => handle.close();
  }, [gen]);

  const send = (msg: ClientEvent) => socketRef.current?.send(msg);

  useEffect(() => {
    if (state.connected && pendingReplayRef.current != null) {
      const nickname = pendingReplayRef.current;
      pendingReplayRef.current = null;
      send({ event: 'setNickname', data: { nickname } });
      send({ event: 'createRoomPve', data: {} });
    }
  }, [state.connected]);

  const actions = {
    setNickname: (nickname: string) => send({ event: 'setNickname', data: { nickname } }),
    createRoom: () => send({ event: 'createRoomPve', data: {} }),
    gotoLobby: (screen: LobbyScreen) => dispatch({ type: 'gotoLobby', screen }),
    grab: (g: boolean) => send({ event: 'landlordElect', data: { grab: g } }),
    play: (pokers: Card[]) => send({ event: 'play', data: { pokers } }),
    pass: () => send({ event: 'pass', data: {} }),
    selectCard: (index: number) => dispatch({ type: 'select', index }),
    setSelection: (indices: number[]) => dispatch({ type: 'setSelection', indices }),
    clearLobbyError: () => dispatch({ type: 'clearLobbyError' }),
    reset: () => {
      dispatch({ type: 'reset' });
      setGen((g) => g + 1); // reconnect for a fresh game
    },
    playAgain: () => {
      pendingReplayRef.current = state.nickname; // replay intent for the new socket
      dispatch({ type: 'bumpRound' });
      dispatch({ type: 'reset' });
      setGen((g) => g + 1);
    },
  };

  return { state, actions };
}
