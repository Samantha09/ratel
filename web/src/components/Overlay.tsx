import { ReactNode } from 'react';

export interface OverlayProps {
  children: ReactNode;
}

export function Overlay({ children }: OverlayProps) {
  return (
    <div
      role="dialog"
      aria-modal="true"
      className="absolute inset-0 z-40 flex animate-fade-in items-center justify-center bg-black/70 p-6 backdrop-blur-sm"
    >
      <div className="edge-highlight w-full max-w-md animate-scale-in rounded-xl border border-hairline-strong bg-surface-1 p-6 shadow-overlay">
        {children}
      </div>
    </div>
  );
}
