import React, { useState } from 'react';
import { Modal, message } from 'antd';
import WaveCard from './WaveCard';
import FilterCard from './FilterCard';
import { WaveParams, FIRParams, IIRParams } from '../types/dsp';

const DEFAULT_WAVE: WaveParams = { type: 0, frequency: 440, amplitude: 0.5, phase: 0, dutyCycle: 0.5 };
const DEFAULT_FIR: FIRParams = { order: 64, window: 0, cutoff: 1000, cutoff2: 3000, passType: 0 };
const DEFAULT_IIR: IIRParams = {
  prototype: 0, passType: 0, order: 4, cutoff: 1000, cutoff2: 3000,
  ripple: 0.5, stopbandAtten: 60, shelfGain: 0, Q: 0.707,
};

interface Props {
  waves: { id: number; params: WaveParams }[];
  filters: { id: number; type: 'FIR' | 'IIR'; params: FIRParams | IIRParams; bypass: boolean }[];
  isPlaying: boolean;
  onWavesChange: (waves: { id: number; params: WaveParams }[]) => void;
  onFiltersChange: (filters: { id: number; type: 'FIR' | 'IIR'; params: FIRParams | IIRParams; bypass: boolean }[]) => void;
  onPlayToggle: () => void;
}

const IIR_PROTO_NAMES = ['Butterworth','Chebyshev I','Chebyshev II','Elliptic','Bessel','Legendre',
  'RBJ LP','RBJ HP','RBJ BP1','RBJ BP2','RBJ BS','RBJ LShelf','RBJ HShelf','RBJ BShelf','RBJ AP'];

const ControlPanel: React.FC<Props> = ({ waves, filters, isPlaying, onWavesChange, onFiltersChange, onPlayToggle }) => {
  const [designOpen, setDesignOpen] = useState(false);
  const [designSpec, setDesignSpec] = useState({ type: 0, fpass: 1000, fstop: 2000, Ap: 1, As: 40, fs: 44100, fpass2: 4000, fstop2: 5000 });
  const [designing, setDesigning] = useState(false);
  const [designResult, setDesignResult] = useState<any>(null);

  const handleAutoDesign = async () => {
    if (!window.dsp) return;
    setDesigning(true); setDesignResult(null);
    try {
      const result = await window.dsp.autoDesign(designSpec);
      setDesignResult(result);
      if (result.success && !result.useFIR) message.success(`Auto design: ${IIR_PROTO_NAMES[result.selectedProto] ?? 'Unknown'} order=${result.order}`);
      else if (result.success) message.info(`Auto design: Kaiser FIR order=${result.firOrder}`);
      else message.warning(result.warning || 'Design failed');
    } catch (e: any) { message.error('Design error: ' + e.message); }
    setDesigning(false);
  };

  const addDesignedFilter = async () => {
    if (!designResult || !designResult.success || !window.dsp) return;
    try {
      if (designResult.useFIR) {
        const params: FIRParams = {
          order: designResult.firOrder,
          window: 4,
          cutoff: designSpec.fpass,
          cutoff2: designSpec.fpass2 || designSpec.fpass * 2,
          passType: designSpec.type,
        };
        const id = await window.dsp.addFilterFIR(params);
        onFiltersChange([...filters, { id, type: 'FIR', params, bypass: false }]);
        message.success('FIR filter added');
      } else {
        const params: IIRParams = {
          prototype: designResult.selectedProto,
          passType: designSpec.type,
          order: designResult.order,
          cutoff: designSpec.fpass,
          cutoff2: designSpec.fpass2 || designSpec.fpass * 2,
          ripple: designSpec.Ap,
          stopbandAtten: designSpec.As,
          shelfGain: 1.0,
          Q: 0.707,
        };
        const id = await window.dsp.addFilterIIR(params);
        onFiltersChange([...filters, { id, type: 'IIR', params, bypass: false }]);
        message.success('IIR filter added');
      }
      setDesignOpen(false);
      setDesignResult(null);
    } catch (e: any) {
      message.error('Failed to add filter: ' + e.message);
    }
  };

  const addWave = async () => {
    if (!window.dsp) return;
    const id = await window.dsp.addWave(DEFAULT_WAVE);
    onWavesChange([...waves, { id, params: { ...DEFAULT_WAVE } }]);
  };

  const deleteWave = async (id: number) => {
    onWavesChange(waves.filter(w => w.id !== id));
    try { await window.dsp.removeWave(id); } catch {}
  };

  const updateWave = async (id: number, partial: Partial<WaveParams>) => {
    const wave = waves.find(w => w.id === id);
    if (!wave) return;
    const newParams = { ...wave.params, ...partial };
    onWavesChange(waves.map(w => w.id === id ? { ...w, params: newParams } : w));
    try { await window.dsp.updateWave(id, newParams); } catch (e) { console.error('updateWave error:', e); }
  };

  const addFilter = async (type: 'FIR' | 'IIR') => {
    if (!window.dsp) return;
    if (type === 'FIR') {
      const id = await window.dsp.addFilterFIR(DEFAULT_FIR);
      onFiltersChange([...filters, { id, type: 'FIR', params: { ...DEFAULT_FIR }, bypass: false }]);
    } else {
      const id = await window.dsp.addFilterIIR(DEFAULT_IIR);
      onFiltersChange([...filters, { id, type: 'IIR', params: { ...DEFAULT_IIR }, bypass: false }]);
    }
  };

  const deleteFilter = async (id: number) => {
    onFiltersChange(filters.filter(f => f.id !== id));
    try { await window.dsp.removeFilter(id); } catch {}
  };

  const updateFilter = async (id: number, partial: Partial<FIRParams | IIRParams>) => {
    const filter = filters.find(f => f.id === id);
    if (!filter) return;
    const newParams = { ...filter.params, ...partial } as FIRParams | IIRParams;
    onFiltersChange(filters.map(f => f.id === id ? { ...f, params: newParams } : f));
    try { await window.dsp.updateFilter(id, newParams); } catch {}
  };

  const toggleBypass = (id: number, bypass: boolean) => {
    onFiltersChange(filters.map(f => f.id === id ? { ...f, bypass } : f));
    setTimeout(() => window.dsp.setBypass(id, bypass).catch(() => {}), 0);
  };

  return (
    <div style={{ height: '100%', display: 'flex', flexDirection: 'column' }}>
      {/* Wave Sources Section */}
      <div className="mac-section-title" style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
        <span>Wave Sources</span>
        <button className="mac-btn" style={{ padding: '2px 10px', fontSize: 11 }} onClick={addWave}>+ Add</button>
      </div>
      <div className="mac-scroll" style={{ maxHeight: '40%' }}>
        {waves.map(w => (
          <WaveCard key={w.id} id={w.id} params={w.params} onDelete={deleteWave} onUpdate={updateWave} />
        ))}
        {waves.length === 0 && <div style={{ padding: '8px 14px', fontSize: 12, color: '#86868b' }}>No wave sources</div>}
      </div>

      {/* Filters Section */}
      <div className="mac-section-title" style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
        <span>Filters</span>
        <div style={{ display: 'flex', gap: 4 }}>
          <button className="mac-btn" style={{ padding: '2px 10px', fontSize: 11 }} onClick={() => addFilter('IIR')}>+ IIR</button>
          <button className="mac-btn" style={{ padding: '2px 10px', fontSize: 11 }} onClick={() => addFilter('FIR')}>+ FIR</button>
        </div>
      </div>
      <div className="mac-scroll" style={{ maxHeight: '35%' }}>
        {filters.map(f => (
          <FilterCard
            key={f.id} id={f.id} type={f.type} params={f.params}
            bypass={f.bypass} onDelete={deleteFilter} onUpdate={updateFilter} onBypass={toggleBypass}
          />
        ))}
        {filters.length === 0 && <div style={{ padding: '8px 14px', fontSize: 12, color: '#86868b' }}>No filters</div>}
      </div>

      {/* Smart Design Button */}
      <div style={{ padding: '4px 14px' }}>
        <button className="mac-btn-ghost" style={{ width: '100%', padding: '5px', fontSize: 11, borderRadius: 6 }} onClick={() => setDesignOpen(true)}>
          ⚡ Smart Design
        </button>
      </div>

      {/* Auto Design Modal */}
      <Modal
        title={<span style={{ fontFamily: 'var(--mac-font)', fontWeight: 600, fontSize: 14 }}>Auto Filter Designer</span>}
        open={designOpen}
        onCancel={() => { setDesignOpen(false); setDesignResult(null); }}
        footer={null}
        width={380}
        style={{ fontFamily: 'var(--mac-font)' }}
      >
        <div style={{ display: 'flex', flexDirection: 'column', gap: 8 }}>
          <FieldRow label="Type">
            <select className="mac-select" value={designSpec.type} onChange={(e) => { const t = +e.target.value; setDesignSpec({...designSpec, type: t, fpass2: t >= 2 ? 4000 : 0, fstop2: t >= 2 ? 5000 : 0 }); }} style={{width:160}}>
              <option value={0}>Low Pass</option>
              <option value={1}>High Pass</option>
              <option value={2}>Band Pass</option>
              <option value={3}>Band Stop</option>
            </select>
          </FieldRow>
          <FieldRow label="Cutoff Freq (Hz)">
            <input className="mac-input" type="number" value={designSpec.fpass} min={1} max={96000} onChange={(e) => setDesignSpec({...designSpec, fpass: +e.target.value})} style={{width:120}} />
          </FieldRow>
          {designSpec.type >= 2 && (
            <FieldRow label="Cutoff Freq 2 (Hz)">
              <input className="mac-input" type="number" value={designSpec.fpass2} min={1} max={96000} onChange={(e) => setDesignSpec({...designSpec, fpass2: +e.target.value})} style={{width:120}} />
            </FieldRow>
          )}
          <FieldRow label="Pass Ripple (dB)">
            <input className="mac-input" type="number" value={designSpec.Ap} min={0.01} max={10} step={0.1} onChange={(e) => setDesignSpec({...designSpec, Ap: +e.target.value})} style={{width:120}} />
          </FieldRow>
          <FieldRow label="Stop Atten (dB)">
            <input className="mac-input" type="number" value={designSpec.As} min={1} max={120} onChange={(e) => setDesignSpec({...designSpec, As: +e.target.value})} style={{width:120}} />
          </FieldRow>
          <button className="mac-btn" style={{ width: '100%', marginTop: 8, justifyContent: 'center' }} disabled={designing} onClick={handleAutoDesign}>
            {designing ? 'Designing...' : 'Run Auto Design'}
          </button>
          {designResult && (
            <div style={{ background: 'rgba(0,0,0,0.04)', borderRadius: 8, padding: 10, marginTop: 4 }}>
              {designResult.success ? (
                <>
                  <div style={{ color: '#30d158', fontWeight: 600, fontSize: 12, marginBottom: 4 }}>✓ Design Succeeded</div>
                  {designResult.useFIR
                    ? <div style={{ fontSize: 12 }}>Kaiser FIR: order={designResult.firOrder}, β={designResult.firBeta.toFixed(2)}</div>
                    : <div style={{ fontSize: 12 }}>{IIR_PROTO_NAMES[designResult.selectedProto] ?? 'IIR'}: order={designResult.order}</div>
                  }
                  {designResult.warning && <div style={{ fontSize: 11, color: '#86868b', marginTop: 4 }}>ℹ {designResult.warning}</div>}
                </>
              ) : (
                <div style={{ color: '#ff453a', fontSize: 12 }}>✗ {designResult.warning || 'Design failed'}</div>
              )}
              {designResult.success && (
                <button className="mac-btn" style={{ width: '100%', marginTop: 6, justifyContent: 'center' }} onClick={addDesignedFilter}>
                  + Add This Filter
                </button>
              )}
            </div>
          )}
        </div>
      </Modal>

      {/* Play Button */}
      <div style={{ marginTop: 'auto', padding: '8px 14px' }}>
        <button className={`mac-play-btn ${isPlaying ? 'stop' : ''}`} style={{ width: '100%' }} onClick={onPlayToggle}>
          {isPlaying ? '⏹ Stop' : '▶ Play'}
        </button>
      </div>
    </div>
  );
};

const FieldRow: React.FC<{ label: string; children: React.ReactNode }> = ({ label, children }) => (
  <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
    <span style={{ fontSize: 12, color: '#86868b' }}>{label}</span>
    {children}
  </div>
);

export default ControlPanel;
