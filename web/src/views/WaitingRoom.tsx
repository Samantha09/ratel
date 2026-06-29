import { useEffect, useRef, useState } from 'react';

export interface WaitingRoomProps {
  code: string;
  onReady: () => void;
  onLeave: () => void;
}

/** 等待室:座位填充动画,完成后落回 createRoomPve(AI 补位)。对应原型 ovWait。 */
export function WaitingRoom({ code, onReady, onLeave }: WaitingRoomProps) {
  const [filled, setFilled] = useState(0); // 已就位的对手数 0..2
  const onReadyRef = useRef(onReady);
  onReadyRef.current = onReady;

  useEffect(() => {
    const t1 = setTimeout(() => setFilled(1), 700);
    const t2 = setTimeout(() => setFilled(2), 1500);
    const t3 = setTimeout(() => onReadyRef.current(), 2400);
    return () => {
      clearTimeout(t1);
      clearTimeout(t2);
      clearTimeout(t3);
    };
  }, []);

  const seats = [
    { icon: '🧑‍✈️', name: '你', ready: true },
    { icon: '👾', name: filled >= 1 ? '贝塔' : '匹配中', ready: filled >= 1 },
    { icon: '🤖', name: filled >= 2 ? '阿尔法' : '匹配中', ready: filled >= 2 },
  ];
  const msg =
    filled >= 2 ? '玩家已就位，准备开始！(空位由 AI 补位)' : filled >= 1 ? '已加入 2 人，等待最后一位…' : '正在匹配玩家…';

  return (
    <div className="overlay">
      <div className="panel">
        <div className="eyebrow">等 待 玩 家</div>
        <h2 style={{ fontSize: 30 }}>
          房间 <span style={{ color: 'var(--primary)' }}>{code}</span>
        </h2>
        <div className="seats-wait">
          {seats.map((s, i) => (
            <div key={i} className={`seat-slot ${s.ready ? 'ready' : 'empty'}`}>
              <div className="av">{s.icon}</div>
              <small>{s.name}</small>
            </div>
          ))}
        </div>
        <p>{msg}</p>
        <div className="cta-row">
          <button type="button" className="btn btn-ghost" style={{ height: 44 }} onClick={onLeave}>
            离开房间
          </button>
        </div>
      </div>
    </div>
  );
}
