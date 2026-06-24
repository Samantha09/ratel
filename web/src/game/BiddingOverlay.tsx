import { Overlay } from '../components/Overlay';
import { Button } from '../components/Button';

export interface BiddingOverlayProps {
  onGrab: (g: boolean) => void;
}

export function BiddingOverlay({ onGrab }: BiddingOverlayProps) {
  return (
    <Overlay>
      <h2 className="font-display text-xl font-semibold text-ink">抢地主</h2>
      <p className="mt-1 text-sm text-ink-subtle">是否抢做地主？地主多得 3 张底牌。</p>
      <div className="mt-4 flex gap-3">
        <Button variant="secondary" className="flex-1" onClick={() => onGrab(false)}>不抢</Button>
        <Button className="flex-1" onClick={() => onGrab(true)}>抢地主</Button>
      </div>
    </Overlay>
  );
}
