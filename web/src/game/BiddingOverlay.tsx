import { Overlay } from '../components/Overlay';
import { Button } from '../components/Button';

export interface BiddingOverlayProps {
  onGrab: (g: boolean) => void;
}

export function BiddingOverlay({ onGrab }: BiddingOverlayProps) {
  return (
    <Overlay>
      <div className="flex items-center gap-3">
        <div className="flex h-11 w-11 items-center justify-center rounded-lg bg-landlord/15 text-2xl">👑</div>
        <div>
          <h2 className="font-display text-xl font-semibold text-ink">抢地主</h2>
          <p className="text-xs text-ink-subtle">叫牌阶段</p>
        </div>
      </div>
      <p className="mt-4 text-sm text-ink-muted">
        是否抢做地主？地主独自对抗两名农民，多得 <span className="font-mono text-ink">3</span> 张底牌，赢牌得分翻倍。
      </p>
      <div className="mt-6 flex gap-3">
        <Button variant="secondary" size="lg" className="flex-1" onClick={() => onGrab(false)}>
          不抢
        </Button>
        <Button size="lg" className="flex-1" onClick={() => onGrab(true)}>
          抢地主
        </Button>
      </div>
    </Overlay>
  );
}
