import { useState } from 'react';
import { Button } from '../components/Button';

export interface LobbyViewProps {
  connecting: boolean;
  onCreate: (nickname: string) => void;
}

export function LobbyView({ connecting, onCreate }: LobbyViewProps) {
  const [nickname, setNickname] = useState('');
  const ready = !connecting && nickname.trim().length > 0;

  const submit = () => {
    if (ready) onCreate(nickname.trim());
  };

  return (
    <div className="flex h-full items-center justify-center p-6">
      <div className="edge-highlight w-full max-w-sm animate-scale-in rounded-xl border border-hairline-strong bg-surface-1 p-8 shadow-overlay">
        <div className="flex items-center gap-3">
          <div className="flex h-10 w-10 items-center justify-center rounded-lg bg-primary text-lg font-bold text-on-primary shadow-glow-primary">
            R
          </div>
          <span className="text-[11px] font-medium uppercase tracking-[0.28em] text-ink-subtle">
            斗地主 · 人机对战
          </span>
        </div>

        <h1 className="display-tracking mt-6 font-display text-4xl font-semibold text-ink">ratel</h1>
        <p className="mt-2 text-sm text-ink-subtle">
          输入昵称，立刻开一局单人对战。地主多得 3 张底牌。
        </p>

        <label htmlFor="nickname" className="mt-7 block text-xs font-medium text-ink-subtle">
          昵称
        </label>
        <input
          id="nickname"
          value={nickname}
          onChange={(e) => setNickname(e.target.value)}
          onKeyDown={(e) => e.key === 'Enter' && submit()}
          placeholder="输入昵称"
          autoFocus
          className="mt-1.5 w-full rounded-md border border-hairline bg-canvas px-3 py-2.5 text-sm text-ink transition-colors placeholder:text-ink-tertiary focus:border-primary-focus focus:outline-none focus:ring-2 focus:ring-primary/40"
        />

        <Button className="mt-5 w-full" size="lg" disabled={!ready} onClick={submit}>
          {connecting && (
            <span
              aria-hidden="true"
              className="h-3.5 w-3.5 animate-spin rounded-full border-2 border-on-primary/40 border-t-on-primary"
            />
          )}
          {connecting ? '连接中…' : '开始人机对战'}
        </Button>

        <p className="mt-5 text-center text-[11px] text-ink-tertiary">
          按 Enter 快速开始 · WS + JSON 实时对战
        </p>
      </div>
    </div>
  );
}
