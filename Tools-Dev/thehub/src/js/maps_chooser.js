const fs = require('fs');
const path = require('path');
const { remote } = require('electron')
const { dialog } = remote;

let folders = [];
let root = "";

var maps = [];

module.exports.SaveMaps = (callback) => {
    var valid = true;
    maps.forEach(map => {
        if (map.name == this.tempName) {
            valid = false;
            
        }
    });

    if (valid) {
        if (this.tempName != undefined && this.tempName != "") {
            this.selectedMap.name = this.tempName;  
            this.tempName = ""; 
        }

        fs.writeFile(this.path, JSON.stringify(maps, null, 4), function (err) {
            if (err) {
                dialog.showErrorBox("Error: Couldn't save map", "The map could not be saved. Err: " + err);
                callback(false);
            } else {
                callback(true);
            }
        });
    } else {
        dialog.showErrorBox("Error: Couldn't save map", "This map name already exists.");
        callback(false);
    }
}

module.exports.LoadMaps = (path) => {

    folders = [];
    this.selectedMap = null;
    this.path = path;

    maps = JSON.parse(fs.readFileSync(path, 'utf8'));

    var MapForm = document.getElementById("MapForm");
    MapForm.innerHTML = "";

    module.exports.UpdateMapsList();

    module.exports.UpInspect("");

    document.getElementById("InspectrList").classList.add("hide");
}

module.exports.UpdateMapsList = () => {
    document.getElementById("new-map-div").style.display = "block";
    var mapsListHTML = document.getElementById("MapsList");
    mapsListHTML.innerHTML = "";

    maps.forEach(map => {
        mapsListHTML.insertAdjacentHTML("beforeend", '<li class="map-list-item list-group-item d-flex justify-content-between" map-name="' + map.name + '"><p><i class="fas fa-layer-group"></i> ' + map.name + '</p></li>');
    });
}

module.exports.CreateNewMap = () => {
    var name = "New Map";

    maps.push({
        name: name,
        scenes: []
    });

    module.exports.UpdateMapsList();
    module.exports.SaveMaps();
}

module.exports.OpenMap = (name) => {
    document.getElementById("new-map-div").style.display = "none";

    var mapsListHTML = document.getElementById("MapsList");
    mapsListHTML.innerHTML = "";

    var MapForm = document.getElementById("MapForm");

    var thismap;

    for (var x = 0; x < maps.length; x++) {
        if (maps[x].name == name) {
            thismap = maps[x];
            break;
        }
    }

    if (thismap == null) {
        dialog.showErrorBox("Error: Can't load map", "The map name was not found.");
        return;
    }

    this.selectedMap = thismap;
    module.exports.UpInspect("");

    MapForm.insertAdjacentHTML("beforeend", '<div class="form-group"><button type="button" class="btn btn-primary map-back"><i class="fas fa-reply"></i>Back</button><button type="button" class="btn btn-primary map-save" style="float: right" ><i class="fas fa-check"></i>Save</button></div><div class="form-group"> <label for="exampleFormControlInput1">Map Name</label> <input type="text" value="' + thismap.name + '" class="form-control" id="map-name"> </div> <div class="form-group"> <label for="exampleFormControlTextarea1">Scenes</label> <ul class="list-group" id="SceneList"> </ul> </div>');

    document.getElementById('map-name').addEventListener("input", (event) => {
        this.tempName = event.target.value;
    });

    module.exports.UpdateSceneList();

    document.getElementById("InspectrList").classList.remove("hide");
}

module.exports.SetMapName = () => {

}

module.exports.UpdateSceneList = () => {
    var sceneList = document.getElementById("SceneList");
    sceneList.innerHTML = "";
    this.selectedMap.maps.forEach(scene => {
        sceneList.insertAdjacentHTML("beforeend", '<li class="map-list-item list-group-item d-flex justify-content-between" scene-name="' + scene + '"><p><i class="far fa-file"></i> ' + scene + '</p></li>');
    });
}

module.exports.AddSceneToMap = (scene) => {
    this.selectedMap.maps.push(scene);
    module.exports.UpdateSceneList();
}

module.exports.RemoveSceneFromMap = (scene) => {
    this.selectedMap.maps = this.selectedMap.maps.filter(item => item !== scene);
    module.exports.UpdateSceneList();
}

module.exports.SetRoot = function (top) {
    root = top;
}

module.exports.GetPathTo = function (object) {
    return makePath() + "/" + object;
}
module.exports.UpInspect = function (folderPath) {
    folders.push(folderPath);
    fs.readdir(makePath(), { withFileTypes: true }, (err, files) => {
        OnFoldersLoaded(err, files, this.selectedMap);
    });
}
module.exports.DownInspect = function (folderPath) {
    folders.pop();
    fs.readdir(makePath(), { withFileTypes: true }, OnFoldersLoaded);
}


function makePath() {
    returnPath = root;
    folders.forEach((v, i) => {
        returnPath += "/" + v;
    });
    return returnPath;
}

function OnFoldersLoaded(err, files, selectedMap) {

    //<li class="list-group-item">Cras justo odio</li>
    var folderList = document.getElementById("InspectrList");
    folderList.innerHTML = "";
    var folderArray = [];
    var filesArray = [];
    files.forEach(element => {
        if (element.isDirectory()) {
            folderArray.push(element.name);
        } else {
            if (element.name !== "project.hub" && element.name !== "sceneeditor.blend")
                filesArray.push(element.name);
        }
    });
    var main = makePath();
    if (path.resolve(main) != path.resolve(root)) {
        folderList.insertAdjacentHTML("beforeend", '<li class="file-inspector-item list-group-item" ><p data-type="up"  data-path="' + main + '/.."><i class="fas fa-reply"></i> Up</p></li>');
    }
    folderArray.sort()
    folderArray.forEach(element => {
        folderList.insertAdjacentHTML("beforeend", '<li class="file-inspector-item  d-flex justify-content-between list-group-item"><p data-path="' + element + '"><i class="far fa-folder"></i> ' + element + '</p> <div class="btn-group justify-content-end btn-group-sm" role="group"><button type="button" data-toggle="tooltip" data-placement="top" title="Open Folder" data-path="' + makePath() + "/" + element + '" class="btn open-folder btn-warning"><i class="far fa-folder-open"></i></button></div></li>')
    })
    filesArray.sort()
    filesArray.forEach(element => {
        const regex = /(\.glb)/gm;
        var result = element.match(regex);
        if (result !== undefined && result !== null && selectedMap != null) {

            var btn = "";

            selectedMap.maps.forEach(sceneName => {
                if (sceneName == element) {
                    btn = '<button type="button" data-toggle="tooltip" data-placement="top" title="Remove from map" scene-name="' + element + '" class="btn remove-scene btn-danger"><i class="fas fa-minus"></i></button>';
                }
            });

            if (btn == "") {
                btn = '<button type="button" data-toggle="tooltip" data-placement="top" title="Add to map" scene-name="' + element + '" class="btn add-scene btn-primary"><i class="fas fa-plus"></i></button>';
            }


            folderList.insertAdjacentHTML("beforeend", '<li class="file-inspector-item list-group-item d-flex justify-content-between"><p data-file="' + element + '" class="open-script-li"><i class="far fa-file"></i> ' + element + '</p><div class="btn-group justify-content-end btn-group-sm" role="group">' + btn + '</div></li>')
        }
    });
    if (filesArray.length == 0 && folderArray.length == 0)
        folderList.insertAdjacentHTML("beforeend", '<li class="file-inspector-item list-group-item d-flex justify-content-center"> - No Content - </div></li>')

}

function LogWhenFailed(err, data) {
    if (err) {
        console.error(err);
        return;
    }
    console.log(data.toString());
}