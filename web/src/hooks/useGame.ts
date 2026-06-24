import { useEffect, useReducer, useRef } from 'react';
import { connectSocket, SocketHandle } from '../net/socket';
import { gameReducer, initialState } from '../state/gameReducer';
import { Card, ClientEvent } from '../types';

export function useGame() {
  const [state, dispatch] = useReducer(gameReducer, initialState);
  const socketRef = useRef<SocketHandle | null>(null);

  useEffect(() => {
    const url = import.meta.env.VITE_WS_URL ?? 'ws://127.0.0.1:8787';
    const handle = connectSocket(url, (event) => dispatch({ type: 'server', event }));
    socketRef.current = handle;
    return () => handle.close();
  }, []);

  const send = (msg: ClientEvent) => socketRef.current?.send(msg);

  const actions = {
    setNickname: (nickname: string) => send({ event: 'setNickname', data: { nickname } }),
    createRoom: () => send({ event: 'createRoomPve', data: {} }),
    grab: (g: boolean) => send({ event: 'landlordElect', data: { grab: g } }),
    play: (pokers: Card[]) => send({ event: 'play', data: { pokers } }),
    pass: () => send({ event: 'pass', data: {} }),
    selectCard: (index: number) => dispatch({ type: 'select', index }),
    reset: () => dispatch({ type: 'reset' }),
  };

  return { state, actions };
}
