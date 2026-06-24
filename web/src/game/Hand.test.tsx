import { describe, it, expect, vi } from 'vitest';
import { render, screen } from '@testing-library/react';
import userEvent from '@testing-library/user-event';
import { Hand } from './Hand';
import { Card } from '../types';

const c = (level: number, type = 1): Card => ({ type: type as Card['type'], level });

describe('Hand', () => {
  it('renders a card per hand entry', () => {
    render(<Hand hand={[c(0), c(1), c(2)]} selected={[]} onToggle={() => {}} />);
    expect(screen.getAllByRole('button').length).toBe(3);
  });

  it('calls onToggle with the index when a card is clicked', async () => {
    const onToggle = vi.fn();
    const user = userEvent.setup();
    render(<Hand hand={[c(0), c(5)]} selected={[]} onToggle={onToggle} />);
    await user.click(screen.getAllByRole('button')[1]);
    expect(onToggle).toHaveBeenCalledWith(1);
  });
});
