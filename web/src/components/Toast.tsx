export interface ToastProps {
  text: string;
}

export function Toast({ text }: ToastProps) {
  return (
    <div role="status" aria-live="polite" className="toast show">
      {text}
    </div>
  );
}
