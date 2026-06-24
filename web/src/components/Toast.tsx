export interface ToastProps {
  text: string;
}

export function Toast({ text }: ToastProps) {
  return (
    <div className="absolute bottom-24 left-1/2 -translate-x-1/2 rounded-md border border-hairline bg-surface-2 px-3 py-1.5 text-sm text-ink shadow-lg">
      {text}
    </div>
  );
}
