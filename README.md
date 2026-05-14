# MYGO-DSP — Modular Yielding Graphical Operators

实时数字信号处理与滤波器设计桌面应用。

![macOS-style UI](https://img.shields.io/badge/UI-macOS%20Style-blue)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C)
![Electron](https://img.shields.io/badge/Electron-34-47848F)
![React](https://img.shields.io/badge/React-19-61DAFB)

## 截图

```
┌──────────────────────────────────────────────────┐
│  ● ● ●  MYGO-DSP                                 │
├──────────┬───────────────────────────────────────┤
│ Wave     │  Time Domain | Spectrum | Pole-Zero    │
│ Sources  │                                        │
│ [+ Add]  │  ~~~~~~~~                              │
│ ┌──────┐ │  ~      ~~~~~      ~~                  │
│ │Wave#0│ │  ~          ~~~~                       │
│ │ ...  │ │                                        │
│ └──────┘ │                                        │
│ Filters  │                                        │
│ [+ IIR]  │                                        │
│ ┌──────┐ │                                        │
│ │IIR#0 │ │                                        │
│ └──────┘ │                                        │
│ [⚡Smart]│                                        │
│ [▶ Play] │                                        │
├──────────┴───────────────────────────────────────┤
│ [▶ 播放]                                          │
└──────────────────────────────────────────────────┘
```

## 快速开始

### 前置要求

- Windows (本项目当前仅支持 Windows)
- [CMake](https://cmake.org/) ≥ 3.16
- [MinGW/GCC](https://www.mingw-w64.org/) ≥ 8.1 (支持 C++17)
- [Node.js](https://nodejs.org/) ≥ 18
- [Git](https://git-scm.com/)

### 1. 克隆

```bash
git clone https://github.com/yourname/MYGO-DSP.git
cd MYGO-DSP
```

### 2. 编译 DSP 引擎

```bash
cd dsp-core
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
```

编译产物：`dsp-core/build/mygo_dsp_engine.exe`（静态链接，无运行时依赖）

### 3. 运行 Electron 应用

```bash
cd frontend
npm install
npx electron .
```

### 4. 打包为独立 exe

```bash
cd frontend
npx vite build
```

手动打包（详见 `PROJECT_PLAN.md`）后运行 `release/MYGO-DSP-win32-x64/MYGO-DSP.exe`。

## 功能

### 波形发生器
- 7 种波形：正弦、方波、三角波、锯齿波、白噪声、粉红噪声、脉冲波
- 参数实时调节：频率(1-96kHz)、幅度(0-1)、相位(0-360°)、占空比(0-100%)
- 多波形独立控制与实时叠加

### FIR 滤波器
- 5 种窗函数：矩形、汉明、汉宁、布莱克曼、凯泽
- 4 种类型：低通、高通、带通、带阻
- 阶数范围 1-200

### IIR 滤波器
- 6 种模拟原型：巴特沃斯、切比雪夫I/II、椭圆(考尔)、贝塞尔、勒让德
- 9 种 RBJ 双二阶：LP/HP/BP(两种)/BS/低架/高架/带架/全通
- 阶数 1-20，参数可调(Q值、纹波、阻带衰减)

### 智能滤波器设计
- 输入通带/阻带技术指标
- 自动选择最优原型（椭圆→切比雪夫→巴特沃斯→FIR回退）
- 一键添加到滤波器链

### 实时可视化
- 时域波形图 (ECharts)
- FFT 频谱图 (256点, Hanning窗)
- 零极点图 (单位圆 + 极点/零点)

## 技术架构

```
React (macOS UI) → preload (contextBridge) → Electron Main → stdin/stdout JSON → C++ DSP Engine
```

- **C++ 引擎**: 单例模式 Engine 管理波形/滤波器实例，`std::mutex` 线程安全
- **IPC**: JSON 行协议，每个请求有唯一递增 id 匹配响应
- **前端**: React 函数式组件 + useState/useCallback Hooks + ECharts 图表
- **UI**: macOS Big Sur 风格，毛玻璃效果 (`backdrop-filter: blur`)

## 项目结构

```
MYGO-DSP/
├── dsp-core/          # C++17 DSP 引擎 (~2500行)
│   ├── include/       # 头文件 (接口定义)
│   ├── src/           # 源文件 (实现)
│   ├── tests/         # Google Test + 基准测试
│   └── build/         # 构建输出
├── frontend/          # Electron + React 前端
│   ├── electron/      # 主进程 + preload
│   ├── src/           # React 组件
│   └── release/       # 打包输出
└── report/            # 开发报告 (HTML+PDF)
```

## 测试

| 测试 | 命令 | 用例数 |
|------|------|--------|
| Google Test | `cmake -DBUILD_TESTS=ON && mingw32-make dsp_tests` | 38 |
| 性能基准 | `cmake -DBUILD_BENCHMARKS=ON && mingw32-make dsp_benchmarks` | 8 |

**关键性能指标** (Intel i7-10750H, MinGW -O2):
- 波形生成: 52.83 ns/sample (1890万样本/秒)
- IIR order=4: 22.02 ns/sample (4540万样本/秒)
- FIR order=64: 319.64 ns/sample (310万样本/秒)
- 完整管线: 41.79 μs/帧 (128样本)

## 开发报告

详细的项目开发报告见 `report/` 目录：
- `report/mygo-dsp-report.html` — 完整 HTML 版本（推荐浏览器打开）
- `report/mygo-dsp-report.pdf` — PDF 版本（浏览器打印生成）

报告涵盖：环境搭建、C++ 基础概念、逐行代码解释、算法原理、IPC 协议、前端架构、构建部署、完整测试数据。

## 许可

MIT
