# Modular Yielding Graphical Operators — MYGO-DSP
> **企业级数字信号处理与滤波器设计平台**

## 1. 项目概述

基于 C++17 + Electron + React 的实时数字信号处理平台，支持波形生成、多类型滤波器设计、实时频谱分析。采用 stdin/stdout JSON 协议的进程间通信架构。

### 技术架构

```
┌─────────────────────────────────────────────┐
│          前端 (Electron + React 19)           │
│  ControlPanel  WaveChart  SpectrumChart  PZ  │
│  (macOS 风格 UI + ECharts 可视化)             │
└─────────────────┬───────────────────────────┘
                  │ IPC (ipcMain/ipcRenderer)
┌─────────────────▼───────────────────────────┐
│          Electron Main Process               │
│   启动 DSP 引擎子进程 + 维护命令队列           │
└─────────────────┬───────────────────────────┘
                  │ stdin/stdout JSON
┌─────────────────▼───────────────────────────┐
│          C++17 DSP Engine                    │
│  波形生成 · FIR/IIR 滤波 · 智能设计 · FFT    │
│  (静态链接, 无运行时依赖)                      │
└─────────────────────────────────────────────┘
```

### 核心特性

| 模块 | 功能 |
|------|------|
| **波形发生器** | 正弦/方波/三角/锯齿/白噪声/粉红噪声/脉冲 |
| **实时控制** | 频率(1-96kHz)、幅度、相位、占空比、多波形叠加 |
| **FIR 滤波** | 5种窗函数 × 4种类型 (LP/HP/BP/BS) |
| **IIR 滤波** | 6种模拟原型 + 9种RBJ双二阶，阶数1-20 |
| **智能设计** | 自动原型选择 + 阶数估算 (巴特沃斯/切比雪夫/椭圆/FIR回退) |
| **实时分析** | 时域波形、FFT频谱(256点)、零极点图 |

### 技术栈

| 层级 | 技术 |
|------|------|
| UI 框架 | Electron 34 |
| 前端 | React 19 + TypeScript + Vite 6 |
| 图表 | ECharts 5 |
| 样式 | 原生 CSS (macOS Big Sur 风格) |
| DSP 核心 | C++17 (静态编译, 无可执行依赖) |
| IPC | stdin/stdout JSON 行协议 |
| 构建 | CMake + MinGW/GCC |

## 2. 项目结构

```
MYGO-DSP/
├── README.md                 # 本文件
├── PROJECT_PLAN.md           # 策划书
│
├── dsp-core/                 # C++ DSP 引擎
│   ├── CMakeLists.txt        # CMake 构建 (库+可执行文件+测试)
│   ├── include/              # 头文件
│   │   ├── DSPMath.h         # Pi/TwoPi 常量
│   │   ├── Buffer.h          # 环形缓冲区模板
│   │   ├── IOperator.h       # 算子抽象接口
│   │   ├── WaveformGenerator.h
│   │   ├── FIRFilter.h
│   │   ├── IIRFilter.h
│   │   └── AutoFilterDesigner.h
│   ├── src/                  # 源文件
│   │   ├── WaveformGenerator.cpp
│   │   ├── FIRFilter.cpp
│   │   ├── IIRFilter.cpp
│   │   ├── Engine.h/cpp      # 引擎调度器
│   │   ├── main.cpp          # IPC 入口
│   │   └── AutoFilterDesigner.cpp
│   ├── tests/                # 测试
│   │   ├── test_waveform.cpp
│   │   ├── test_fir.cpp
│   │   ├── test_iir.cpp
│   │   ├── test_engine.cpp
│   │   ├── test_auto_design.cpp
│   │   └── bench_engine.cpp
│   └── build/                # 构建输出
│       └── mygo_dsp_engine.exe
│
├── frontend/                 # Electron + React 前端
│   ├── package.json
│   ├── vite.config.ts
│   ├── index.html
│   ├── tsconfig.json
│   ├── electron/
│   │   ├── main.cjs          # Electron 主进程
│   │   └── preload.cjs       # 预加载脚本
│   ├── src/
│   │   ├── main.tsx          # React 入口
│   │   ├── App.tsx           # 根组件
│   │   ├── App.css           # macOS 风格样式
│   │   ├── constants.ts
│   │   ├── types/dsp.ts
│   │   └── components/
│   │       ├── ControlPanel.tsx
│   │       ├── WaveCard.tsx
│   │       ├── FilterCard.tsx
│   │       ├── WaveChart.tsx
│   │       ├── SpectrumChart.tsx
│   │       └── PoleZeroChart.tsx
│   └── release/              # 打包输出
│       └── MYGO-DSP-win32-x64/
│           └── MYGO-DSP.exe
│
└── report/                   # 开发报告
    ├── mygo-dsp-report.html
    └── mygo-dsp-report.pdf
```

## 3.  IPC 协议一览

请求格式 (stdin):
```json
{"id": 1, "cmd": "addWave", "type": 0, "frequency": 440, "amplitude": 0.5, "sampleRate": 44100}
```

响应格式 (stdout):
```json
{"id": 1, "status": "ok", "data": 0}
```

| 命令 | 说明 |
|------|------|
| `init` | 初始化引擎 |
| `addWave` | 添加波形 (返回波形ID) |
| `removeWave` | 删除波形 (wid) |
| `updateWave` | 更新波形参数 |
| `addFilterIIR` | 添加 IIR 滤波器 |
| `addFilterFIR` | 添加 FIR 滤波器 |
| `removeFilter` | 删除滤波器 (fid) |
| `updateFilter` | 更新滤波器参数 |
| `setBypass` | 设置旁路 (bid + bypass + target) |
| `process` | 处理一帧音频 (返回128个采样点) |
| `autoDesign` | 智能滤波器设计 |
| `getCounts` | 查询波形/滤波器数量 |
| `getPoleZero` | 获取滤波器零极点 |
| `exit` | 退出引擎 |

## 4. 构建指南

### 编译 DSP 引擎
```bash
cd dsp-core
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
```

### 构建前端
```bash
cd frontend
npm install
npx vite build
```

### 打包 Electron
```bash
cd frontend
# 构建 asar
npx --package @electron/asar asar pack dist+electron app.asar
# 复制到 Electron 目录
```

## 5. 开发阶段

| 阶段 | 内容 | 状态 |
|------|------|------|
| P1 | C++ DSP 核心 (波形/FIR/IIR/智能设计) | ✅ |
| P2 | IPC 协议 (main.cpp stdin/stdout JSON) | ✅ |
| P3 | Electron 主进程 + preload 桥接 | ✅ |
| P4 | React macOS 风格 UI | ✅ |
| P5 | ECharts 图表 (波形/频谱/零极点) | ✅ |
| P6 | Google Test + 性能基准 + 构建打包 | ✅ |

## 6. 测试

```bash
# Google Test (38 用例)
cd build
cmake .. -DBUILD_TESTS=ON
mingw32-make dsp_tests && ./dsp_tests

# 性能基准
cmake .. -DBUILD_BENCHMARKS=ON
mingw32-make dsp_benchmarks && ./dsp_benchmarks
```

## 7. 许可

MIT License
