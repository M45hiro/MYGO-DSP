import React from 'react';
import ReactEChartsCore from 'echarts-for-react/lib/core';
import * as echarts from 'echarts';

interface Props {
  poles: { real: number; imag: number }[];
  zeros: { real: number; imag: number }[];
}

const PoleZeroChart: React.FC<Props> = ({ poles, zeros }) => {
  const hasData = poles.length > 0 || zeros.length > 0;
  const circle: [number, number][] = [];
  for (let i = 0; i <= 360; i++) {
    const a = (i * Math.PI) / 180;
    circle.push([Math.cos(a), Math.sin(a)]);
  }

  const option = {
    backgroundColor: 'transparent',
    grid: { left: 45, right: 20, top: 10, bottom: 25 },
    xAxis: {
      type: 'value', min: -1.5, max: 1.5,
      axisLine: { lineStyle: { color: 'rgba(0,0,0,0.1)' } },
      axisLabel: { fontSize: 10, color: '#86868b' },
      splitLine: { lineStyle: { color: 'rgba(0,0,0,0.04)' } },
    },
    yAxis: {
      type: 'value', min: -1.5, max: 1.5,
      axisLine: { lineStyle: { color: 'rgba(0,0,0,0.1)' } },
      axisLabel: { fontSize: 10, color: '#86868b' },
      splitLine: { lineStyle: { color: 'rgba(0,0,0,0.04)' } },
    },
    series: [
      {
        name: 'Unit Circle', type: 'line', data: circle,
        showSymbol: false, lineStyle: { color: 'rgba(0,0,0,0.15)', type: 'dashed', width: 1 },
        silent: true,
      },
      {
        name: 'Poles', type: 'scatter', data: poles.map(p => [p.real, p.imag]),
        symbol: 'x', symbolSize: 12, color: '#ff453a',
      },
      {
        name: 'Zeros', type: 'scatter', data: zeros.map(z => [z.real, z.imag]),
        symbol: 'circle', symbolSize: 10, color: '#30d158',
        symbolBorderColor: '#30d158', symbolBorderWidth: 2,
      },
    ],
    legend: { data: ['Poles', 'Zeros'], bottom: 0, textStyle: { fontSize: 10, color: '#86868b' } },
  };

  if (!hasData) {
    return (
      <div style={{ height: '100%', width: '100%', display: 'flex', flexDirection: 'column', alignItems: 'center', justifyContent: 'center', color: '#86868b' }}>
        <ReactEChartsCore echarts={echarts} theme="mac" option={option} style={{ height: '100%', width: '100%' }} />
        <div style={{ position: 'absolute', fontSize: 13 }}>Add a filter to see pole-zero plot</div>
      </div>
    );
  }
  return <ReactEChartsCore echarts={echarts} theme="mac" option={option} style={{ height: '100%', width: '100%' }} />;
};

export default PoleZeroChart;
