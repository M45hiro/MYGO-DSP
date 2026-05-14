export type WaveType = 'Sine' | 'Square' | 'Triangle' | 'Sawtooth' | 'WhiteNoise' | 'PinkNoise' | 'Pulse';
export type FIRWindow = 'Rectangular' | 'Hamming' | 'Hanning' | 'Blackman' | 'Kaiser';
export type FilterPass = 'LowPass' | 'HighPass' | 'BandPass' | 'BandStop';
export type IIRPrototype =
  'Butterworth' | 'ChebyshevI' | 'ChebyshevII' | 'Elliptic' | 'Bessel' | 'Legendre' |
  'RBJ_LowPass' | 'RBJ_HighPass' | 'RBJ_BandPass1' | 'RBJ_BandPass2' | 'RBJ_BandStop' |
  'RBJ_LowShelf' | 'RBJ_HighShelf' | 'RBJ_BandShelf' | 'RBJ_AllPass';
export interface WaveParams { type: number; frequency: number; amplitude: number; phase: number; dutyCycle: number; }
export interface FIRParams { order: number; window: number; cutoff: number; cutoff2: number; passType: number; }
export interface IIRParams { prototype: number; passType: number; order: number; cutoff: number; cutoff2: number; ripple: number; stopbandAtten: number; shelfGain: number; Q: number; }
export interface AutoDesignSpec {
  type: number; fs: number; fpass: number; fstop: number;
  fpass2?: number; fstop2?: number; Ap: number; As: number;
}
export interface AutoDesignResult {
  success: boolean; warning: string; useFIR: boolean;
  selectedProto: number; order: number;
  firOrder: number; firBeta: number;
}
export interface DSPApi {
  init(sampleRate: number, bufferSize: number): Promise<void>;
  addWave(params: WaveParams): Promise<number>;
  removeWave(id: number): Promise<void>;
  updateWave(id: number, params: Partial<WaveParams>): Promise<void>;
  addFilterFIR(params: FIRParams): Promise<number>;
  addFilterIIR(params: IIRParams): Promise<number>;
  removeFilter(id: number): Promise<void>;
  updateFilter(id: number, params: Partial<FIRParams | IIRParams>): Promise<void>;
  setBypass(id: number, bypass: boolean): Promise<void>;
  process(): Promise<Float64Array>;
  getCounts(): Promise<{waves: number, filters: number}>;
  autoDesign(spec: AutoDesignSpec): Promise<AutoDesignResult>;
  getPoleZero(fid: number): Promise<{poles: {real:number,imag:number}[], zeros: {real:number,imag:number}[]}>;
}
export interface ElectronAPI {
  close: () => void;
  minimize: () => void;
  maximize: () => void;
}
declare global { interface Window { dsp: DSPApi; electronAPI?: ElectronAPI; } }
