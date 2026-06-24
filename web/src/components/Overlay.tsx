import { ReactNode } from 'react';

export interface OverlayProps {
  children: ReactNode;
}

export function Overlay({ children }: OverlayProps) {
  return (
    <div className="absolute inset-0 z-20 flex items-center justify-center bg-black/60 backdrop-blur-sm">
      <div className="w-full max-w-md rounded-xl border border-hairline bg-surface-1 p-6 shadow-2xl">
        {children}
      </div>
    </div>
  );
}
