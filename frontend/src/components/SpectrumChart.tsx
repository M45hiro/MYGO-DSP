import React from 'react';
import ReactEChartsCore from 'echarts-for-react/lib/core';
import * as echarts from 'echarts';

interface Props {
  data: number[];
}

function computeSpectrum(data: number[], sr: number): { freq: number[]; mag: number[] } {
  const n = data.length;
  if (n < 2) return { freq: [], mag: [] };
  const fftSize = 256;
  const N = fftSize;

  let re = new Float64Array(N);
  let im = new Float64Array(N);

  for (let i = 0; i < Math.min(n, N); i++) {
    const w = 0.5 * (1 - Math.cos(2 * Math.PI * i / Math.min(n, N)));
    re[i] = data[i] * w;
    im[i] = 0;
  }

  for (let i = 1, j = 0; i < N; i++) {
    let b = N >> 1;
    for (; j & b; b >>= 1) j ^= b;
    j ^= b;
    if (i < j) {
      let tmp = re[i]; re[i] = re[j]; re[j] = tmp;
      tmp = im[i]; im[i] = im[j]; im[j] = tmp;
    }
  }

  for (let len = 2; len <= N; len <<= 1) {
    const wlenR = Math.cos(-2 * Math.PI / len);
    const wlenI = Math.sin(-2 * Math.PI / len);
    for (let i = 0; i < N; i += len) {
      let wr = 1, wi = 0;
      for (let j = 0; j < len / 2; j++) {
        const ur = re[i + j], ui = im[i + j];
        const vr = re[i + j + len / 2] * wr - im[i + j + len / 2] * wi;
        const vi = re[i + j + len / 2] * wi + im[i + j + len / 2] * wr;
        re[i + j] = ur + vr;
        im[i + j] = ui + vi;
        re[i + j + len / 2] = ur - vr;
        im[i + j + len / 2] = ui - vi;
        const tmp = wr;
        wr = tmp * wlenR - wi * wlenI;
        wi = tmp * wlenI + wi * wlenR;
      }
    }
  }

  const nb = N / 2;
  let maxMag = 0;
  const fc: number[] = [], mg: number[] = [];
  for (let i = 0; i < nb; i++) {
    const m = Math.sqrt(re[i] * re[i] + im[i] * im[i]) / N;
    if (m > maxMag) maxMag = m;
    fc.push(sr * i / N);
    mg.push(m);
  }

  const limit = 1024;
  if (nb > limit) {
    const step = Math.floor(nb / limit);
    const df: number[] = [], dm: number[] = [];
    for (let i = 0; i < nb; i += step) { df.push(fc[i]); dm.push(mg[i]); }
    return { freq: df, mag: dm };
  }
  return { freq: fc, mag: mg };
}

const SpectrumChart: React.FC<Props> = ({ data }) => {
  const sr = 44100;
  const { freq, mag } = React.useMemo(() => computeSpectrum(data, sr), [data]);
  const maxMag = Math.max(...mag, 1e-10);
  const magDb = mag.map(m => Math.max(20 * Math.log10(Math.max(m / maxMag, 1e-10)), -80));

  const option = {
    backgroundColor: 'transparent',
    animation: false,
    grid: { left: 55, right: 10, top: 10, bottom: 22 },
    xAxis: {
      type: 'value',
      name: 'f (kHz)',
      nameTextStyle: { fontSize: 10, color: '#86868b' },
      axisLine: { lineStyle: { color: 'rgba(0,0,0,0.15)' } },
      axisLabel: { fontSize: 10, color: '#86868b', formatter: (v: number) => (v / 1000).toFixed(1) },
      splitLine: { lineStyle: { color: 'rgba(0,0,0,0.06)' } },
    },
    yAxis: {
      type: 'value', min: -80, max: 5,
      name: 'dB',
      nameTextStyle: { fontSize: 10, color: '#86868b' },
      axisLine: { show: false },
      axisLabel: { fontSize: 10, color: '#86868b' },
      splitLine: { lineStyle: { color: 'rgba(0,0,0,0.06)' } },
    },
    series: [{
      type: 'line', data: freq.map((f, i) => [f, magDb[i]]),
      showSymbol: false, lineStyle: { color: '#30d158', width: 1.5 },
      areaStyle: { color: 'rgba(48, 209, 88, 0.15)' },
    }],
  };

  return <ReactEChartsCore echarts={echarts} theme="mac" option={option} style={{ height: '100%', width: '100%' }} />;
};

export default SpectrumChart;
