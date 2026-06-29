import { describe, it, expect } from 'vitest';
import { render, screen } from '@testing-library/react';
import { PlayingCard } from './PlayingCard';

describe('PlayingCard', () => {
  it('renders rank and suit', () => {
    render(<PlayingCard card={{ type: 4, level: 8 }} />); // ♥ J
    // Rank shows in the corner index; suit appears in the corner and as the center pip.
    expect(screen.getAllByText('J').length).toBeGreaterThan(0);
    expect(screen.getAllByText('♥').length).toBeGreaterThan(0);
  });

  it('renders the big joker', () => {
    render(<PlayingCard card={{ type: 0, level: 14 }} />);
    expect(screen.getByText('JOKER')).toBeInTheDocument();
    expect(screen.getByText('大')).toBeInTheDocument();
  });

  it('applies a selected class when selected', () => {
    const { container } = render(<PlayingCard card={{ type: 1, level: 0 }} selected />);
    expect(container.firstChild).toHaveClass('selected');
  });
});
