const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const { spawn } = require('child_process');
const readline = require('readline');

let mainWindow;
let dspProcess;
let rl;
let pending = {};
let nextId = 1;
let isProd;

function startDspProcess() {
  const isDev = process.argv.includes('--dev') || process.env.NODE_ENV === 'development' || process.resourcesPath.includes('node_modules');
  isProd = !isDev;
  const exePath = isProd
    ? path.join(process.resourcesPath, 'dsp', 'mygo_dsp_engine.exe')
    : path.join(__dirname, '..', '..', 'dsp-core', 'build', 'mygo_dsp_engine.exe');
  console.log('DSP exe path:', exePath);
  try {
    dspProcess = spawn(exePath, [], {
      stdio: ['pipe', 'pipe', 'pipe'],
      windowsHide: true,
    });
    dspProcess.on('error', (err) => console.error('DSP spawn error:', err));
  } catch (e) {
    console.error('DSP spawn failed:', e);
  }

  rl = readline.createInterface({
    input: dspProcess.stdout,
    crlfDelay: Infinity,
  });

  rl.on('line', (line) => {
    try {
      const response = JSON.parse(line.trim());
      const id = response.id;
      console.log('recv:', line.trim(), 'pending keys:', Object.keys(pending).join(','));
      if (id !== undefined && pending[id]) {
        if (response.status === 'error') {
          pending[id].reject(new Error(response.data || 'unknown error'));
        } else {
          pending[id].resolve(response.data !== undefined ? response.data : response);
        }
        delete pending[id];
      }
    } catch (e) {
      console.error('Failed to parse DSP response:', e);
    }
  });

  dspProcess.stderr.on('data', (data) => {
    console.error('DSP stderr:', data.toString());
  });

  dspProcess.on('close', (code) => {
    console.log('DSP process exited with code:', code);
    dspProcess = null;
  });
}

function sendCommand(cmd, params = {}) {
  return new Promise((resolve, reject) => {
    if (!dspProcess || !dspProcess.stdin.writable) {
      reject(new Error('DSP process not running'));
      return;
    }
    const reqId = nextId++;
    const request = { id: reqId, cmd };
    for (const k in params) request[k] = params[k];
    pending[reqId] = { resolve, reject };
    const json = JSON.stringify(request) + '\n';
    console.log('send:', json.trim());
    dspProcess.stdin.write(json);
  });
}

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 960,
    height: 680,
    frame: false,
    titleBarStyle: 'hidden',
    backgroundColor: '#1a1a2e',
    webPreferences: {
      preload: path.join(__dirname, 'preload.cjs'),
      nodeIntegration: false,
      contextIsolation: true,
    },
  });
  mainWindow.setMenuBarVisibility(false);

  mainWindow.loadFile(path.join(__dirname, '../dist/index.html'));
  if (!isProd) mainWindow.webContents.openDevTools();
}

app.whenReady().then(() => {
  startDspProcess();
  createWindow();

  ipcMain.handle('window-close', (event) => {
    BrowserWindow.fromWebContents(event.sender)?.close();
  });
  ipcMain.handle('window-minimize', (event) => {
    BrowserWindow.fromWebContents(event.sender)?.minimize();
  });
  ipcMain.handle('window-maximize', (event) => {
    const win = BrowserWindow.fromWebContents(event.sender);
    if (win) {
      if (win.isMaximized()) win.unmaximize();
      else win.maximize();
    }
  });

  ipcMain.handle('init', async (_event, sampleRate, bufferSize) => {
    return sendCommand('init', { sampleRate, bufferSize });
  });

  ipcMain.handle('addWave', async (_event, params) => {
    const result = await sendCommand('addWave', params);
    return parseInt(result, 10);
  });

  ipcMain.handle('removeWave', async (_event, wid) => {
    return sendCommand('removeWave', { wid });
  });

  ipcMain.handle('updateWave', async (_event, wid, params) => {
    return sendCommand('updateWave', { wid, ...params });
  });

  ipcMain.handle('addFilterFIR', async (_event, params) => {
    return sendCommand('addFilterFIR', params);
  });

  ipcMain.handle('addFilterIIR', async (_event, params) => {
    return sendCommand('addFilterIIR', params);
  });

  ipcMain.handle('removeFilter', async (_event, fid) => {
    return sendCommand('removeFilter', { fid });
  });

  ipcMain.handle('updateFilter', async (_event, fid, params, filterType) => {
    return sendCommand('updateFilter', { fid, ...params, isIIR: filterType === 'IIR' ? 1 : 0 });
  });

  ipcMain.handle('setBypass', async (_event, bid, bypass) => {
    const reqId = nextId++;
    const json = JSON.stringify({ id: reqId, cmd: 'setBypass', bid, bypass: bypass ? 1 : 0, target: 'filter' }) + '\n';
    dspProcess.stdin.write(json);
    return 'ok';
  });

  let lastProcessResult = null;

  ipcMain.handle('process', async () => {
    const result = await sendCommand('process');
    if (Array.isArray(result)) return result;
    if (result && result.data) return result.data;
    const zeros = new Array(128); zeros.fill(0);
    return zeros;
  });

  ipcMain.handle('getCounts', async () => {
    return sendCommand('getCounts');
  });

  ipcMain.handle('autoDesign', async (_event, spec) => {
    return sendCommand('autoDesign', { ...spec });
  });

  ipcMain.handle('getPoleZero', async (_event, fid) => {
    const result = await sendCommand('getPoleZero', { fid });
    if (result && result.poles) return result;
    return { poles: [], zeros: [] };
  });
});

app.on('window-all-closed', () => {
  if (dspProcess) {
    dspProcess.kill();
  }
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});
