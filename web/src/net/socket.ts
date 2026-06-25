import { ClientEvent, ServerEvent } from '../types';

export interface SocketHandle {
  send: (msg: ClientEvent) => void;
  close: () => void;
}

export function connectSocket(url: string, onEvent: (e: ServerEvent) => void): SocketHandle {
  const ws = new WebSocket(url);
  ws.onopen = () => onEvent({ event: 'connected', data: {} } as ServerEvent);
  ws.onmessage = (ev) => {
    try {
      onEvent(JSON.parse(ev.data));
    } catch {
      // ignore malformed frames
    }
  };
  return {
    send: (msg) => {
      if (ws.readyState === WebSocket.OPEN) ws.send(JSON.stringify(msg));
    },
    close: () => ws.close(),
  };
}
