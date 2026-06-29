import { ButtonHTMLAttributes } from 'react';

type Variant = 'primary' | 'ghost' | 'gold';
type Size = 'md' | 'lg';

export interface ButtonProps extends ButtonHTMLAttributes<HTMLButtonElement> {
  variant?: Variant;
  size?: Size;
}

const variantClass: Record<Variant, string> = {
  primary: 'btn-primary',
  ghost: 'btn-ghost',
  gold: 'btn-gold',
};

/**
 * PlayStation-style pill button. Visual rules live in index.css (`.btn`, `.btn-*`);
 * `size` is accepted for call-site compatibility but the prototype uses one height.
 */
export function Button({ variant = 'primary', size: _size = 'md', className = '', ...rest }: ButtonProps) {
  return <button {...rest} className={`btn ${variantClass[variant]} ${className}`.trim()} />;
}
