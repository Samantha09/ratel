import { ReactNode } from 'react';

export interface OverlayProps {
  children: ReactNode;
  /** Extra class on the panel, e.g. `result`. */
  panelClassName?: string;
}

export function Overlay({ children, panelClassName = '' }: OverlayProps) {
  return (
    <div role="dialog" aria-modal="true" className="overlay">
      <div className={`panel ${panelClassName}`.trim()}>{children}</div>
    </div>
  );
}
