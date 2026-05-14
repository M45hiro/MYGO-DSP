import React, { useState, useEffect, useRef, useCallback } from 'react';
import { Tabs } from 'antd';
import ControlPanel from './components/ControlPanel';
import WaveChart from './components/WaveChart';
import SpectrumChart from './components/SpectrumChart';
import PoleZeroChart from './components/PoleZeroChart';
import { WaveParams, FIRParams, IIRParams } from './types/dsp';

const SAMPLE_RATE = 44100;
const BUFFER_SIZE = 128;

const App: React.FC = () => {
  const [waves, setWaves] = useState<{ id: number; params: WaveParams }[]>([]);
  const [filters, setFilters] = useState<{ id: number; type: 'FIR' | 'IIR'; params: FIRParams | IIRParams; bypass: boolean }[]>([]);
  const [outputData, setOutputData] = useState<number[]>([]);
  const [isPlaying, setIsPlaying] = useState(false);
  const [initialized, setInitialized] = useState(false);
  const processRef = useRef<number | null>(null);
  const [poles, setPoles] = useState<{real:number,imag:number}[]>([]);
  const [zeros, setZeros] = useState<{real:number,imag:number}[]>([]);
  const [refreshPZ, setRefreshPZ] = useState(0);

  const triggerRefreshPZ = useCallback(() => setRefreshPZ(n => n + 1), []);

  useEffect(() => {
    if (!window.dsp || filters.length === 0) { setPoles([]); setZeros([]); return; }
    let allPoles: {real:number,imag:number}[] = [];
    let allZeros: {real:number,imag:number}[] = [];
    let pending = filters.length;
    let done = 0;
    filters.forEach(f => {
      window.dsp.getPoleZero(f.id).then(r => {
        if (r) {
          if (r.poles) allPoles = allPoles.concat(r.poles);
          if (r.zeros) allZeros = allZeros.concat(r.zeros);
        }
        done++;
        if (done === pending) { setPoles(allPoles); setZeros(allZeros); }
      }).catch(() => {
        done++;
        if (done === pending) { setPoles(allPoles); setZeros(allZeros); }
      });
    });
  }, [filters, refreshPZ]);

  useEffect(() => {
    if (!initialized && window.dsp) {
      window.dsp.init(SAMPLE_RATE, BUFFER_SIZE).then(() => setInitialized(true)).catch(console.error);
    }
  }, [initialized]);

  const processLoop = useCallback(async () => {
    if (!isPlaying || !window.dsp) return;
    try {
      const data = await window.dsp.process();
      if (data && data.length > 0) {
        setOutputData(Array.from(data));
      }
    } catch (e) {
      console.error('Process error:', e);
    }
    processRef.current = window.setTimeout(processLoop, 50);
  }, [isPlaying]);

  useEffect(() => {
    if (isPlaying) {
      processLoop();
    }
    return () => {
      if (processRef.current) clearTimeout(processRef.current);
    };
  }, [isPlaying, processLoop]);

  const togglePlay = () => setIsPlaying(!isPlaying);

  return (
    <div style={{ height: '100vh', display: 'flex', flexDirection: 'column', background: 'linear-gradient(135deg, #1a1a2e 0%, #16213e 50%, #0f3460 100%)' }}>
      {/* macOS Title Bar */}
      <div className="mac-titlebar">
        <div className="traffic-lights">
          <button className="traffic-light traffic-close" title="Close" onClick={() => { if (window.electronAPI) window.electronAPI.close(); else window.close(); }}>
            <span className="icon">✕</span>
          </button>
          <button className="traffic-light traffic-minimize" title="Minimize" onClick={() => { if (window.electronAPI) window.electronAPI.minimize(); }}>
            <span className="icon">─</span>
          </button>
          <button className="traffic-light traffic-zoom" title="Zoom" onClick={() => { if (window.electronAPI) window.electronAPI.maximize(); }}>
            <span className="icon">+</span>
          </button>
        </div>
        <div className="title-text">MYGO-DSP</div>
      </div>

      {/* Main Content */}
      <div style={{ flex: 1, display: 'flex', overflow: 'hidden' }}>
        {/* Sidebar */}
        <div className="mac-sidebar" style={{ width: 250, display: 'flex', flexDirection: 'column', flexShrink: 0 }}>
          <ControlPanel
            waves={waves} filters={filters} isPlaying={isPlaying}
            onWavesChange={setWaves} onFiltersChange={(f: any) => { setFilters(f); triggerRefreshPZ(); }}
            onPlayToggle={togglePlay}
          />
        </div>

        {/* Chart Area */}
        <div style={{ flex: 1, display: 'flex', flexDirection: 'column', padding: 10, gap: 10, overflow: 'hidden' }}>
          <div className="chart-panel" style={{ flex: 1, display: 'flex', flexDirection: 'column', overflow: 'hidden', padding: 4 }}>
            <Tabs
              defaultActiveKey="waveform"
              tabBarStyle={{ paddingLeft: 8, marginBottom: 0 }}
              items={[
                { key: 'waveform', label: 'Time Domain', children: <div style={{ height: '100%' }}><WaveChart data={outputData} /></div> },
                { key: 'spectrum', label: 'Spectrum', children: <div style={{ height: '100%' }}><SpectrumChart data={outputData} /></div> },
                { key: 'polezero', label: 'Pole-Zero', children: <div style={{ height: '100%' }}><PoleZeroChart poles={poles} zeros={zeros} /></div> },
              ]}
              style={{ height: '100%', display: 'flex', flexDirection: 'column' }}
            />
          </div>
        </div>
      </div>
    </div>
  );
};

export default App;
