import { describe, it, expect, vi } from 'vitest';
import { render, screen, fireEvent } from '@testing-library/react';
import { MainMenu } from './MainMenu';

describe('MainMenu', () => {
  it('renders both mode buttons', () => {
    render(<MainMenu onPve={() => {}} onPvp={() => {}} />);
    expect(screen.getByText(/人机对战/)).toBeInTheDocument();
    expect(screen.getByText(/玩家对战/)).toBeInTheDocument();
  });

  it('surfaces lobbyError when present (so a failed createRoomPve is not silent)', () => {
    render(
      <MainMenu
        onPve={() => {}}
        onPvp={() => {}}
        lobbyError="机器人资源不足，请重启 gateway 和 LLM agent 后再试"
      />,
    );
    expect(screen.getByText(/机器人资源不足/)).toBeInTheDocument();
  });

  it('clears the lobby error and requests PvE when 人机对战 is clicked', () => {
    const onPve = vi.fn();
    render(<MainMenu onPve={onPve} onPvp={() => {}} lobbyError="boom" />);
    fireEvent.click(screen.getByText(/人机对战/));
    expect(onPve).toHaveBeenCalledOnce();
  });
});
