const remote = require('electron').remote;
const config = require('./config.js').config;
const url = require('url')
const path = require('path')

module.exports.OpenSceneEditor = (project,scene) => {
    scene = (scene === undefined)? "":scene;
    project = project +"/"+ config.folders.client +  "/" +config.folders.assets + "/gltfs/";
    const BrowserWindow = remote.BrowserWindow;
    var win = new BrowserWindow({ frame: false ,darkTheme:true,        webPreferences:{
      nodeIntegration : true
    } });
    win.maximize();
    win.loadURL(url.format({
        slashes: true,
        protocol: 'file:',
        pathname: path.resolve('./src/threejs/editor/index.html'),
        query: {
          project: project,
          file: scene
        }
      }));
    win.webContents.openDevTools()
    console.log(`file://${__dirname}/content/scene_edit.html?project=${project}#file==${scene}`)
    console.log(`file://${__dirname}/../threejs/editor/index.html`)
};