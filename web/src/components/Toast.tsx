export interface ToastProps {
  text: string;
}

export function Toast({ text }: ToastProps) {
  return (
    <div
      role="status"
      aria-live="polite"
      className="absolute bottom-28 left-1/2 z-30 flex -translate-x-1/2 animate-toast-in items-center gap-2 rounded-lg border border-danger/40 bg-surface-2 px-3.5 py-2 text-sm text-ink shadow-overlay"
    >
      <span aria-hidden="true" className="text-danger">⚠</span>
      {text}
    </div>
  );
}
