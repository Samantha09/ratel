import { useState } from 'react';

interface MockRoom {
  code: string;
  name: string;
  players: number;
  state: 'open' | 'playing' | 'full';
}

// 本地演示用 mock 房间列表(后端无房间列表 API;真实加入会落回 PVE + AI 补位)。
const MOCK_ROOMS: MockRoom[] = [
  { code: '7351', name: '高手过招', players: 2, state: 'open' },
  { code: '2890', name: '欢乐场', players: 1, state: 'open' },
  { code: '4117', name: '新手村', players: 3, state: 'playing' },
  { code: '8804', name: '土豪局', players: 3, state: 'full' },
];

export interface RoomManagerProps {
  onJoin: (code: string) => void;
  onCreate: () => void;
  onBack: () => void;
}

/** 房间管理(mock):4 位房号加入 / 房间列表 / 创建。对应原型 ovRoom。 */
export function RoomManager({ onJoin, onCreate, onBack }: RoomManagerProps) {
  const [code, setCode] = useState('');

  const submit = () => {
    if (/^\d{4}$/.test(code)) onJoin(code);
  };

  return (
    <div className="overlay">
      <div className="panel" style={{ textAlign: 'left' }}>
        <div className="eyebrow" style={{ textAlign: 'center' }}>
          房 间 管 理
        </div>
        <h2 style={{ fontSize: 28, textAlign: 'center', marginBottom: 6 }}>玩家对战</h2>
        <p style={{ textAlign: 'center', marginTop: 8 }}>
          加入房间与其他玩家匹配对战。人数不足时由 AI 自动补位(本地演示)。
        </p>

        <div className="join-bar">
          <input
            className="room-input"
            maxLength={4}
            inputMode="numeric"
            placeholder="输入4位房间号"
            value={code}
            onChange={(e) => setCode(e.target.value.replace(/\D/g, '').slice(0, 4))}
            onKeyDown={(e) => e.key === 'Enter' && submit()}
          />
          <button type="button" className="btn btn-ghost" style={{ height: 48, padding: '0 22px' }} onClick={submit}>
            加入
          </button>
        </div>

        <div className="room-list">
          {MOCK_ROOMS.map((r) => {
            const open = r.state === 'open';
            const lbl = open ? '可加入' : r.state === 'playing' ? '游戏中' : '已满员';
            return (
              <div key={r.code} className="room-card">
                <div className="info">
                  <h4>{r.name}</h4>
                  <span>
                    房号 {r.code} · {r.players}/3 人
                  </span>
                </div>
                <div className="meta">
                  <span className={`room-state ${r.state}`}>
                    <span className="led" />
                    {lbl}
                  </span>
                  <button type="button" className="btn-mini go" disabled={!open} onClick={() => open && onJoin(r.code)}>
                    {open ? '加入' : '—'}
                  </button>
                </div>
              </div>
            );
          })}
        </div>

        <div className="cta-row" style={{ marginTop: 20 }}>
          <button type="button" className="btn btn-primary" onClick={onCreate}>
            + 创建房间
          </button>
          <button type="button" className="btn btn-ghost" style={{ height: 44 }} onClick={onBack}>
            返回主菜单
          </button>
        </div>
      </div>
    </div>
  );
}
