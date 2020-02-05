const fs = require('fs');
const p = require('path');

let ClientConfig = {}, 
ServerConfig = {},
ProjectConfig = {};

module.exports.ClientConfig = ClientConfig;
module.exports.ServerConfig = ServerConfig;
module.exports.ProjectConfig = ProjectConfig;

module.exports.LoadProjectConfig = path => {
    module.exports.ClientConfig = JSON.parse(fs.readFileSync(p.join(path, "Client/config.json")));
    module.exports.ServerConfig = JSON.parse(fs.readFileSync(p.join( path, "Server/config.json")));
    module.exports.ProjectConfig = JSON.parse(fs.readFileSync(p.join(path, "project.hub")));
}





module.exports.MakeProjectDir = project =>{
    return config.projects + "/"+ project;
};

module.exports.SaveConfig = config => {

    var appDir = p.dirname(require.main.filename);
   var configfile = p.join(appDir,'../../../',config.path, "project.hub");
    fs.writeFileSync(configfile, JSON.stringify(config, null, 4), function(err) {
        if (err) {
            console.log(err);
        }
    });
}