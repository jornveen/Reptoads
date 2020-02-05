const { app, BrowserWindow,ipcMain } = require('electron')
const webservices = require("./js/webservices")
/**
 *  Main file of the main window: Main window process:
 *  Add only stuff which needs to be here in regards of the main process
 *  preferably put stuff encapsulated into seperated files.
 */
//gobla instance of window
let win
let splash
global.splashscreen = splash;

function createWindow () {
  win = new BrowserWindow({ 
        width: 1024,
        height: 768,
        backgroundColor: '#212529',
        frame: false,
        show:false
    })
win.loadFile('src/content/project_chooser.html')

splash = new BrowserWindow({ 
  width: 100,
  height: 100,
  backgroundColor: '#212529',
  frame: false
})
splash.loadFile("src/content/splashscreen.html")
win.loadFile('src/content/project_chooser.html')

win.webContents.openDevTools();
/**
 *  ~~~~~~~~~~~ Window Events ~~~~~~~~~~~
 */
  win.on('closed', () => {
    win = null
    splash.destroy()
  });

  win.once('ready-to-show', () => {
      splash.hide();
      win.show()
  })
  ipcMain.on('show-splash', (event, arg) => {
    splash.show();
  })
win.webContents.on("did-finish-load",()=>{
 if(!win.isVisible()){
    splash.hide();
    win.show()
    win.maximize();
  }
})
  
}
/**
 *  ~~~~~~~~~~~ App Events ~~~~~~~~~~~
 */
app.on('ready', async () => { createWindow()})
app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit()
  }
})

app.on('activate', () => {
  if (win === null) {
    createWindow()
  }
})