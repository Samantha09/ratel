import { Overlay } from '../components/Overlay';

export interface BiddingOverlayProps {
  onGrab: (g: boolean) => void;
}

/**
 * 叫分阶段面板。后端只有 grab/不抢二元决策,1/2/3 分为视觉装饰:
 * 不叫 → onGrab(false);1/2/3 分 → onGrab(true)。
 */
export function BiddingOverlay({ onGrab }: BiddingOverlayProps) {
  return (
    <Overlay>
      <div className="eyebrow">叫 分 阶 段</div>
      <h2 style={{ fontSize: 30 }}>是否叫地主？</h2>
      <p>叫分越高，赢牌得分越高。地主独得三张底牌，单挑两位农民。</p>
      <div className="cta-row bid">
        <button type="button" className="btn btn-ghost" onClick={() => onGrab(false)}>
          不叫
        </button>
        <button type="button" className="btn btn-ghost" onClick={() => onGrab(true)}>
          1 分
        </button>
        <button type="button" className="btn btn-ghost" onClick={() => onGrab(true)}>
          2 分
        </button>
        <button type="button" className="btn btn-primary" onClick={() => onGrab(true)}>
          3 分
        </button>
      </div>
    </Overlay>
  );
}
