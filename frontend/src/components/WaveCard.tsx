import React from 'react';
import { WAVE_TYPES } from '../constants';
import { WaveParams } from '../types/dsp';

interface Props {
  id: number;
  params: WaveParams;
  onDelete: (id: number) => void;
  onUpdate: (id: number, params: Partial<WaveParams>) => void;
}

const MacField: React.FC<{ label: string; children: React.ReactNode }> = ({ label, children }) => (
  <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', minHeight: 24 }}>
    <span className="mac-label">{label}</span>
    {children}
  </div>
);

const WaveCard: React.FC<Props> = ({ id, params, onDelete, onUpdate }) => {
  return (
    <div className="mac-card" style={{ padding: '8px 10px', margin: '0 6px 6px' }}>
      <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: 6 }}>
        <span style={{ fontSize: 12, fontWeight: 600 }}>Wave #{id}</span>
        <button className="traffic-light traffic-close" style={{ width: 16, height: 16 }} onClick={() => onDelete(id)} title="Delete">
          <span className="icon" style={{ fontSize: 9, opacity: 1 }}>✕</span>
        </button>
      </div>
      <div style={{ display: 'flex', flexDirection: 'column', gap: 3 }}>
        <MacField label="Type">
          <select className="mac-select" value={params.type} onChange={(e) => onUpdate(id, { type: +e.target.value })} style={{ width: 130 }}>
            {WAVE_TYPES.map(t => <option key={t.value} value={t.value}>{t.label.split('(')[0].trim()}</option>)}
          </select>
        </MacField>
        <MacField label="Freq">
          <input className="mac-input" type="number" value={params.frequency} min={1} max={96000}
            onChange={(e) => onUpdate(id, { frequency: +e.target.value })} style={{ width: 90 }} />
        </MacField>
        <MacField label="Amp">
          <input className="mac-input" type="number" value={params.amplitude} min={0} max={1} step={0.05}
            onChange={(e) => onUpdate(id, { amplitude: +e.target.value })} style={{ width: 90 }} />
        </MacField>
        <MacField label="Phase°">
          <input className="mac-input" type="number" value={params.phase} min={0} max={360}
            onChange={(e) => onUpdate(id, { phase: +e.target.value })} style={{ width: 90 }} />
        </MacField>
        <MacField label="Duty%">
          <input className="mac-input" type="number" value={Math.round(params.dutyCycle * 100)} min={0} max={100}
            onChange={(e) => onUpdate(id, { dutyCycle: +e.target.value / 100 })} style={{ width: 90 }} />
        </MacField>
      </div>
    </div>
  );
};

export default WaveCard;
