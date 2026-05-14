# MYGO-DSP — Modular Yielding Graphical Operators

**实时数字信号处理与滤波器设计平台**  
**Real-time Digital Signal Processing & Filter Design Platform**

[![C++17](https://img.shields.io/badge/C++-17-00599C)](https://en.cppreference.com/w/cpp/17)
[![Electron](https://img.shields.io/badge/Electron-34-47848F)](https://www.electronjs.org/)
[![React](https://img.shields.io/badge/React-19-61DAFB)](https://react.dev/)

---

## 中文 / Chinese

### 📌 项目简介

MYGO-DSP 是一个基于 C++17 + Electron + React 的企业级数字信号处理与滤波器设计平台。支持波形生成、FIR/IIR 滤波、实时频谱分析、智能滤波器设计等功能。

- **C++ DSP 引擎**: 七种波形发生器、FIR 窗函数法滤波、IIR 模拟原型法滤波、自动滤波器设计
- **IPC 通信**: stdin/stdout JSON 行协议，请求-响应模式
- **桌面 UI**: macOS Big Sur 风格，Electron + React 19 + ECharts 可视化

### ✅ 核心特性

| 模块 | 功能 |
|------|------|
| 波形发生器 | 正弦/方波/三角/锯齿/白噪声/粉红噪声/脉冲 |
| 实时控制 | 频率(1-96kHz)、幅度、相位、占空比、多波形叠加 |
| FIR 滤波 | 5种窗函数 × 4种类型 (LP/HP/BP/BS)，阶数 1-200 |
| IIR 滤波 | 6种模拟原型 + 9种RBJ双二阶，阶数 1-20 |
| 智能设计 | 自动原型选择 + 阶数估算 |
| 实时分析 | 时域波形、FFT频谱(256点)、零极点图 |

### 🔧 快速开始

#### 前置要求 / Prerequisites

| 工具 | 版本要求 |
|------|---------|
| CMake | ≥ 3.16 |
| MinGW/GCC | ≥ 8.1 (C++17) |
| Node.js | ≥ 18 |
| Git | - |

#### 编译 DSP 引擎 / Build DSP Engine

```bash
cd dsp-core
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
```

#### 运行前端 / Run Frontend

```bash
cd frontend
npm install
npx electron .
```

#### 打包发布 / Package

```bash
cd frontend
npx vite build
npx --package @electron/asar asar pack build_app app.asar
```

### 📁 项目结构

```
MYGO-DSP/
├── dsp-core/          # C++17 DSP 引擎 (~2500行)
│   ├── include/       # 头文件 (接口定义)
│   ├── src/           # 源文件 (实现)
│   ├── tests/         # Google Test + 基准测试
│   └── CMakeLists.txt # CMake 构建
├── frontend/          # Electron + React 前端
│   ├── electron/      # 主进程 + preload
│   ├── src/           # React 组件 + 样式
│   └── release/       # 打包输出
└── report/            # 项目开发报告 (LaTeX + PDF)
```

### 📊 性能 / Performance

| 测试项 | 吞吐量 | 延迟/样本 |
|--------|--------|----------|
| 波形生成 | 18.9M 样本/秒 | 52.83 ns |
| IIR order=4 | 45.4M 样本/秒 | 22.02 ns |
| FIR order=64 | 3.1M 样本/秒 | 319.64 ns |
| 完整管线 | 23,928 帧/秒 | 41.79 μs |

### 📖 开发报告

详细报告见 `report/` 目录：
- **LaTeX 源文件**: `mygo-dsp-report.tex` (4072行, 99页)
- **PDF**: `mygo-dsp-report.pdf`

涵盖：环境搭建、C++ 基础、逐行代码解释、算法原理、IPC 协议、前端架构、测试验证。

### 📄 许可 / License

MIT License

---

## English / English

### 📌 Overview

MYGO-DSP is an enterprise-grade digital signal processing and filter design platform built with C++17, Electron, and React. It features waveform generation, FIR/IIR filtering, real-time spectrum analysis, and intelligent filter design.

- **C++ DSP Engine**: 7 waveform types, FIR window method, IIR analog prototype method, auto filter designer
- **IPC Protocol**: stdin/stdout JSON line protocol, request-response pattern
- **Desktop UI**: macOS Big Sur style, Electron + React 19 + ECharts

### ✅ Features

| Module | Capabilities |
|--------|-------------|
| Waveform Gen | Sine/Square/Triangle/Sawtooth/WhiteNoise/PinkNoise/Pulse |
| Real-time Control | Freq(1-96kHz), Amplitude, Phase, Duty Cycle, Multi-waveform mix |
| FIR Filter | 5 window functions × 4 types (LP/HP/BP/BS), order 1-200 |
| IIR Filter | 6 prototypes + 9 RBJ biquads, order 1-20 |
| Smart Design | Auto prototype selection + order estimation |
| Real-time Analysis | Time domain, FFT spectrum (256pt), pole-zero plot |

### 🔧 Quick Start

#### Prerequisites

| Tool | Version |
|------|---------|
| CMake | ≥ 3.16 |
| MinGW/GCC | ≥ 8.1 (C++17) |
| Node.js | ≥ 18 |
| Git | - |

#### Build DSP Engine

```bash
cd dsp-core
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
```

#### Run Frontend

```bash
cd frontend
npm install
npx electron .
```

#### Package

```bash
cd frontend
npx vite build
npx --package @electron/asar asar pack build_app app.asar
```

### 📁 Project Structure

```
MYGO-DSP/
├── dsp-core/          # C++17 DSP engine (~2500 lines)
│   ├── include/       # Headers (interfaces)
│   ├── src/           # Source (implementations)
│   ├── tests/         # Google Test + benchmarks
│   └── CMakeLists.txt # CMake build
├── frontend/          # Electron + React frontend
│   ├── electron/      # Main process + preload
│   ├── src/           # React components + styles
│   └── release/       # Packaged output
└── report/            # Development report (LaTeX + PDF)
```

### 📊 Performance

| Test | Throughput | Latency/Sample |
|------|-----------|---------------|
| Waveform Gen | 18.9M samples/s | 52.83 ns |
| IIR order=4 | 45.4M samples/s | 22.02 ns |
| FIR order=64 | 3.1M samples/s | 319.64 ns |
| Full Pipeline | 23,928 frames/s | 41.79 μs |

### 📖 Development Report

See `report/` directory:
- **LaTeX source**: `mygo-dsp-report.tex` (4072 lines, 99 pages)
- **PDF**: `mygo-dsp-report.pdf`

### 📄 License

MIT License

Copyright (c) 2026 M45hiro

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
