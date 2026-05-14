const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('dsp', {
  init: (sampleRate, bufferSize) => ipcRenderer.invoke('init', sampleRate, bufferSize),
  addWave: (params) => ipcRenderer.invoke('addWave', params),
  removeWave: (id) => ipcRenderer.invoke('removeWave', id),
  updateWave: (id, params) => ipcRenderer.invoke('updateWave', id, params),
  addFilterFIR: (params) => ipcRenderer.invoke('addFilterFIR', params),
  addFilterIIR: (params) => ipcRenderer.invoke('addFilterIIR', params),
  removeFilter: (id) => ipcRenderer.invoke('removeFilter', id),
  updateFilter: (id, params, type) => ipcRenderer.invoke('updateFilter', id, params, type),
  setBypass: (id, bypass) => ipcRenderer.invoke('setBypass', id, bypass),
  process: async () => {
    const result = await ipcRenderer.invoke('process');
    console.log('preload process result:', typeof result, result?.constructor?.name, result?.length);
    return result;
  },
  getCounts: () => ipcRenderer.invoke('getCounts'),
  autoDesign: (spec) => ipcRenderer.invoke('autoDesign', spec),
  getPoleZero: (fid) => ipcRenderer.invoke('getPoleZero', fid),
});

contextBridge.exposeInMainWorld('electronAPI', {
  close: () => ipcRenderer.invoke('window-close'),
  minimize: () => ipcRenderer.invoke('window-minimize'),
  maximize: () => ipcRenderer.invoke('window-maximize'),
});
