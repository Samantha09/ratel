import { useState } from 'react';
import { useGame } from './hooks/useGame';
import { LobbyView } from './views/LobbyView';
import { MainMenu } from './views/MainMenu';
import { RoomManager } from './views/RoomManager';
import { WaitingRoom } from './views/WaitingRoom';
import { GameView } from './views/GameView';

const genCode = () => String(1000 + Math.floor(Math.random() * 9000));

export default function App() {
  const { state, actions } = useGame();
  // Mock PVP 仅在前端演示;房号只用于等待室展示,真实对局仍走 createRoomPve。
  const [roomCode, setRoomCode] = useState<string | null>(null);

  if (state.phase === 'connecting' || state.phase === 'lobby') {
    if (state.lobbyScreen === 'menu') {
      return <MainMenu onPve={actions.createRoom} onPvp={() => actions.gotoLobby('rooms')} />;
    }
    if (state.lobbyScreen === 'rooms') {
      return (
        <RoomManager
          onJoin={(code) => {
            setRoomCode(code);
            actions.gotoLobby('waiting');
          }}
          onCreate={() => {
            setRoomCode(genCode());
            actions.gotoLobby('waiting');
          }}
          onBack={() => actions.gotoLobby('menu')}
        />
      );
    }
    if (state.lobbyScreen === 'waiting') {
      return (
        <WaitingRoom
          code={roomCode ?? '----'}
          onReady={actions.createRoom}
          onLeave={() => actions.gotoLobby('menu')}
        />
      );
    }
    // 'nickname' 子屏(或 connecting 中)
    return (
      <LobbyView
        connecting={state.phase === 'connecting'}
        lobbyError={state.lobbyError}
        onClearLobbyError={actions.clearLobbyError}
        onCreate={(nickname) => {
          actions.setNickname(nickname);
          actions.gotoLobby('menu');
        }}
      />
    );
  }

  return (
    <GameView
      state={state}
      actions={{
        selectCard: actions.selectCard,
        setSelection: actions.setSelection,
        play: () => actions.play(state.selected.map((i) => state.hand[i]).filter(Boolean)),
        pass: actions.pass,
        grab: actions.grab,
        reset: actions.reset,
        playAgain: actions.playAgain,
      }}
    />
  );
}
