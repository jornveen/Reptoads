const fs = require('fs');
const path = require('path');
let folders =  []
let root = "";
module.exports.SetRoot = function(top){
    root = top;
}

module.exports.GetPathTo = function(object){
    return makePath() + "/"+ object;
}
module.exports.UpInspect = function(folderPath)
{
    folders.push(folderPath);
    fs.readdir(makePath(),{withFileTypes:true},OnFoldersLoaded);
}
module.exports.Jump = (folderPath,root)=>{
    folders = root.split(",")
    fs.readdir(path.resolve(folderPath),{withFileTypes:true},OnFoldersLoaded); 
}
module.exports.DownInspect = function(folderPath)
{
    folders.pop();
    fs.readdir(makePath(),{withFileTypes:true},OnFoldersLoaded);
}

module.exports.PathExists = (p)=>{
    return fs.existsSync(path.resolve(p));
}


function LoadModels(config,callback){
    let dir = path.join(root,'Client',config.clientConfig.assets.models);
    fs.readdir(dir,{withFileTypes:true},callback);
}

module.exports.LoadModels = LoadModels;

function makePath(){
    var returnPath = root;
    folders.forEach((v,i)=>{
        returnPath += "/" + v;
    });
    return returnPath;
}

function OnFoldersLoaded(err,files)
{
    console.log(err)
    //<li class="list-group-item">Cras justo odio</li>
    var folderList = document.getElementById("InspectrList");
    folderList.innerHTML = "";
    var folderArray = [];
    var filesArray = [];
    files.forEach(element => {
        if(element.isDirectory()){
            folderArray.push(element.name);
        }else{
            if(element.name !== "project.hub" && element.name !== "sceneeditor.blend")
                filesArray.push(element.name);
        }
    });
    var main = makePath();
    folderList.insertAdjacentHTML("beforeend",'<li class="file-inspector-item list-group-item" >'+main.replace('//','/')+'</li>');
    if(path.resolve(main) != path.resolve(root) ){
        folderList.insertAdjacentHTML("beforeend",'<li class="file-inspector-item list-group-item" ><p data-type="up"  data-path="'+main+'/.."><i class="fas fa-reply"></i> Up</p></li>');
    }
    folderArray.sort()
    folderArray.forEach(element => {
        folderList.insertAdjacentHTML("beforeend",'<li class="file-inspector-item  d-flex justify-content-between list-group-item"><p data-path="'+element+'"><i class="far fa-folder"></i> '+element+'</p> <div class="btn-group justify-content-end btn-group-sm" role="group"><button type="button" data-toggle="tooltip" data-placement="top" title="Open Folder" data-path="'+makePath()+"/"+element+'" class="btn open-folder btn-warning"><i class="far fa-folder-open"></i></button></div></li>')
    })
    filesArray.sort()
    filesArray.forEach(element => {
        const regex = /(\.lua)/gm;
        var result = element.match(regex);
        if(result !== undefined && result !== null){
            let revision = JSON.parse(fs.readFileSync(makePath()+"/"+element.replace(".lua",".meta")));
            folderList.insertAdjacentHTML("beforeend",'<li class="file-inspector-item list-group-item d-flex justify-content-between"><span class="badge badge-info">#'+revision+'</span> <p data-file="'+element+'" class="open-script-li"><i class="fas fa-file-code"></i> '+element+'</p> <div class="btn-group justify-content-end btn-group-sm" role="group"><button type="button" data-toggle="tooltip" data-placement="top" title="Validate Script" data-path="'+makePath()+"/"+element+'" class="btn validate-script btn-secondary"><i class="far fa-check-square"></i></button><button type="button" data-toggle="tooltip" data-placement="top" title="Open in Code" data-path="'+makePath()+"/"+element+'" class="btn open-file btn-primary"><i class="fas fa-external-link-square-alt"></i></button><button type="button" data-toggle="tooltip" data-placement="top" title="Open Folder" data-path="'+makePath()+'" class="btn btn-warning open-folder"><i class="far fa-folder-open"></i></button></div></li>')    
        }else if(element.includes(".glb")){ 
            folderList.insertAdjacentHTML("beforeend",'<li class="file-inspector-item list-group-item d-flex justify-content-between"><p data-file="'+element+'"><i class="far fa-file"></i> '+element+'</p> <div class="btn-group justify-content-end btn-group-sm" role="group"><button type="button" data-toggle="tooltip" data-placement="top" title="Open in Scene Editor" data-path="'+element+'" class="btn open-file btn-primary"><i class="fas fa-draw-polygon"></i></button><button type="button" data-toggle="tooltip" data-placement="top" title="Open Folder" data-path="'+makePath()+'" class="btn btn-warning open-folder"><i class="far fa-folder-open"></i></button></div></li>')
        }else if(!element.includes(".meta")){
            folderList.insertAdjacentHTML("beforeend",'<li class="file-inspector-item list-group-item d-flex justify-content-between"><p data-file="'+element+'"><i class="far fa-file"></i> '+element+'</p> <div class="btn-group justify-content-end btn-group-sm" role="group"><button type="button" data-toggle="tooltip" data-placement="top" title="Open in Code" data-path="'+makePath()+"/"+element+'" class="btn open-file btn-primary"><i class="fas fa-external-link-square-alt"></i></button><button type="button" data-toggle="tooltip" data-placement="top" title="Open Folder" data-path="'+makePath()+'" class="btn btn-warning open-folder"><i class="far fa-folder-open"></i></button></div></li>')
        }
    });
    if(filesArray.length == 0 && folderArray.length == 0)
    folderList.insertAdjacentHTML("beforeend",'<li class="file-inspector-item list-group-item d-flex justify-content-center"> - No Content - </div></li>')

}



function LogWhenFailed(err, data)
{
    if(err){
        console.error(err);
        return;
    }
    console.log(data.toString());
}