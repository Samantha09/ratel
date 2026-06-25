import { useEffect, useReducer, useRef, useState } from 'react';
import { connectSocket, SocketHandle } from '../net/socket';
import { gameReducer, initialState } from '../state/gameReducer';
import { Card, ClientEvent } from '../types';

export function useGame() {
  const [state, dispatch] = useReducer(gameReducer, initialState);
  const socketRef = useRef<SocketHandle | null>(null);
  // Bumping `gen` tears down the old socket and opens a fresh one. The gateway
  // shuts the WS down after gameOver (CODE_CLIENT_EXIT), so replaying a game
  // requires a brand-new connection — same effect as a manual page refresh.
  const [gen, setGen] = useState(0);

  useEffect(() => {
    const url = import.meta.env.VITE_WS_URL ?? 'ws://127.0.0.1:8787';
    const handle = connectSocket(url, (event) => dispatch({ type: 'server', event }));
    socketRef.current = handle;
    return () => handle.close();
  }, [gen]);

  const send = (msg: ClientEvent) => socketRef.current?.send(msg);

  const actions = {
    setNickname: (nickname: string) => send({ event: 'setNickname', data: { nickname } }),
    createRoom: () => send({ event: 'createRoomPve', data: {} }),
    grab: (g: boolean) => send({ event: 'landlordElect', data: { grab: g } }),
    play: (pokers: Card[]) => send({ event: 'play', data: { pokers } }),
    pass: () => send({ event: 'pass', data: {} }),
    selectCard: (index: number) => dispatch({ type: 'select', index }),
    reset: () => {
      dispatch({ type: 'reset' });
      setGen((g) => g + 1); // reconnect for a fresh game
    },
  };

  return { state, actions };
}
