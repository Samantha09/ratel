import { useGame } from './hooks/useGame';
import { LobbyView } from './views/LobbyView';
import { GameView } from './views/GameView';

export default function App() {
  const { state, actions } = useGame();

  if (state.phase === 'connecting' || state.phase === 'lobby') {
    return (
      <LobbyView
        connecting={state.phase === 'connecting'}
        lobbyError={state.lobbyError}
        onClearLobbyError={actions.clearLobbyError}
        onCreate={(nickname) => {
          actions.setNickname(nickname);
          actions.createRoom();
        }}
      />
    );
  }

  return (
    <GameView
      state={state}
      actions={{
        selectCard: actions.selectCard,
        play: () => actions.play(state.selected.map((i) => state.hand[i]).filter(Boolean)),
        pass: actions.pass,
        grab: actions.grab,
        reset: actions.reset,
      }}
    />
  );
}
