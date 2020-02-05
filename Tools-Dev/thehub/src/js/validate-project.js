const THEHUB = require("../js/thehub.js");
const webservices = require("../js/webservices").Webservice();
const Store = require("electron-store");
const fs = require('fs');
const config = require('./config.js');
const path = require('path');

function ParseValue(value) {
    if (!isNaN(value) && value != null && value != "") {
        return parseFloat(value)
    }
    return value;
}

function ValidateConfig(project, same, needUpdate) {

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

    let serverConfig = {}
    let clientConfig = {}
    webservices.GetProjectConfig({ projectId: project.id, configType: 1 }, (config) => {
        if (config.parent != "root") {
            if (!clientConfig.hasOwnProperty(config.parent))
                clientConfig[config.parent] = {};
            clientConfig[config.parent][config.name] = ParseValue(config.value);
        } else {
            clientConfig[config.name] = ParseValue(config.value);
        }
    });
    webservices.GetProjectConfig({ projectId: project.id, configType: 0 }, (config) => {
        if (config.parent != "root") {
            if (!serverConfig.hasOwnProperty(config.parent))
                serverConfig[config.parent] = {};
            serverConfig[config.parent][config.name] = ParseValue(config.value);
        } else {
            serverConfig[config.name] = ParseValue(config.value);
        }
    });
    webservices.OnEvent("get-config-client", () => {
        let path = config.MakeAssetDir(project.realname).client.root + "/config.json";
        let currentConfig = JSON.parse(fs.readFileSync(path));
        let validate = 0;
        currentConfig.forEach((v, i) => {
            if (typeof v == "object") {
                v.forEach((vv, ii) => {
                        validate += (clientConfig[i][ii] != vv) ? 1 : 0;
                })
            } else {
                validate += (clientConfig[i] != v) ? 1 : 0;
            }
        });
        if (validate > 0){
            needUpdate(1,clientConfig)
        }else{
            fs.writeFileSync(path,JSON.stringify(clientConfig));
            same(1)
        }
    });

    webservices.OnEvent("get-config-server", () => {
        let path = config.MakeAssetDir(project.realname).server.root + "/config.json";
        let currentConfig = JSON.parse(fs.readFileSync(path));
        let validate = 0;
        currentConfig.forEach((v, i) => {
            if (typeof v == "object") {
                v.forEach((vv, ii) => {
                    validate += (serverConfig[i][ii] != vv) ? 1 : 0;
                })
            } else {
                validate += (serverConfig[i] != v) ? 1 : 0;
            }
        });
        if (validate > 0)
            needUpdate(0,serverConfig)
        else
            same(0)
    });

}

function GetStatusOfProject(project) {
    let projectPath = path.join(config.config.projects, project.realname);
    return fs.existsSync(projectPath);
}

function GetScriptName(name){
    if(name == undefined) return "DummyScript";
    return name.replace(/\s/g,''); 
}

function Unique(array,entry){
    let check = false;
    array.forEach(el =>{
        if(entry.script == el.script){
            check = true;
            return;
        }else 
        if(entry.script.id == el.script.id){
            check = true;
            return;
        }
    })
    return check;
}

function CompareScripts(scriptFolder,sPath,scriptB,revisions){
    let scriptpath = scriptFolder + sPath+"/" + GetScriptName(scriptB.scriptName);
    let scriptpathMeta = scriptFolder +  sPath+"/" + GetScriptName(scriptB.scriptName) + ".meta";
    if(fs.existsSync(scriptpathMeta) && fs.existsSync(scriptpath+".lua")){
    let scriptSource = fs.readFileSync(scriptpath+".lua").toString();
    let scriptRevision = fs.readFileSync(scriptpathMeta);
    let revision = JSON.parse(scriptRevision);
        if (revision != scriptB.revision || scriptSource != scriptB.content) {
            scriptB.path = scriptpath
            let obj =       {
                script: scriptB,
                revision: revision != scriptB.revision,
                sourceRevision:revision,
                source:scriptSource != scriptB.content,
                sourceCode: scriptSource
            }
            if(!Unique(revisions,obj)){
            revisions.push(
                      obj
                            )
                        }
        }else{
            console.log("NOTHING TODO")
        }
    }else{
        fs.writeFileSync(scriptpath+".lua",scriptB.content)
        fs.writeFileSync(scriptpathMeta,scriptB.revision)
    }
}
var revisions = []
var projectId = 0;
var scriptFolder = ""
var ValidationScriptsCallback = ()=>{};
function ValidateScripts(project,callback) {
    ValidationScriptsCallback = callback
    projectId = project.id;
    let projectConfig = require("./project_config")
    projectConfig.LoadProjectConfig(config.MakeProjectDir(project.realname));
     scriptFolder = path.join(config.MakeProjectDir(project.realname), "Server", projectConfig.ServerConfig.assets.scripts);
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
            if(!Array.isArray(script))
                CompareScripts(scriptFolder,"Libs",script,revisions);
        },
        (err) => {
            console.error(err);
        },"done-request-other"
    );

}

document.addEventListener("done-request-other",()=>{
    
    webservices.GetScripts(
        {
            projectId: projectId,
            getPlayerCards: true
        },
        (script) => {
            CompareScripts(scriptFolder,"Cards",script,revisions);
        },
        (err) => {
            console.error(err);
        },"done-request-player-cards"
    );
});

document.addEventListener("done-request-player-cards",()=>{
    webservices.GetScripts(
        {
            projectId: projectId,
            getMonsters: true
        },
        (script) => {
            CompareScripts(scriptFolder,"Monsters",script,revisions);
        },
        (err) => {
            console.error(err);
        },"done-request-monster-cards"
    );
})

document.addEventListener("done-request-monster-cards",()=>{
    console.log(revisions)
    ValidationScriptsCallback(revisions)
    revisions = []
})

module.exports.ValidateConfig = ValidateConfig;
module.exports.ValidateScripts = ValidateScripts;
module.exports.GetStatusOfProject = GetStatusOfProject;