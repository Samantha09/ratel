import { describe, it, expect, vi } from 'vitest';
import { render, screen } from '@testing-library/react';
import userEvent from '@testing-library/user-event';
import { LobbyView } from './LobbyView';

describe('LobbyView', () => {
  it('calls onCreate with the entered nickname', async () => {
    const onCreate = vi.fn();
    const user = userEvent.setup();
    render(<LobbyView connecting={false} onCreate={onCreate} />);
    await user.type(screen.getByPlaceholderText('请输入昵称'), 'san');
    await user.click(screen.getByRole('button', { name: '开始游戏' }));
    expect(onCreate).toHaveBeenCalledWith('san');
  });
});
