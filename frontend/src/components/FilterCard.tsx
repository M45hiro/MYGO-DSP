import React from 'react';
import { FIR_WINDOWS, FILTER_PASSES, IIR_PROTOTYPES } from '../constants';
import { FIRParams, IIRParams } from '../types/dsp';

interface Props {
  id: number;
  type: 'FIR' | 'IIR';
  params: FIRParams | IIRParams;
  bypass: boolean;
  onDelete: (id: number) => void;
  onUpdate: (id: number, params: Partial<FIRParams | IIRParams>) => void;
  onBypass: (id: number, bypass: boolean) => void;
}

const MacField: React.FC<{ label: string; children: React.ReactNode }> = ({ label, children }) => (
  <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', minHeight: 24 }}>
    <span className="mac-label">{label}</span>
    {children}
  </div>
);

const isFIR = (p: FIRParams | IIRParams): p is FIRParams => 'window' in p;
const isIIR = (p: FIRParams | IIRParams): p is IIRParams => 'prototype' in p;

const FilterCard: React.FC<Props> = ({ id, type, params, bypass, onDelete, onUpdate, onBypass }) => {
  const passType = 'passType' in params ? params.passType : 0;
  const showFc2 = passType === 2 || passType === 3;

  return (
    <div className="mac-card" style={{ padding: '8px 10px', margin: '0 6px 6px', opacity: bypass ? 0.55 : 1 }}>
      <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: 6 }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: 6 }}>
          <span style={{ fontSize: 12, fontWeight: 600 }}>{type} #{id}</span>
          <button
            className={`mac-switch ${!bypass ? 'on' : ''}`}
            onClick={() => onBypass(id, !bypass)}
            title={bypass ? 'Bypassed' : 'Active'}
          />
        </div>
        <button className="traffic-light traffic-close" style={{ width: 16, height: 16 }} onClick={() => onDelete(id)} title="Delete">
          <span className="icon" style={{ fontSize: 9, opacity: 1 }}>✕</span>
        </button>
      </div>
      <div style={{ display: 'flex', flexDirection: 'column', gap: 3 }}>
        {type === 'FIR' && isFIR(params) && (
          <>
            <MacField label="Pass">
              <select className="mac-select" value={params.passType} onChange={(e) => onUpdate(id, { passType: +e.target.value })} style={{ width: 120 }}>
                {FILTER_PASSES.map(p => <option key={p.value} value={p.value}>{p.label.split('(')[0].trim()}</option>)}
              </select>
            </MacField>
            <MacField label="Window">
              <select className="mac-select" value={params.window} onChange={(e) => onUpdate(id, { window: +e.target.value })} style={{ width: 120 }}>
                {FIR_WINDOWS.map(w => <option key={w.value} value={w.value}>{w.label}</option>)}
              </select>
            </MacField>
            <MacField label="Order">
              <input className="mac-input" type="number" value={params.order} min={1} max={200} onChange={(e) => onUpdate(id, { order: +e.target.value })} style={{ width: 80 }} />
            </MacField>
            <MacField label="Fc">
              <input className="mac-input" type="number" value={params.cutoff} min={1} max={96000} onChange={(e) => onUpdate(id, { cutoff: +e.target.value })} style={{ width: 80 }} />
            </MacField>
            {showFc2 && (
              <MacField label="Fc2">
                <input className="mac-input" type="number" value={params.cutoff2} min={1} max={96000} onChange={(e) => onUpdate(id, { cutoff2: +e.target.value })} style={{ width: 80 }} />
              </MacField>
            )}
          </>
        )}
        {type === 'IIR' && isIIR(params) && (
          <>
            <MacField label="Proto">
              <select className="mac-select" value={params.prototype} onChange={(e) => onUpdate(id, { prototype: +e.target.value })} style={{ width: 120 }}>
                {IIR_PROTOTYPES.map(p => <option key={p.value} value={p.value}>{p.label}</option>)}
              </select>
            </MacField>
            <MacField label="Pass">
              <select className="mac-select" value={params.passType} onChange={(e) => onUpdate(id, { passType: +e.target.value })} style={{ width: 120 }}>
                {FILTER_PASSES.map(p => <option key={p.value} value={p.value}>{p.label.split('(')[0].trim()}</option>)}
              </select>
            </MacField>
            <MacField label="Order">
              <input className="mac-input" type="number" value={params.order} min={1} max={20} onChange={(e) => onUpdate(id, { order: +e.target.value })} style={{ width: 80 }} />
            </MacField>
            <MacField label="Fc">
              <input className="mac-input" type="number" value={params.cutoff} min={1} max={96000} onChange={(e) => onUpdate(id, { cutoff: +e.target.value })} style={{ width: 80 }} />
            </MacField>
            {showFc2 && (
              <MacField label="Fc2">
                <input className="mac-input" type="number" value={params.cutoff2} min={1} max={96000} onChange={(e) => onUpdate(id, { cutoff2: +e.target.value })} style={{ width: 80 }} />
              </MacField>
            )}
            <MacField label="Ripple">
              <input className="mac-input" type="number" value={params.ripple} min={0.01} max={10} step={0.1} onChange={(e) => onUpdate(id, { ripple: +e.target.value })} style={{ width: 80 }} />
            </MacField>
            <MacField label="Stop dB">
              <input className="mac-input" type="number" value={params.stopbandAtten} min={1} max={100} onChange={(e) => onUpdate(id, { stopbandAtten: +e.target.value })} style={{ width: 80 }} />
            </MacField>
            <MacField label="Gain">
              <input className="mac-input" type="number" value={params.shelfGain} min={-60} max={60} onChange={(e) => onUpdate(id, { shelfGain: +e.target.value })} style={{ width: 80 }} />
            </MacField>
            <MacField label="Q">
              <input className="mac-input" type="number" value={params.Q} min={0.01} max={100} step={0.01} onChange={(e) => onUpdate(id, { Q: +e.target.value })} style={{ width: 80 }} />
            </MacField>
          </>
        )}
      </div>
    </div>
  );
};

export default FilterCard;
