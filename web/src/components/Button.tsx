import { ButtonHTMLAttributes } from 'react';

type Variant = 'primary' | 'secondary';
type Size = 'md' | 'lg';

export interface ButtonProps extends ButtonHTMLAttributes<HTMLButtonElement> {
  variant?: Variant;
  size?: Size;
}

const styles: Record<Variant, string> = {
  primary:
    'bg-primary text-on-primary shadow-card hover:bg-primary-hover active:bg-primary-focus active:translate-y-px',
  secondary:
    'bg-surface-1 text-ink border border-hairline hover:bg-surface-2 hover:border-hairline-strong active:translate-y-px',
};

const sizes: Record<Size, string> = {
  md: 'px-3.5 py-2 text-sm',
  lg: 'px-5 py-2.5 text-[15px]',
};

export function Button({ variant = 'primary', size = 'md', className = '', ...rest }: ButtonProps) {
  return (
    <button
      {...rest}
      className={`inline-flex items-center justify-center gap-2 rounded-md font-medium transition-all duration-150 focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-primary focus-visible:ring-offset-2 focus-visible:ring-offset-canvas disabled:pointer-events-none disabled:cursor-not-allowed disabled:opacity-40 ${sizes[size]} ${styles[variant]} ${className}`}
    />
  );
}
