import { describe, it, expect } from 'vitest';
import { render, screen } from '@testing-library/react';
import { PlayingCard } from './PlayingCard';

describe('PlayingCard', () => {
  it('renders rank and suit', () => {
    render(<PlayingCard card={{ type: 4, level: 8 }} />); // ♥ J
    expect(screen.getByText('J')).toBeInTheDocument();
    expect(screen.getByText('♥')).toBeInTheDocument();
  });

  it('renders the big joker', () => {
    render(<PlayingCard card={{ type: 0, level: 14 }} />);
    expect(screen.getByText('大王')).toBeInTheDocument();
  });

  it('applies a selected class when selected', () => {
    const { container } = render(<PlayingCard card={{ type: 1, level: 0 }} selected />);
    expect(container.firstChild).toHaveClass('-translate-y-2');
  });
});
