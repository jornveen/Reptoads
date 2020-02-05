'use strict';
const { ipcRenderer, remote } = require('electron');
const { dialog } = require('electron').remote;
const THEHUB = require("../js/thehub.js");
const webservices = require("../js/webservices").Webservice();
const Store = require("electron-store");
const fs = require('fs');
const config = require('./config.js');
const path = require('path');
const unzip = require('unzip');

const updater = require('../js/app-updater.js');
const validate = require('../js/validate-project');


function GetScriptName(name){
    return name.replace(/\s/g,''); 
}

function getLatest(type, onUpdate, onError) {
    var u = updater.Updater();
    u.GetLatest(type,
        () => {
            onUpdate();
        }, () => {
            u.UpdateFiles(THEHUB.toTitleCase(type) + ".zip", onUpdate, onError);
        }, onError);
}

let projects = {};

function binder(elements, event, callback) {
    for (let el of elements) {
        el.addEventListener(event, callback);
    }
}

/// controlls:

module.exports.handleProjects = function () {
    webservices.Start((err) => {
        if (err) {
            throw err;
            let content = document.getElementById("content");
            content.innerHTML = '<div class="alert alert-primary" role="alert">Fatal Mysql Error:' + err + '</div>';
            return;
        }
        projects.projects = []
        webservices.GetProjects(
            (element) => {
                projects.projects.push(element);
            },
            (err) => {
                let content = document.getElementById("content");
                content.innerHTML = '<div class="alert alert-primary" role="alert">Fatal Mysql Error:' + err + '</div>';
            });
        /*
            if (!Object.prototype.forEach) {
                Object.defineProperty(Object.prototype, 'forEach', {
                    value: function (callback, thisArg) {
                        if (this == null) {
                            throw new TypeError('Not an object');
                        }
                        thisArg = thisArg || window;
                        for (var key in this) {
                            if (this.hasOwnProperty(key)) {
                                callback.call(thisArg, this[key], key, this);
                            }
                        }
                    }
                });
            }
            console.log(fp)
            let querya = []
            fp.forEach((v,k)=>{
                if(typeof v == "object"){
                    v.forEach((vv,kk)=>{
                        querya.push("INSERT INTO `projectConfig` (`id`, `projectId`, `name`, `parent`, `value`, `type`) VALUES (NULL, 1, '"+kk+"', '"+k+"', '"+vv+"', '1');");
                    })
                }else{
                    querya.push("INSERT INTO `projectConfig` (`id`, `projectId`, `name`, `parent`, `value`, `type`) VALUES (NULL, 1, '"+k+"', 'root', '"+v+"', '1');");
                }
                });
            fs.writeFileSync('sql.sql',querya.join("\n"));
            return;
        */

        webservices.OnEvent("get-projects", () => {
            console.log(projects.projects)
            module.exports.DisplayProjects();
        });
    });
};

function GetStatusOfProject(project) {
    let projectPath = path.join(config.config.projects, project.realname);
    return fs.existsSync(projectPath);
}

function CreateScripts(project) {
    let projectId = project.id;
    let projectConfig = require("./project_config")
    projectConfig.LoadProjectConfig(config.MakeProjectDir(project.realname));
    let scriptFolder = path.join(config.MakeProjectDir(project.realname),"Server", projectConfig.ServerConfig.assets.scripts);
    if (!fs.existsSync(scriptFolder + "Libs"))
        fs.mkdirSync(scriptFolder + "Libs");
    if (!fs.existsSync(scriptFolder + "Cards"))
        fs.mkdirSync(scriptFolder + "Cards");
    if (!fs.existsSync(scriptFolder + "Monsters"))
        fs.mkdirSync(scriptFolder + "Monsters");
    webservices.GetScripts(
        {
            projectId: projectId,
            getOther: true
        },
        (script) => {
            let scriptpath = scriptFolder + "Libs/" + GetScriptName(script.scriptName) + ".lua";
            let scriptpathMeta = scriptFolder + "Libs/" + GetScriptName(script.scriptName) + ".meta";
            fs.writeFileSync(scriptpath, script.content);
            fs.writeFileSync(scriptpathMeta, script.revision);
        },
        (err) => {
            console.error(err);
        }
    );
    webservices.GetScripts(
        {
            projectId: projectId,
            getPlayerCards: true
        },
        (script) => {
            let scriptpath = scriptFolder + "Cards/" + GetScriptName(script.scriptName) + ".lua";
            let scriptpathMeta = scriptFolder + "Cards/" + GetScriptName(script.scriptName) + ".meta";
            fs.writeFileSync(scriptpath, script.content);
            fs.writeFileSync(scriptpathMeta, script.revision);
        },
        (err) => {
            console.error(err);
        }
    );
    webservices.GetScripts(
        {
            projectId: projectId,
            getMonsters: true
        },
        (script) => {
            let scriptpath = scriptFolder + "Monsters/" + GetScriptName(script.scriptName) + ".lua";
            let scriptpathMeta = scriptFolder + "Monsters/" + GetScriptName(script.scriptName) + ".meta";
            fs.writeFileSync(scriptpath, script.content);
            fs.writeFileSync(scriptpathMeta, script.revision);
        },
        (err) => {
            console.error(err);
        }
    );
}

function ParseValue(value) {
    if (!isNaN(value) && value != null && value != "") {
        return parseFloat(value)
    }
    return value;
}
var configDownloadStatus = 0;

function DownloadConfig(project, callback) {
    let serverConfig = {}
    let clientConfig = {}
    configDownloadStatus = 0;
    webservices.GetProjectConfig({ projectId: project.id, configType: 0 }, (config) => {
        if (config.parent != "root") {
            if (!serverConfig.hasOwnProperty(config.parent))
                serverConfig[config.parent] = {};
            serverConfig[config.parent][config.name] = ParseValue(config.value);
        } else {
            serverConfig[config.name] = ParseValue(config.value);
        }
    });
    webservices.GetProjectConfig({ projectId: project.id, configType: 1 }, (config) => {
        if (config.parent != "root") {
            if (!clientConfig.hasOwnProperty(config.parent))
                clientConfig[config.parent] = {};
            clientConfig[config.parent][config.name] = ParseValue(config.value);
        } else {
            clientConfig[config.name] = ParseValue(config.value);
        }
    });
    webservices.OnEvent("get-config-client", () => {
        let path = config.MakeAssetDir(project.realname).client.root + "/config.json";
        fs.writeFileSync(path, JSON.stringify(clientConfig));
        configDownloadStatus += 1;
        if (configDownloadStatus == 2) {
            callback()
        }
        console.log("[PROJECTS] update config")
    });

    webservices.OnEvent("get-config-server", () => {
        let path = config.MakeAssetDir(project.realname).server.root + "/config.json";
        fs.writeFileSync(path, JSON.stringify(serverConfig));
        configDownloadStatus += 1;
        if (configDownloadStatus == 2) {
            callback()
        }
        console.log("[PROJECTS] update config")
    });
}


function DownloadProject(projectId, callback) {
    webservices.GetProjects((project) => {
        module.exports.NewProject(project, (obj) => {
            if (obj.hasOwnProperty("error")) {
                THEHUB.NotifFail("Fail", obj.error);
                module.exports.DisplayProjects();
            } else {
                THEHUB.NotifSuccess("Success", "Project was downloaded");
                //GetConfig
                DownloadConfig(project, callback);
            }

        })
    }, (err) => {
        alert(err);
        THEHUB.NotifFail("Fail", err);
        throw err;
        return;
    }, { id: projectId });
}


module.exports.DisplayProjects = function () {
    let content = document.getElementById("content");
    content.innerHTML = '<div class="list-group" id="listOfProjects"></div>';
    let listOfProjects = document.getElementById("listOfProjects");
    projects.projects.forEach(element => {
        let status = "";
        if (GetStatusOfProject(element)) {
            status = '<i class="fas fa-download text-success"></i>'
        } else {
            status = '<i class="fas fa-exclamation-triangle text-danger"></i>'
        }
        let projectPath = path.join(config.config.projects, element.realname);
        //<p class="project-open" data-path="' + projectPath + '">' + element.name + '- <span class="path">' + projectPath + '</span></p> 
        listOfProjects.insertAdjacentHTML('afterbegin', '<div class="list-group-item list-group-item-action  project-open" data-projectid="' + element.id + '">' +
            '<div class="d-flex w-100 justify-content-between">' +
            '<h5 class="mb-1 project-open" data-path="' + element.id + '">' + status + ' <span id="config-status-' + element.id + '"></span> ' + element.name + '</h5>' +
            '</div>' +
            '<p class="mb-1">' + element.description + '</p>' +
            '<small>' + projectPath + '</small>' +
            '<div class="btn-group btn-block justify-content-end" role="group" ><button type="button" class="btn project-extern btn-warning" data-path="' + projectPath + '"><i class="fas fa-folder-open"></i></button><button type="button" class="btn btn-danger project-delete" onclick="deletethis( ' + "'" + element.id + "'" + ')" data-projectid="' + element.id + '"><i class="fas fa-trash-alt"></i></button></div>' +
            '</div>');
        validate.ValidateConfig(element, (type) => {
            let el = document.getElementById('config-status-' + element.id);
            el.innerHTML = el.innerHTML + ' <i class="fas fa-cog text-success"></i>'
        }, (type) => {
            let el = document.getElementById('config-status-' + element.id);
            el.innerHTML = el.innerHTML + ' <i class="fas fa-cog text-danger"></i>'
        });
        if(validate.GetStatusOfProject(element))
            validate.ValidateScripts(element,(revs)=>{
                let el = document.getElementById('config-status-' + element.id);
                    if(revs.length == 0){
                        el.innerHTML = el.innerHTML + ' <i class="far fa-file-code text-success"></i>'
                    }else{
                        el.innerHTML = el.innerHTML + ' <i class="far fa-file-code text-danger"></i>'
                    }
            });
    });

    binder(document.getElementsByClassName('project-delete-confirm'), 'click', function (evn) {
        var projectId = this.getAttribute("data-projectid");
        module.exports.DeleteProject(projectId);
        THEHUB.NotifSuccess("Success", "Project was deleted");
        evn.stopPropagation();
    });

    binder(document.getElementsByClassName('project-extern'), 'click', function (evn) {
        var dataPath = this.getAttribute("data-path");
        var $this = this;
        //var pathStr = path.resolve(__dirname, "../" + dataPath);
        require('child_process').exec('start "" "' + dataPath + '"', [], (error, stdout, stderr) => {
            if (error) {
                $this.parentElement.innerHTML = '<div class="alert alert-danger" role="alert">Could not open path!</div>';
                THEHUB.NotifFail("Error", "Could not open path!");
            }
        });
        evn.stopPropagation();
    });

    binder(document.getElementsByClassName('project-open'), 'click', function (evn) {
        var projectId = this.getAttribute("data-projectid");
        module.exports.OpenProject(projectId);
        document.getElementById("content").innerHTML = "";
        THEHUB.Spinner(document.getElementById("content"));
        evn.stopPropagation();
    });
}
module.exports.UpdateProjects = function (project) {
    THEHUB.Spinner(document.getElementById("content"));
    module.exports.handleProjects();
    webservices.OnEvent("get-projects", () => {
        module.exports.DisplayProjects();
    });
};
function OpenProjectFromHDD(value, window) {
    projects.last = value.id;
    console.log("[THE HUB] Found Project");
    var detail = {
        name: "project",
        value: value.id
    };
    var store = new Store();
    store.set("current-project", detail);
    window.hide();
    ipcRenderer.send("show-splash");
    window.loadFile('src/content/project_view.html');
    return;
}
module.exports.OpenProject = function (projectId) {
    if (projectId == null) {
        alert("Project Id was empty!");
        return;
    }
    let store = new Store();
    store.set("last-project",projectId);
    let window = remote.getCurrentWindow();
    var projectWasFound = false;
    projects.projects.forEach((value, index) => {
        if (value.id == projectId) {
            if (!GetStatusOfProject(value)) {
                DownloadProject(projectId, () => {
                    CreateScripts(value);
                    store.set('last-project-obj',value)
                    OpenProjectFromHDD(value, window);
                });
                return;
            }
            store.set('last-project-obj',value)
            OpenProjectFromHDD(value, window);
            projectWasFound = true;
            return;
        }
    });
    if (!projectWasFound) {
        console.log("Error: Project not on list.");
    }
};

module.exports.OpenLastProject = function () {
    let store = new Store();
    module.exports.OpenProject(store.get("last-project",null));
}

module.exports.DeleteProject = function (projectPath) {
    var deleteFolderRecursive = function (filepath) {
        if (fs.existsSync(filepath)) {
            fs.readdirSync(filepath).forEach(function (file, index) {
                var curPath = path.join(filepath, file);
                if (fs.lstatSync(curPath).isDirectory()) { // recurse
                    deleteFolderRecursive(curPath);
                } else { // delete file
                    fs.unlinkSync(curPath);
                }
            });
            if (fs.existsSync(filepath)) {
                fs.rmdirSync(filepath);
            }
        }
    };
    var pathToDelete = path.join(__dirname, "../" + projectPath);
    console.log(pathToDelete);
    deleteFolderRecursive(pathToDelete);

    for (var x = 0; x < Object.values(projects.projects).length; x++) {
        console.log(Object.values(projects.projects[x]));
        if (Object.values(projects.projects[x]).indexOf(projectPath) != -1) {
            projects.projects.splice(x, 1);
            break;
        }
    }
    module.exports.UpdateProjects();
};

module.exports.NewProject = function (project, callback) {
    project.path = path.join(config.config.projects, project.realname);
    if (fs.existsSync(project.path)) {
        callback({ "error": "folder exists!" });
    } else {
        var dir = config.MakeAssetDir(project.realname);
        fs.mkdir(project.path, { recursive: true }, (err) => {
            if (err == null) {
                fs.createReadStream(config.config.example_project)
                    .pipe(unzip.Extract({
                        path: project.path
                    })
                    ).on('finish', () => {
                        projects.projects.push(project);
                        // Get latest and  update:
                        getLatest("client", () => {
                            getLatest("server", () => {
                                fs.writeFileSync(project.path + "/project.hub", JSON.stringify(project, null, 4));
                                callback({});
                            }, () => {
                                THEHUB.NotifInfo("Info", "Could not download and unpack the latest version of the Application");
                                fs.writeFileSync(project.path + "/project.hub", JSON.stringify(project, null, 4));
                                callback({});
                            });
                        }, () => {
                            THEHUB.NotifInfo("Info", "Could not download and unpack the latest version of the Application");
                            fs.writeFileSync(project.path + "/project.hub", JSON.stringify(project, null, 4));
                            callback({});
                        });
                    })
            } else {
                callback({ "error": "Could not create folder!" });
            }
        });
    }
};