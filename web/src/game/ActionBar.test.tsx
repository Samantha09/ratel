import { describe, it, expect, vi } from 'vitest';
import { render, screen } from '@testing-library/react';
import userEvent from '@testing-library/user-event';
import { ActionBar } from './ActionBar';

describe('ActionBar', () => {
  it('disables both buttons when not my turn', () => {
    render(<ActionBar myTurn={false} canPass={false} onPlay={() => {}} onPass={() => {}} />);
    expect(screen.getByRole('button', { name: '出牌' })).toBeDisabled();
    expect(screen.getByRole('button', { name: '不出' })).toBeDisabled();
  });

  it('enables play and calls onPlay when it is my turn', async () => {
    const onPlay = vi.fn();
    const user = userEvent.setup();
    render(<ActionBar myTurn={true} canPass={false} onPlay={onPlay} onPass={() => {}} />);
    await user.click(screen.getByRole('button', { name: '出牌' }));
    expect(onPlay).toHaveBeenCalled();
  });

  it('disables pass when canPass is false', () => {
    render(<ActionBar myTurn={true} canPass={false} onPlay={() => {}} onPass={() => {}} />);
    expect(screen.getByRole('button', { name: '不出' })).toBeDisabled();
  });
});
