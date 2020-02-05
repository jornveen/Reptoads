const p = require('path');
const config = require('./config.js');
const projectConfig = require("./project_config.js");
const { remote } = require('electron');
const loader = require('monaco-loader');
const FileManager = require('./filemanager.js');
var events = require('events');
const { ipcRenderer } = require('electron');

module.exports.openEditor = function (path) {
    loader().then((monaco) => {
        const editor = monaco.editor.create(document.getElementById('config-editor-client'), {
          language: 'json',
          theme: 'vs-light',
          automaticLayout: true,
        });
        const fileManager = new FileManager({ editor, monaco });
        var config = p.join( path, "client/config.json");
        this.clientFileManager = fileManager;
        fileManager.openFile(config);
    });


    loader().then((monaco) => {
        const editor = monaco.editor.create(document.getElementById('config-editor-server'), {
          language: 'json',
          theme: 'vs-light',
          automaticLayout: true,
        });
        const fileManager = new FileManager({ editor, monaco });
        this.serverFileManager = fileManager;
        var config = p.join( path, "server/config.json");
        fileManager.openFile(config);
    });

    document.querySelector('#save-config-btn').onclick = () => {
        this.serverFileManager.saveFile();
        this.clientFileManager.saveFile();
    }
}
module.exports.closeEditor = function () {
    document.getElementById('config-editor-client').innerHTML = "";
    document.getElementById('config-editor-server').innerHTML = "";
}
module.exports.openMapChooser = function () {
    let window = remote.getCurrentWindow();
    window.loadFile('src/content/maps_chooser.html');
}
module.exports.openUIEditor = function () {
    let window = remote.getCurrentWindow();
    window.loadFile('src/content/ui_editor.html');
}

module.exports.openDeckEditor = function () {
    let window = remote.getCurrentWindow();
    console.log(999)
    window.loadFile('src/content/deck_editor.html');
}
module.exports.backToProjectView = function () {
    let window = remote.getCurrentWindow();
    window.loadFile('src/content/project_view.html');
}
module.exports.openAIEditor = function (path) {
    let window = remote.getCurrentWindow();
    window.hide();
    ipcRenderer.send("show-splash");
    window.loadFile('src/content/ai_editor.html'); 
}
module.exports.openProfilesEditor = function () {
    let window = remote.getCurrentWindow();
    window.loadFile('src/content/profiles_editor.html');
}
module.exports.openPreview = function(file){
    const {OpenExternalProgram} = require("./thehub")
    OpenExternalProgram('preview',file);
}



function GenerateHTML(v,value){
    var html = '<div class="input-group-prepend"> <span class="input-group-text" id="name-addon" style="text-transform: capitalize; ">'+v+'</span> </div><input type="text" class="form-control" id="'+v+'-data" value="'+value+'"  aria-describedby="basic-addon">';
    var div = document.createElement("div");
    div.classList.add("input-group");
    div.classList.add("mb-3");
    div.innerHTML = html;
    return div;
}


function DisplayProjectConfig(project,elemnt){
    projectConfig.LoadProjectConfig(project);
    Object.keys(projectConfig.ProjectConfig).forEach((v,k)=>{
                var value = projectConfig.ProjectConfig[v];
                var el = GenerateHTML(v,value);
                var div = document.createElement("div");
                var html = "";
                div.classList.add("input-group-append");
                if(v == "drive"){
                    html = '<button class="btn btn-outline-secondary dropdown-toggle" type="button" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Choose</button> <div class="dropdown-menu" id="teamdrives"> </div>';  
                  div.innerHTML = html;
                  var driveInput = el.querySelector("#drive-data");
                  driveInput.setAttribute("data-id",driveInput.getAttribute("value"));
                  console.log(value)
                  driveInput.setAttribute("value",value);

                }else if (v == "file"){
                    html = '<button class="btn btn-outline-secondary dropdown-toggle" type="button" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Choose</button> <div class="dropdown-menu" id="drive-files"> </div>';  
                    div.innerHTML = html;
                    var fileInput = el.querySelector("#file-data");
                    fileInput.setAttribute("data-id",fileInput.getAttribute("value"));
                    fileInput.setAttribute("value",value);
                }else if(v == 'teamdrive'){
                    el.querySelector("input").setAttribute("disabled",true); 
                }
                el.appendChild(div);
                if(v == "drive"){
                var info = document.createElement("p");
                info.classList.add("text-info");
                info.innerHTML = 'Status: <span id="teamdrive-update-status">Loaded</span>';
                elemnt.appendChild(info)
                }
            if(v == "realname" || v == "path"){
                el.querySelector("input").setAttribute("disabled",true); 
            }
            elemnt.appendChild(el);

    });
}


function StartGame(project,onError){
    var eventEmitter = new events.EventEmitter();
    var u = require("../js/app-updater.js").Updater();


    var status = {
        client:false,
        server:false
    }

    eventEmitter.on('client-ok', function (e) { status.client = true; eventEmitter.emit("app-ok")});
    eventEmitter.on('server-ok', function (e) {  status.server = true;  eventEmitter.emit("app-ok")});
    eventEmitter.on('app-ok', function (e) {
        if(status.server && status.client){
            THEHUB.OpenExternalProgram("game", project.path);
        }
     });

    if(!require('fs').existsSync(project.path + "/Client/TBSG.exe")){
        u.UpdateFiles("Client.zip",(progress)=>{
            if(progress >= 100){
                eventEmitter.emit("client-ok")
            }
        },onError);
    }else{
        eventEmitter.emit("client-ok")
    }

    if(!require('fs').existsSync(project.path + "/Server/Server.exe")){
        u.UpdateFiles("Server.zip",(progress)=>{
            if(progress >= 100){
                eventEmitter.emit("server-ok");
            }
        },onError);
    }else{
        eventEmitter.emit("server-ok");
    }
}


function SaveProjectConfig(project){
    projectConfig.SaveConfig(project);
}
module.exports.StartGame = StartGame;
module.exports.SaveProjectConfig = SaveProjectConfig;
module.exports.DisplayProjectConfig = DisplayProjectConfig;