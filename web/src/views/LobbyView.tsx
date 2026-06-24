import { useState } from 'react';
import { Button } from '../components/Button';

export interface LobbyViewProps {
  connecting: boolean;
  onCreate: (nickname: string) => void;
}

export function LobbyView({ connecting, onCreate }: LobbyViewProps) {
  const [nickname, setNickname] = useState('');

  return (
    <div className="flex h-full items-center justify-center">
      <div className="w-full max-w-sm rounded-xl border border-hairline bg-surface-1 p-8">
        <h1 className="font-display display-tracking text-3xl font-semibold text-ink">ratel</h1>
        <p className="mt-1 text-sm text-ink-subtle">斗地主 · 人机对战</p>
        <input
          value={nickname}
          onChange={(e) => setNickname(e.target.value)}
          placeholder="输入昵称"
          className="mt-6 w-full rounded-md border border-hairline bg-canvas px-3 py-2 text-sm text-ink outline-none focus:border-primary"
        />
        <Button
          className="mt-4 w-full"
          disabled={connecting || nickname.trim().length === 0}
          onClick={() => onCreate(nickname.trim())}
        >
          {connecting ? '连接中…' : '开始人机对战'}
        </Button>
      </div>
    </div>
  );
}
