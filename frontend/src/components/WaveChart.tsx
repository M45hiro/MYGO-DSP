import React from 'react';
import ReactEChartsCore from 'echarts-for-react/lib/core';
import * as echarts from 'echarts';

interface Props {
  data: number[];
}

const macChartTheme = {
  backgroundColor: 'transparent',
  textStyle: { fontFamily: "-apple-system, BlinkMacSystemFont, 'Inter', 'Helvetica Neue', sans-serif" },
};

echarts.registerTheme('mac', macChartTheme);

const WaveChart: React.FC<Props> = ({ data }) => {
  const seriesData = React.useMemo(() => {
    if (!data || data.length === 0) return [];
    return data.map((v, i) => [i, v]);
  }, [data]);

  const option = {
    backgroundColor: 'transparent',
    animation: false,
    grid: { left: 45, right: 10, top: 10, bottom: 20 },
    xAxis: {
      type: 'value',
      axisLine: { lineStyle: { color: 'rgba(0,0,0,0.1)' } },
      axisLabel: { fontSize: 10, color: '#86868b' },
      splitLine: { lineStyle: { color: 'rgba(0,0,0,0.04)' } },
    },
    yAxis: {
      type: 'value',
      axisLine: { show: false },
      axisLabel: { fontSize: 10, color: '#86868b' },
      splitLine: { lineStyle: { color: 'rgba(0,0,0,0.04)' } },
    },
    series: [{
      type: 'line',
      data: seriesData,
      smooth: false,
      showSymbol: false,
      lineStyle: { color: '#0071e3', width: 1.5 },
      areaStyle: {
        color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
          { offset: 0, color: 'rgba(0, 113, 227, 0.15)' },
          { offset: 1, color: 'rgba(0, 113, 227, 0.01)' },
        ]),
      },
    }],
  };

  return <ReactEChartsCore echarts={echarts} theme="mac" option={option} style={{ height: '100%', width: '100%' }} />;
};

export default WaveChart;
