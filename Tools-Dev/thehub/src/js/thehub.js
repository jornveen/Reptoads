////////////////// requires //////////////////////////////////
const remote = require('electron').remote;
const dialog = remote.dialog;
const fs = require('fs');
const child_process = require('child_process');
const nodePath = require('path');
////////////// INTERNAL ///////////////////////////////////////
const config = require('./config.js');
const webservices = require("./webservices").Webservice();
const Sheets = require("./google-sheets");
const Store = require('electron-store');
const validate = require("./validate-project")
///////////////// GLOBALS ////////////////////////////////////

var template = {
    Monster: fs.readFileSync("db/template.monster.lua"),
    PlayerCard: fs.readFileSync("db/template.playercard.lua"),
    Lib: fs.readFileSync("db/template.lib.lua"),
    AI: fs.readFileSync("db/template.aibehaviour.lua")
}
let getTemplate = (type) => {
    if (type == 0)
        return template.Monster
    else
        return template.PlayerCard
}

var THEHUB = {
    project: {},
    validation_rules: [
        {
            where: "server",
            what: "assets",
            fileOrFolder: ""
        },
        {
            where: "client",
            what: "root",
            fileOrFolder: "../project.hub"
        },
        {
            where: "client",
            what: "assets",
            fileOrFolder: ""
        },
        {
            where: "client",
            what: "root",
            fileOrFolder: "config.json"
        },
        {
            where: "server",
            what: "root",
            fileOrFolder: "config.json"
        },
        {
            where: "client",
            what: "assets",
            fileOrFolder: "maps.json"
        },
        {
            where: "server",
            what: "assets",
            fileOrFolder: "maps.json"
        },
        {
            where: "server",
            what: "assets",
            fileOrFolder: "cards.xml"
        }
    ]
};
var currentWindow = remote.getCurrentWindow();
const COOKIES_LOADED = 'cookies-loaded';
const PROJECT_VALIDATED = 'project-validation';
const PROJECT_NOT_VALIDATED = 'project-validation-error';
///////////////// EVENTS ////////////////////////////////////
var projectValidated = new CustomEvent(PROJECT_VALIDATED, { detail: { project: null } });
var cookiesLoaded = new CustomEvent(COOKIES_LOADED, { detail: { project: null } });
var projectLoadError = new CustomEvent(PROJECT_NOT_VALIDATED, { detail: { msg: null } });
////////////////////////////////////////////////////////////

function ParseAppArguments(args) {
    if (remote.process.argv[2] != args.argument) {
        args.callback();
    }
}


function LoadProjectFromCookies() {
    let store = new Store();
    let currentProject = store.get("current-project", { name: "project", value: 0 });
    if (currentProject.value == 0) {
        alert("Current projectId is incorrect");
        return;
    }
    webservices.Start((err) => {
        if (err) {
            throw err;
            let content = document.getElementById("content");
            content.innerHTML = '<div class="alert alert-primary" role="alert">Fatal Mysql Error:' + err + '</div>';
            return;
        }
        let projectId = currentProject.value;
        webservices.GetProjects((project) => {
            project.path = nodePath.join(config.config.projects, project.realname);
            cookiesLoaded.detail.project = project;
            THEHUB.project = project;
            window.dispatchEvent(cookiesLoaded);
        }, (err) => {
            alert(err);
        }, { id: projectId });
        return;
    });
}

function ValidateFolderOrFile(where, what, fileOrFolder) {
    var projectName = THEHUB.project.realname;
    var dir = config.MakeAssetDir(projectName);
    if (!config.ValidateFileOrFolder(projectName, where, what, fileOrFolder)) {
        console.error("[THEHUB] Could not load folder the " + where + " folder: " + dir[where][what] + "/" + fileOrFolder);
        projectLoadError.detail.msg = "Could not load " + where + "'s folder: " + dir[where][what] + "/" + fileOrFolder;
        window.dispatchEvent(projectLoadError);
        return false;
    }
    return true;
}



function ValidateProjectAndLoad() {
    if (fs.existsSync(THEHUB.project.path)) {
        console.log("[THEHUB] Could load project dir: " + THEHUB.project.path);
        var projectName = THEHUB.project.realname;
        var dir = config.MakeAssetDir(projectName);
        for (var i = 0; i < THEHUB.validation_rules.length; ++i) {
            var entry = THEHUB.validation_rules[i];
            if (!ValidateFolderOrFile(entry.where, entry.what, entry.fileOrFolder))
                return;
        }
        window.dispatchEvent(projectValidated);
    } else {
        projectLoadError.detail.msg = "Could not load project dir: " + THEHUB.project.path;
        window.dispatchEvent(projectLoadError);
        console.error("[THEHUB] Could not find " + THEHUB.project.path);
    }
}

function OnclearsLoaded(callback) {
    window.addEventListener(COOKIES_LOADED, function (event) {
        callback(THEHUB.project);
    });
}

function OnProjectValidated(callback) {
    window.addEventListener(PROJECT_VALIDATED, function (event) {
        //load config:
        var projectName = THEHUB.project.realname;
        var dir = config.MakeAssetDir(projectName);
        const server = JSON.parse(fs.readFileSync(dir.server.root + '/config.json'));
        THEHUB.project.serverConfig = server;
        const client = JSON.parse(fs.readFileSync(dir.client.root + '/config.json'));
        THEHUB.project.clientConfig = client;
        callback(THEHUB.project);
    });
}

function OnProjectValidationError(callback) {
    window.addEventListener(PROJECT_NOT_VALIDATED, function (event) {
        callback(event.detail.msg, THEHUB.project);
    });
}

function UpdateOutput(type, msg) {
    var el = document.getElementById("execution-status");
    var output = document.getElementById("textOutputArea");
    if (el != null) {
        if (type == "error") {
            el.innerHTML = "Failed";
            el.classList.remove("badge-success");
            el.classList.add("badge-danger");
        } else {
            el.innerHTML = "Done";
            el.classList.remove("badge-danger");
            el.classList.add("badge-success");
        }
        output.innerHTML += msg;
    }
}

function OpenExternalProgram(program, path) {

    if (program == "blender") {
        var scene = nodePath.join(__dirname, "../", path, "sceneeditor.blend");
        child_process.exec('start "" "' + scene + '"', [], (error, stdout, stderr) => {
            if (error) {
                dialog.showErrorBox("Error: Could not Open Blender", "Could not open Blender. File could not be found [" + config.config.programs.blender + "]");
            }
        });
    } else if (program == "code") {
        var arg = (path === undefined) ? "" : path;
        child_process.execFile(config.config.programs.code, [arg], (error, stdout, stderr) => {
            if (error) {
                console.error(error)
                dialog.showErrorBox("Error: Could not Open Editor", "Could not open the Script Editor. File could not be found [" + config.config.programs.code + "]");
            }
        });
    } else if (program == "explorer") {
        path = nodePath.resolve(path);
        child_process.exec('start "" "' + path + '"', [], (error, stdout, stderr) => {
            if (error) {
                dialog.showErrorBox("Error: Could not open Folder", "Could not open folder");
            }
        });
    } else if (program == "excel") {
        path = nodePath.resolve(path);
        console.log('start "' + config.config.programs.excel + '" "' + path + '"');
        child_process.exec('start "' + config.config.programs.excel + '" "' + path + '"', [], (error, stdout, stderr) => {
            if (error) {
                dialog.showErrorBox("Error: Could not open Excel", "Could not open Excel: " + error);
            }
        });
    } else if (program == "game") {
        let client = nodePath.join(__dirname, "..", path, "Client", "TBSG.exe");
        let server = nodePath.join(__dirname, "..", path, "Server", "Server.exe");
        let clientWorkDir = nodePath.join(__dirname, "..", path, "Client");
        let serverWorkDir = nodePath.join(__dirname, "..", path, "Server");
        child_process.exec('start /D "' + serverWorkDir + '" Server.exe', [], (error, stdout, stderr) => {
            if (error) {
                UpdateOutput("error", "\nServer:\n" + error);
                dialog.showErrorBox("Error: Could not open the Server Application", "Could not open Server Application. Error:\n" + error);
            }
            child_process.exec('start /D "' + clientWorkDir + '" TBSG.exe', [], (error, stdout, stderr) => {
                if (error) {
                    UpdateOutput("error", "\nClient:\n" + error);
                    dialog.showErrorBox("Error: Could not open the client Application", "Could not open Client Application. Error:\n" + error);
                }
            })
        })
    }else if(program == "preview"){
        let clientWorkDir = nodePath.join(THEHUB.project.path, "Client");
        child_process.execFile(config.config.programs.viewer,["--load-model",path],{cwd:clientWorkDir}, (error, stdout, stderr) => {
            if (error) {
                dialog.showErrorBox("Error: Could not open ViewerApplication", "Could not open ViewerApplication: " + error);
            }else{
                LogToResultConsole("[ViewerApplication] "+stdout.toString());
            }
        });
    }else if(program == "ui"){
        let clientWorkDir = nodePath.join(THEHUB.project.path, "Client");
        child_process.execFile(config.config.programs.viewer,["--ui",path],{cwd:clientWorkDir}, (error, stdout, stderr) => {
            if (error) {
                dialog.showErrorBox("Error: Could not open ViewerApplication", "Could not open ViewerApplication: " + error);
            }else{
                LogToResultConsole("[ViewerApplication] "+stdout.toString());
            }
        });
    }
}

function LogToResultConsole(_string) {
    var textarea = document.getElementById("textOutputArea");
    textarea.insertAdjacentHTML('beforeend', "\n" + _string);
    textarea.scrollTop = textarea.scrollHeight;
}


function RunLuaCheck(path,type) {
    type = (type == undefined)? 0:type;
    var luaCheck = nodePath.resolve(config.config.programs.lua);
    let cwd = nodePath.join(GetCurrentProject().path,"Server")
    var luaPath = "";
    if(path == undefined){
        let config = GetCurrentProject()
        luaPath = nodePath.join(config.path,"Server",config.serverConfig.assets.scripts);
    }else{
        luaPath = nodePath.join(__dirname, '../' + path);
    }
    var log = document.getElementById("textOutputArea");
    log.innerText = "";
    var modes = ["folder","basic"]
    var luaProcess = child_process.spawn(luaCheck, [modes[type],luaPath],{
        cwd: cwd
    });

    luaProcess.stdout.on('data', function (data) {
        LogToResultConsole(data.toString());
    });

    luaProcess.stderr.on('data', function (data) {
        LogToResultConsole('Error: ' + data.toString());
    });

    luaProcess.on('exit', function (code) {
        LogToResultConsole('LuaCheck process exited with code ' + code.toString());
    });
}

function WriteObjectToFile(file, object) {
    fs.writeFileSync(file, JSON.stringify(object, null, 4), function (err) {
        if (err) {
            console.log(err);
        }
    });
}
function WriteToFile(file, data) {
    fs.writeFileSync(file, data);
}

function GetCurrentProject() {
    return THEHUB.project;
}


function DisplayNotification(type, title, text) {
    var elements = document.querySelectorAll(".notification");
    if (elements.length > 0) {
        elements.forEach((node) => {
            node.parentNode.removeChild(node);
        })
    }
    var elm = document.createElement("div");
    elm.classList.add("notification", "notif-active", 'notif-' + type);
    elm.innerHTML = '<div class="notif-head"> <h3>' + title + '</h3> </div><div class="notif-body"> <p>' + text + '</p></div>';
    document.body.appendChild(elm);
}

function NotifSuccess(title, text) {
    DisplayNotification("success", title, text);
}
function NotifInfo(title, text) {
    DisplayNotification("info", title, text);
}
function NotifFail(title, text) {
    DisplayNotification("fail", title, text);
}

function FindCardWithName(cards, name) {

    for (var i = 0; i < cards.length; i++) {
        if (cards[i].Meta.Name == name) {
            return cards[i];
        }
    }
    return null;

}

/////////////////////////////////////////////////////////////////////////////////////
///// ADD Script to DB
/////////////////////////////////////////////////////////////////////////////////////
function AddScriptToDB(type, id) {

    webservices.Insert({
        table: "scripts",
        eventData: { id: id, type: type },
        data: {
            revision: 1,
            content: "'" + getTemplate(type) + "'"
        }
    }, data => { 
        console.log(data) 
    }, data => { 
        console.log(data) 
    }, "add-script-to-db")
}

document.addEventListener("add-script-to-db", (res) => {
    if (res.detail.pos.type == 0) { // Monster
        webservices.Insert({
            table: "scriptMonsterRelation",
            data: {
                monsterId: res.detail.pos.id,
                scriptId: res.detail.result.insertId
            }
        }, err => { console.log(err) }, "add-monsterScript-to-db")
    } else { // player card
        webservices.Insert({
            table: "scriptCardRelation",
            data: {
                cardId: res.detail.pos.id,
                scriptId: res.detail.result.insertId
            }
        }, err => { console.log(err) }, "add-cardScript-to-db")
    }
})

function UpdateScript(oldName,newName,type){
    newName = newName.replace(/\s/g,''); 
    oldName = oldName.replace(/\s/g,''); 
    let projectConfig = require("./project_config")
    projectConfig.LoadProjectConfig(config.MakeProjectDir(GetCurrentProject().realname));
    let path_ = ["Monsters","Cards"]
    let scriptFolder = nodePath.join(config.MakeProjectDir(GetCurrentProject().realname), "Server", projectConfig.ServerConfig.assets.scripts,path_[type],"/");
        console.log(scriptFolder+oldName+".lua")
        if(fs.existsSync(scriptFolder+oldName+".lua")){
            fs.renameSync(scriptFolder+oldName+".lua",scriptFolder+newName+".lua");
        }
        if(fs.existsSync(scriptFolder+oldName+".meta")){
            fs.renameSync(scriptFolder+oldName+".meta",scriptFolder+newName+".meta");
        }
    
}

function ValidateIfCardsHaveScript(data) {
    if (data.Cards.length != 0) {

        function find(array, callback) {
            for (var i = 0; i < array.length; i++) {
                if (callback(array[i])) return array[i];
            }
            return null;
        }
        function find_index(array, callback) {
            for (var i = 0; i < array.length; i++) {
                if (callback(array[i])) return i;
            }
            return null;
        }
        function isNumeric(value) {
            return /^-{0,1}\d+$/.test(value);
        }
        
        function create_query(value) {
            if(isNumeric(value)){
                return parseInt(value);
            }
            return "'" + value + "'";
        }
        var projectId = GetCurrentProject().id
        webservices.Start(() => {

            /////////////////////////////////////////////////////////////////////////////////////
            ///// UPDATE CARD TYPES (INCL MONSTER TYPES)
            /////////////////////////////////////////////////////////////////////////////////////
            webservices.Clear('cardTypes');
            data.CardTypes.forEach(type => {
                webservices.Insert(
                    {
                        table: "cardTypes",
                        eventData: undefined,
                        data: {
                            name: create_query(type)
                        }
                    }, err => { console.log(err) }, "add-cardType"
                );
            })
            /////////////////////////////////////////////////////////////////////////////////////
            ///// ADD REWARDS
            /////////////////////////////////////////////////////////////////////////////////////
            webservices.Clear('cardRarity');
            data.Rarities.forEach(rarity => {
                webservices.Insert(
                    {
                        table: "cardRarity",
                        eventData: undefined,
                        data: {
                            name: create_query(rarity.Rarity),
                            Chance: create_query(rarity.Chance),
                        }
                    }, err => { console.log(err) }, "add-rarity"
                );
            })
            /////////////////////////////////////////////////////////////////////////////////////
            ///// UPDATE CARDS
            /////////////////////////////////////////////////////////////////////////////////////

            data.Cards.forEach(card => {
                webservices.GetCards({
                    projectId: projectId,
                    cardId: (isNaN(card.id)) ? 0 : card.id
                }, result => {
                    let newCard = null;
                    if (result.length == 0) {
                        if (card.Effects.MonsterDamage != undefined) {
                            newCard = {
                                table: "cards",
                                eventData: card.Pos,
                                data: {
                                    projectId: create_query(projectId),
                                    name: create_query(card.Meta.Name),
                                    description: create_query(card.Meta.Description),
                                    cardRarityId: create_query(find_index(data.Rarities, element => { return element == card.Meta.Rarity }) + 1),
                                    cardTypeId: create_query(find_index(data.CardTypes, element => { return element == card.Meta.CardType }) + 1),
                                    monsterDamage: create_query(card.Effects.MonsterDamage),
                                    opponentDamage: create_query(card.Effects.OpponentDamage),
                                    selfDamage: create_query(card.Effects.SelfDamage),
                                    monsterHealth: create_query(card.Effects.MonsterHealth),
                                    opponentHealth: create_query(card.Effects.OpponentHealth),
                                    selfHealth: create_query(card.Effects.SelfHealth),
                                    monsterArmor: create_query(card.Effects.MonsterArmor),
                                    opponentArmor: create_query(card.Effects.OpponentArmor),
                                    selfArmor: create_query(card.Effects.SelfArmor),
                                    opponentDraw: create_query(card.Effects.OpponentDraw),
                                    selfDraw: create_query(card.Effects.SelfDraw),
                                    opponentDiscard: create_query(card.Effects.OpponentDiscard),
                                    selfDiscard: create_query(card.Effects.SelfDiscard),
                                }
                            }
                        } else {
                            newCard = {
                                table: "cards",
                                eventData: card.Pos,
                                data: {
                                    projectId: create_query(projectId),
                                    name: create_query(card.Meta.Name),
                                    description: create_query(card.Meta.Description),
                                    cardRarityId: create_query(find_index(data.Rarities, element => { return element == card.Meta.Rarity }) + 1),
                                    cardTypeId: create_query(find_index(data.CardTypes, element => { return element == card.Meta.CardType }) + 1),
                                }
                            }
                        }
                        if(newCard.data.name != "'null'"){
                            webservices.Insert(newCard, err => { console.log(err) },err => { console.error(err) }, "add-card");
                        }
                    } else if (!card.Deleted) {
                        if (card.Effects.MonsterDamage != undefined) {
                            if (result.name != card.Meta.Name) {
                                UpdateScript(result.name, card.Meta.Name,1);
                            }
                            webservices.Update({
                                table: "cards",
                                where: "id ='" + result.id + "'",
                                changes:
                                    [
                                        { name: "name", value: (card.Meta.Name) },
                                        { name: "description", value: (card.Meta.Description) },
                                        { name: "cardRarityId", value: (find_index(data.Rarities, element => { return element == card.Meta.Rarity }) + 1) },
                                        { name: "cardTypeId", value: (find_index(data.CardTypes, element => { return element == card.Meta.CardType }) + 1) },
                                        { name: "monsterDamage", value: (card.Effects.MonsterDamage) },
                                        { name: "opponentDamage", value: (card.Effects.OpponentDamage) },
                                        { name: "selfDamage", value: (card.Effects.SelfDamage) },
                                        { name: "monsterHealth", value: (card.Effects.MonsterHealth) },
                                        { name: "opponentHealth", value: (card.Effects.OpponentHealth) },
                                        { name: "selfHealth", value: (card.Effects.SelfHealth) },
                                        { name: "monsterArmor", value: (card.Effects.MonsterArmor) },
                                        { name: "opponentArmor", value: (card.Effects.OpponentArmor) },
                                        { name: "selfArmor", value: (card.Effects.SelfArmor) },
                                        { name: "opponentDraw", value: (card.Effects.OpponentDraw) },
                                        { name: "selfDraw", value: (card.Effects.SelfDraw) },
                                        { name: "opponentDiscard", value: (card.Effects.OpponentDiscard) },
                                        { name: "selfDiscard", value: (card.Effects.SelfDiscard) }
                                    ]
                            }, err => { console.log(err) },err => { console.error(err) })

                        } else {
                            if (result.name != card.Meta.Name) {
                                UpdateScript(result.name, card.Meta.Name,1);
                            }
                            webservices.Update({
                                table: "cards",
                                where: "id ='" + result.id + "'",
                                changes:
                                    [
                                        { name: "name", value: (card.Meta.Name) },
                                        { name: "description", value: (card.Meta.Description) },
                                        { name: "cardRarityId", value: (find_index(data.Rarities, element => { return element == card.Meta.Rarity }) + 1) },
                                        { name: "cardTypeId", value: (find_index(data.CardTypes, element => { return element == card.Meta.CardType }) + 1) }
                                    ]
                            }, err => { console.log(err) },err => { console.error(err) });
                        }
                    } else {
                        webservices.Delete("cards", [{ name: "id", value: result.id }], (res) => {
                            Sheets.ClearCells(GetCurrentProject().file, "Cards!A" + card.Pos[1] + ":U" + card.Pos[1]);
                        })
                    }
                }, error => {
                    console.log(error)
                });
            });

            /////////////////////////////////////////////////////////////////////////////////////
            ///// UPDATE MONSTERS
            /////////////////////////////////////////////////////////////////////////////////////
            data.Monster.forEach(monster => {
                webservices.GetMonsters({
                    projectId: projectId,
                    monsterId: (isNaN(monster.id)) ? 0 : monster.id
                }, result => {
                    if (result == 0) {
                        webservices.Insert(
                            {
                                table: "monsters",
                                eventData: monster.Pos,
                                data: {
                                    projectId: create_query(projectId),
                                    name: create_query(monster.Meta.Name),
                                    description: create_query(monster.Meta["Ondeath Description"]),
                                    cardTypeId: create_query(find_index(data.CardTypes, element => { return element == monster.Data.Type }) + 1),
                                    maxHealth: create_query((monster.Data.MaxHealth == undefined) ? 0 : monster.Data.MaxHealth),
                                    trait: create_query((monster.Data.MonsterTrait == undefined) ? 0 : monster.Data.MonsterTrait)
                                }
                            }, err => { console.log(err) }, "add-monster"
                        );
                    } else if (!monster.Deleted) {
                        // UPDATE
                        console.log("UPDATE" + result.id)
                        if (result.name != monster.Meta.Name) {
                            UpdateScript(result.name, monster.Meta.Name,0);
                        }
                        webservices.Update({
                            table: "monsters",
                            where: "id ='" + result.id + "'",
                            changes:
                                [
                                    { name: "name", value: (monster.Meta.Name) },
                                    { name: "description", value: (monster.Meta["Ondeath Description"]) },
                                    { name: "maxHealth", value: ((monster.Data.MaxHealth == undefined) ? 0 : monster.Data.MaxHealth) },
                                    { name: "trait", value: ((monster.Data.MonsterTrait == undefined) ? 0 : monster.Data.MonsterTrait) },
                                    { name: "cardTypeId", value: (find_index(data.CardTypes, element => { return element == monster.Data.CardType }) + 1) }
                                ]
                        },  err => { console.log(err) },err => { console.error(err) });
                    } else {
                        webservices.Delete("monsters", [{ name: "id", value: result.id }], (res) => {
                            Sheets.ClearCells(GetCurrentProject().file, "Monster!A" + monster.Pos[1] + ":K" + monster.Pos[1]);
                        })
                    }
                })
            });
            /////////////////////////////////////////////////////////////////////////////////////
            ///// ADD REWARDS
            /////////////////////////////////////////////////////////////////////////////////////
            webservices.Clear("monsterRewards", (e, v) => {
                data.Rewards.forEach(reward => {
                    if (reward.MonsterId == undefined) return;
                    webservices.Insert(
                        {
                            table: "monsterRewards",
                            eventData: undefined,
                            data: {
                                cardId: create_query(reward.MonsterId),
                                rewardTypeId: create_query(find_index(["Currency", "Armor", "Health", "Card"], element => { return element == reward.Reward }) + 1),
                                value: create_query(reward.Value)
                            }
                        }, err => {
                            console.log(err)
                        }, "add-monster-reward"
                    );
                })
            });
        })
    }
}
document.addEventListener("add-monster", result => {
    if (result.detail.result.insertId != undefined) {
        console.log(GetCurrentProject())
        Sheets.UpdateSheetCell(GetCurrentProject().file, "Monster!B" + result.detail.pos[1], [[result.detail.result.insertId]])
        AddScriptToDB(0, result.detail.result.insertId)
    }
})
document.addEventListener("add-card", result => {
    if (result.detail.result.insertId != undefined) {
        console.log(GetCurrentProject())
        console.log("added script")
        Sheets.UpdateSheetCell(GetCurrentProject().file, "Cards!B" + result.detail.pos[1], [[result.detail.result.insertId]])
        AddScriptToDB(1, result.detail.result.insertId)
    }
})

function Spinner(element) {
    var el = document.createElement("div");
    el.classList.add("d-flex");
    el.classList.add("justify-content-center");
    el.classList.add("hub-spinner");
    el.innerHTML = ' <div class="spinner-grow text-primary" role="status"> <span class="sr-only">Loading...</span> </div><div class="spinner-grow text-danger" role="status"> <span class="sr-only">Loading...</span> </div><div class="spinner-grow text-warning" role="status"> <span class="sr-only">Loading...</span> </div><div class="spinner-grow text-success" role="status"> <span class="sr-only">Loading...</span> </div>'
    element.appendChild(el);
}


function ClearCache() {
    var currentWindow = remote.getCurrentWindow();
    const ses = currentWindow.webContents.session;
    var store = new Store();
    store.clear();

    ses.clearStorageData(() => {
        location.reload();
    });

}


module.exports.toTitleCase = (phrase) => {
    return phrase
        .toLowerCase()
        .split(' ')
        .map(word => word.charAt(0).toUpperCase() + word.slice(1))
        .join(' ');
};
function OnCookiesLoaded(callback) {
    window.addEventListener(COOKIES_LOADED, function (event) {
        callback(THEHUB.project);
    });
}
module.exports.ParseAppArguments = ParseAppArguments;
module.exports.LoadProjectFromCookies = LoadProjectFromCookies;
module.exports.ValidateProjectAndLoad = ValidateProjectAndLoad;
module.exports.OnCookiesLoaded = OnCookiesLoaded;
module.exports.OnProjectValidated = OnProjectValidated;
module.exports.OnProjectValidationError = OnProjectValidationError;
module.exports.OpenExternalProgram = OpenExternalProgram;
module.exports.RunLuaCheck = RunLuaCheck;
module.exports.LogToResultConsole = LogToResultConsole;
module.exports.WriteObjectToFile = WriteObjectToFile;
module.exports.WriteToFile = WriteToFile;
module.exports.GetCurrentProject = GetCurrentProject;
module.exports.NotifSuccess = NotifSuccess;
module.exports.NotifInfo = NotifInfo;
module.exports.NotifFail = NotifFail;
module.exports.ValidateIfCardsHaveScript = ValidateIfCardsHaveScript;
module.exports.Spinner = Spinner;
module.exports.ClearCache = ClearCache;