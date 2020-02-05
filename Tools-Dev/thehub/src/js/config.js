const fs = require('fs');
const nodePath = require('path');
const config = JSON.parse(fs.readFileSync('./config.json'));



function SaveConfig(config){
    fs.writeFileSync('./config.json', JSON.stringify(config, null, 4), function(err) {
        if (err) {
            console.log(err);
        }
    });
}

module.exports.SaveConfig = SaveConfig;
module.exports.config = config;


function MakeProjectDir(project){
    return config.projects + "/"+ project;
}

function MakeAssetDir(project) {
    return {
        "server": {
            "root":MakeProjectDir(project) + "/" + config.folders.server,
            "assets": MakeProjectDir(project) + "/" + config.folders.server + "/" +config.folders.assets,
        },
        "client": {
            "root":MakeProjectDir(project) + "/" + config.folders.client,
            "assets":MakeProjectDir(project) + "/" + config.folders.client +  "/" +config.folders.assets
        }
    };
};

/**
 * 
* @param {string} projectname projectname
 * @param {string} where server or client
 * @param {string} type asset or root
 * @param {string} fileOrFolder file or folder name
 * @return {bool} on success true on failure false
 */
function ValidateFileOrFolder(projectname,where,type,fileOrFolder){
    let folder = "";
    let basis = module.exports.MakeAssetDir(projectname);
    if(where === "server" || where === "client"){
        folder = basis[where][type];
        folder = nodePath.resolve(folder);
        return fs.existsSync(folder+"/"+fileOrFolder);
    }else{
        console.error("[config.js] cannot deal with the where keyword needs to be client or server");
    }
}

module.exports.MakeProjectDir = MakeProjectDir;
module.exports.MakeAssetDir = MakeAssetDir;
module.exports.ValidateFileOrFolder = ValidateFileOrFolder;

