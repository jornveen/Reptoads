///////////////// UPDATES /////////////////////
var Client = require('ftp')
var fs = require('fs');
var path = require('path')
var config = require('../js/config.js');
const unzip = require('unzip');
const webservices = require("./webservices").Webservice()
const validate = require("./validate-project");
const child_process = require('child_process');

function Merge(script){
    fs.writeFileSync(script.path + ".lua.tmp", script.content);
    child_process.execFile("p4merge", [script.path + ".lua", script.path + ".lua.tmp"]).on("close", (e) => {
        fs.writeFileSync(script.path + ".meta", script.revision);
        fs.unlinkSync(script.path + ".lua.tmp");
    });    
}

function Updater() {

    var client = new Client();
    function checkForUpdates(list, callback) {
        var result = {
            client: {
                version: 0,
                updateNeeded: false
            },
            server: {
                version: 0,
                updateNeeded: false
            }
        }
        function fillResult(file, type) {
            var version = file.date.getTime();
            if (config.config.apps[type].currentversion < version || !fs.existsSync("db/" + file.name)) {
                result[type].updateNeeded = true;
                result[type].version = version;
                console.log(22);
            } else {
                result[type].updateNeeded = false;
                result[type].version = version;
            }
        }
        list.forEach((file, k) => {
            if (file.name == "Client.zip") {
                fillResult(file, "client");
            } else
                if (file.name == "Server.zip") {
                    fillResult(file, "server");
                }
        });
        callback(result);
    }


    async function UpdateFiles(filename, callback, error) {
        if (fs.existsSync("db/" + filename)) {
            fs.readdir(config.config.projects, function (err, items) {
                if (err) throw err;
                var max = items.length;
                var counter = 0;
                items.forEach((projectName, k) => {
                    fs.createReadStream("db/" + filename)
                        .pipe(unzip.Extract({
                            path: config.config.projects + "/" + projectName
                        })).on('finish', () => {
                            counter++;
                            callback((counter / max) * 100)
                        });
                });
            });
        } else {
            error();
        }
    }


    async function Download(filename, callback) {
        client.get(filename, function (err, stream) {
            if (err) throw err;
            stream.once('close', function () { client.end(); });
            stream.pipe(fs.createWriteStream("db/" + filename)).on('finish', () => {
                UpdateFiles(filename, callback, () => {
                    console.error("Could not find db/" + filename);
                });
            });
            // update all projects:
        });
    }


    function ValidateConfig() {
        return config.config.repo.host != "" &&
            config.config.repo.user != "" &&
            config.config.repo.password != "" &&
            config.config.repo.port != "" &&
            config.config.repo.secure != null;
    }

    async function Connect(callback, error) {
        if (ValidateConfig()) {

            client.on('ready', () => {
                callback(client);
            });
            client.on('error', () => {
                error();
            })
            client.connect({
                host: config.config.repo.host,
                user: config.config.repo.user,
                password: config.config.repo.password,
                port: config.config.repo.port,
                secure: config.config.repo.secure,
                secureOptions: { rejectUnauthorized: false }
            })
        } else {
            error();
        }
    }


    async function UpdateApp(onUpdate, error) {
        Connect((c) => {
            c.list(function (err, list) {
                if (err) throw err;
                checkForUpdates(list, onUpdate);
                c.end();
            });
        }, error);
    }


    function GetLatest(
        type,
        onUpdateSuccess,
        onUpdatedNeeded,
        onError
    ) {
        const Store = require("electron-store");
        let store = new Store();
        let project = store.get("last-project-obj", {});
        if (type == 'server' || type == 'client') {
            UpdateApp((result) => {
                if (result[type].updateNeeded) {
                    Download(THEHUB.toTitleCase(type) + ".zip", (progress) => {
                        if (progress >= 100) {
                            onUpdateSuccess();
                        }
                    });
                } else {
                    onUpdatedNeeded()
                }
                config.config.apps[type].lastcheck = new Date().getTime();
                config.config.apps[type].latestversion = result[type].version;
                config.config.apps[type].currentversion = result[type].version;
                config.SaveConfig(config.config);
            }, onError);
        } else if (type == 'scripts') {
            webservices.Start(() => {
                validate.ValidateScripts(project, (revisions) => {
                    revisions.forEach(revision => {
                        Merge(revision.script);
                    });
                    onUpdateSuccess();
                });
            });
        } else if (type == "config") {
            validate.ValidateConfig(project, () => {
                onUpdateSuccess();
             }, (type, configSource) => {
                if (type == 0)// server
                {
                    let projectDir = path.join(config.MakeProjectDir(project.realname), "Server", "config.json");
                    fs.writeFileSync(projectDir, JSON.stringify(configSource));
                    onUpdateSuccess();
                } else {
                    let projectDir = path.join(config.MakeProjectDir(project.realname), "Client", "config.json");
                    fs.writeFileSync(projectDir, JSON.stringify(configSource));
                    onUpdateSuccess();
                }
            });
        }
    }
    var doneSync = 0;
    document.addEventListener("sync-step-done",()=>{
        doneSync++;
        if(doneSync >= 3){
            document.dispatchEvent(new Event("hub-sync-done"));
        }
    })
    function Sync() {
        ///////////////////////////////////////////////////////////
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
        /////////////////////////////////////////////////////////

        const Store = require("electron-store");
        let store = new Store();
        let project = store.get("last-project-obj", {});
        webservices.Start(() => {
            ///////////////////////////////////////////////////////////
            /// Sync Config's
            ///////////////////////////////////////////////////////////
            let typeDesc = ["server", "client"]
            validate.ValidateConfig(project, () => { 
                document.dispatchEvent(new Event("sync-step-done"));
            },
                (type, c) => {
                    let path = config.MakeAssetDir(project.realname)[typeDesc[type]].root + "/config.json";
                    let currentConfig = JSON.parse(fs.readFileSync(path));
                    webservices.UpdateConfig(currentConfig, type);

                });
            ///////////////////////////////////////////////////////////
            /// Sync Scripts's
            ///////////////////////////////////////////////////////////
            validate.ValidateScripts(project, (revisions) => {
                if (revisions.length != 0) {
                    console.log(revisions)
                    revisions.forEach(revision =>{
                        if(revision.revision){
                            if(revision.sourceRevision < revision.script.revision){
                                Merge(revision.script);
                            }
                        }else if(revision.source && revision.sourceRevision == revision.script.revision){
                            webservices.UpdateScript(revision);
                            let version = 1 + revision.script.revision;
                            fs.writeFileSync(revision.script.path + ".meta", version);
                        }
                    });
                }
                document.dispatchEvent(new Event("sync-step-done"));
            });
        });
    }

    return {
        UpdateApp: UpdateApp,
        Download: Download,
        UpdateFiles: UpdateFiles,
        GetLatest: GetLatest,
        Sync: Sync
    };
}

module.exports.Updater = Updater;
