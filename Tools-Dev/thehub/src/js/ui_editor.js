const fs = require('fs');
const p = require('path');
const config = require('./project_config.js');
const { remote } = require('electron');
const Mousetrap = require('mousetrap');

const dialog = remote.dialog;

var parsedUI;
var currentUIFile = {
    name: "",
    location: ""
};
var selectedElement;

var UI = undefined;
var AssetLoader = undefined;
var Project = undefined;

var CurrentScene = [];
var Changes = []



function AddChanges(changes) {
    var change = {}
    Object.assign(change, changes)
    Changes.push(change)
}

function Reset() {
    module.exports.LoadUIFile(currentUIFile.location);
    Changes = [];
}


function GoBack(changes) {
    var back = {}
    if (changes == undefined) {
        back = Changes.pop();
    } else {
        back = changes;
    }
    if (back == undefined) return;
    var i = 0;
    UI.displayObjects.forEach(element => {
        if (element.Id == back.Id) {
            Object.keys(UI.displayObjects[i]).forEach(key => {
                UI.displayObjects[i][key] = back[key];
            })
            selectedElement = UI.displayObjects[i].text;
            module.exports.LoadProperties(selectedElement);
            return true;
        }
        i++;
    })
    UI.shouldReDraw = true;
}

Mousetrap.bind(['command+r', 'ctrl+r'], () => {
    Reset();
    // return false to prevent default behavior and stop event from bubbling
    return false
})
Mousetrap.bind(['command+z', 'ctrl+z'], () => {
    GoBack();
    // return false to prevent default behavior and stop event from bubbling
    return false
})


module.exports.Reset = Reset;
module.exports.GoBack = GoBack;

function CreateJsonFromUI() {
    var json = {
        name: (currentUIFile == undefined) ? undefined : currentUIFile.name,
        elements: []
    };
    for (let element of UI.displayObjects) {
        console.log(element);
        if (!element.visible && element.del) continue;
        console.log(88)
        var temp = {
            name: element.name,
            tag: (element.tag == undefined || element.tag == null) ? "" : element.tag,
            script: "",
            position: {
                x: element.x,
                y: element.y,
                depth: element.depth ? element.depth : 0.00
            },
            size: {
                width: element.width,
                height: element.height
            },
            rotation: element.rotation,
            type: "",
            typable_data: {

            },
            children: []
        };
        console.log(element)
        if (element.type == "UIImage") {
            temp.type = "UIImage";
            temp.typable_data = {
                image: element.assetName,
                interactable: element.interactable,
                events: element.events
            }
        } else if (element.type == "text" || element.type == "UIText") {
            temp.type = "UIText";
            console.log(element)
            var colors = element.color.split(/[(),]+/);
            temp.typable_data = {
                text: element.text,
                horizontal_alignment: element.horizontal_alignment,
                vertical_alignment: element.vertical_alignment,
                font_path: element.font_path,
                font_size: element.font_size,
                interactable: element.interactable,
                color: {
                    r: parseInt(colors[1]) / 255,
                    g: parseInt(colors[2]) / 255,
                    b: parseInt(colors[3]) / 255,
                    a: parseInt(colors[4])
                }
            }
            if (element.isinputbox) temp.typable_data.input = element.input_data;
        }

        if (element.parent) {
            console.log(element.parent);
            var found = false;
            for (let index = 0; index < json.elements.length; index++) {
                if (json.elements[index].name == element.parent.name) {
                    found = true;
                    json.elements[index].children.push(temp)
                }
            }
            /*
            if (!found) {
                elements.push(element);
            }*/
        } else {
            json.elements.push(temp);
        }
    }
    return json
}

function CreateList() {
    var sceneList = document.getElementById("scene-list");
    sceneList.innerHTML = ""
    var elements = UI.displayObjects.slice();
    var i = 0;
    while (elements.length > 0) {
        var element = elements.shift();
        sceneList.insertAdjacentHTML("beforeend", `
        <div class="form-row">
        <div class="form-group col-md-12">
        <button class='btn btn-warning btn-block' data-index="${i}" id="scene-${element.Id}">${element.name}</button>
        </div>
    </div>     
          `);
        sceneList.querySelector("#scene-" + element.Id).addEventListener("click", function () {
            var elementObj = UI.displayObjects.slice();
            selectedElement = elementObj[parseInt(this.getAttribute("data-index"))];
            module.exports.LoadProperties(selectedElement);
        })
        i++;
    }
}
module.exports.ResetChanges = function () {
    Changes = [];
}
module.exports.AddChanges = AddChanges
module.exports.CreateJsonFromUI = CreateJsonFromUI

module.exports.LoadEditor = (ui, assetLoader, project) => {
    UI = ui;
    AssetLoader = assetLoader;
    Project = project;

    fs.readdir(p.join(Project.path, '/Client/data/Textures'), { withFileTypes: true }, (err, files) => {
        var folderArray = [];
        var filesArray = [];
        files.forEach(element => {
            if (element.isDirectory()) {
                folderArray.push(element.name);
            } else {
                filesArray.push(element.name);
            }
        });
        filesArray.sort()
        filesArray.forEach(element => {
            if (element.includes(".png") || element.includes(".jpg")) {
                AssetLoader.add.image(p.join(Project.path, '/Client/data/Textures', element));
            }
        });

        AssetLoader.load(function () {
            console.log("Assets loaded!");
        });
    });

    config.LoadProjectConfig(Project.path);
}

module.exports.LoadUIFile = (file) => {
    UI.reset();

    var data = fs.readFileSync(file, 'utf8');
    parsedUI = JSON.parse(data);

    currentUIFile = {
        name: parsedUI.name,
        location: file
    };

    console.log(currentUIFile);
    module.exports.LoadUI(parsedUI)
    CreateList();

}



var freeElements = []

function CreateSprite(UIelement) {
    var sprite = UI.createSprite(p.join(Project.path, '/Client/data/Textures', UIelement.typable_data.image));
    sprite.alpha = 1;
    sprite.name = UIelement.name;
    sprite.x = UIelement.position.x;
    sprite.y = UIelement.position.y;
    sprite.depth = UIelement.position.depth ? UIelement.position.depth : 0.00;
    sprite.height = UIelement.size.height;
    sprite.width = UIelement.size.width;
    sprite.pivot.x = 0;
    sprite.pivot.y = 1;
    sprite.assetName = UIelement.typable_data.image;
    sprite.anchor.x = ThreeUI.anchors.left;
    sprite.anchor.y = ThreeUI.anchors.bottom
    sprite.interactable = UIelement.typable_data.interactable;
    sprite.tag = (UIelement.tag == undefined) ? null : UIelement.tag;
    var events = [];
    var idCount = 0;
    UIelement.typable_data.events.forEach(element => {
        element.id = idCount++;
        events.push(element);
    });
    sprite.events = events;

    sprite.onMousedown(() => {
        selectedElement = sprite;
        module.exports.LoadProperties(sprite);
    });

    sprite.onMouseup(() => {
        selectedElement = sprite;
        module.exports.LoadProperties(sprite);
    });
    return sprite;
}

function CreateTextElement(UIelement, callback) {
    var asset = CreateText(UIelement);
    createImageBitmap(asset).then(function (value) {

        var text = UI.createSprite(value);
        text.type = "UIText";
        text.text = UIelement.typable_data.text;
        text.name = UIelement.name;
        text.tag = null;
        text.anchor.x = ThreeUI.anchors.left;
        text.anchor.y = ThreeUI.anchors.bottom;
        text.pivot.x = 0;
        text.pivot.y = 1;
        text.textX = asset.textX;
        text.textY = asset.textY;
        text.width = UIelement.size.width;
        text.height = UIelement.size.height;
        text.visible = true;
        text.del = false;
        text.color = 'rgba(' + UIelement.typable_data.color.r * 255 + ',' + UIelement.typable_data.color.g * 255 + ',' + UIelement.typable_data.color.b * 255 + ',' + UIelement.typable_data.color.a + ')';
        text.tag = (UIelement.tag == undefined) ? null : UIelement.tag;
        text.horizontal_alignment = UIelement.typable_data.horizontal_alignment
        text.vertical_alignment = UIelement.typable_data.vertical_alignment
        text.font_path = UIelement.typable_data.font_path
        text.font_size = UIelement.typable_data.font_size
        text.interactable = UIelement.typable_data.interactable
        text.isinputbox = false;
        text.x = UIelement.position.x;
        text.y = UIelement.position.y;
        text.depth = UIelement.position.depth;
        text.onMousedown(() => {
            if (!isUILock) {
                selectedElement = text;
                module.exports.LoadProperties(text);
            }
            text.isLocked = isUILock;
        });

        text.onMouseup(() => {
        });
        freeElements.push(text)
        CurrentScene.push(text);
        AddChanges(text);
        callback(text);
    })
}


module.exports.LoadUI = function (data) {
    freeElements = [];
    data.elements.forEach(UIelement => {
        //console.log(JSON.stringify(UIelement));

        if (UIelement.type == "UIImage") {

            var sprite = CreateSprite(UIelement);
            sprite.children = [];
            freeElements.push(sprite);
            module.exports.AddChild(sprite, UIelement.children);
            AddChanges(sprite)
            CurrentScene.push(sprite);
        } else if (UIelement.type == "UIText") {
            CreateTextElement(UIelement, (text) => {
                text.children = [];
                freeElements.push(text);
                module.exports.AddChild(text, UIelement.children);
                AddChanges(text)
                CurrentScene.push(text);
            });
        }
    });
}



module.exports.LoadProperties = (target) => {
    var propertiesList = document.getElementById("tools-list");
    console.log(target)
    if (target) selectedElement = target;
    else target = selectedElement;
    propertiesList.innerHTML = "";

    var text = "";
    var events = "";
    var children = [];
    if (target.children != undefined) {
        children = target.children
    }
    propertiesList.insertAdjacentHTML("beforeend", "<h3>Hierarchy</h3>");
    //attach
    propertiesList.insertAdjacentHTML("beforeend", `
    <div class="dropdown">
    <button class="btn btn-secondary dropdown-toggle" type="button" id="dropdownMenuButton" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
      Choose new parent
    </button>
    <div class="dropdown-menu" id="attach-to" aria-labelledby="dropdownMenuButton">
    </div>
    </div>        
      `);
    freeElements.forEach(el => {
        if (target.Id == el.Id) return;
        propertiesList.querySelector("#attach-to").insertAdjacentHTML("beforeend", `<a class="dropdown-item" id="attach-${el.Id}"href="#">${el.name}</a>`);
        document.querySelector("#attach-" + el.Id).addEventListener("click", function () {
            target.x = 0;
            target.y = 0;
            if (target.parent != undefined) {
                for (let i = 0; i < target.parent.children.length; i++) {
                    if (target.parent.children[i].Id == target.Id) {
                        target.parent.children.splice(i, 1);
                    }
                }
            }
            target.parent = el;
            if (target.parent.children == undefined) {
                target.parent.children = [];
            }
            target.parent.children.push(target);
            module.exports.LoadProperties(target);
        });
    })



    if (target.parent != undefined) {
        propertiesList.insertAdjacentHTML("beforeend",
            `
        <div class="form-row">
            <div class="form-group col-md-12">
            <button class='btn btn-warning btn-block' id="element-${target.parent.Id}">Parent: ${target.parent.name}</button>
            </div>
        <hr stlye="margin-top:15px;margin-bottom:15;">
        <div class="form-group col-md-12">
        <button class='btn btn-danger btn-block' id="deparent-${target.parent.Id}">Detatch from Parent</button>

        </div>
        </div>
        `);
        document.querySelector("#element-" + target.parent.Id).addEventListener("click", function () {
            module.exports.LoadProperties(target.parent);
        });
        document.querySelector("#deparent-" + target.parent.Id).addEventListener("click", function () {
            target.x = 0;
            target.y = 0;
            for (let i = 0; i < target.parent.children.length; i++) {
                if (target.parent.children[i].Id == target.Id) {
                    target.parent.children.splice(i, 1);
                }
            }
            target.parent = undefined;
            module.exports.LoadProperties(target);
        });
    }

    if (target.children != undefined) {
        for (let i = 0; i < target.children.length; i++) {
            propertiesList.insertAdjacentHTML("beforeend",
                `
            <div class="form-row">
                <div class="form-group col-md-12">
                <button class='btn btn-info btn-block' id="element-${target.children[i].Id}">Child ${target.children[i].name}</button>
                </div>
            </div>`);
            document.querySelector("#element-" + target.children[i].Id).addEventListener("click", function () {
                console.log(target.children[i].name);
                module.exports.LoadProperties(target.children[i]);
            })

        }
    }
    text += "<hr>"
    if (selectedElement.type == "UIText") {

        text += `
            <div class="form-row">
                <div class="form-group col-md-12">
                    <label for="element-text">Text</label>
                    <input value="${selectedElement.text}" type="text" class="form-control properties-input" property="text">
                </div>
            </div>
            <div class="form-row">
            <div class="form-group col-md-12">
                <label for="element-text">color</label>
                <input value="${selectedElement.color}" type="text" class="form-control properties-input" property="color">
            </div>
        </div>
            <div class="form-row">
                <div class="form-group col-md-12">
                    <label for="element-text">Text Size</label>
                    <input value="${selectedElement.font_size}" type="number" class="form-control properties-input" property="textsize">
                </div>
            </div>
            <div class="form-row">
                <div class="form-group col-md-12">
                    <label for="element-text">Font</label>
                    <input value="${selectedElement.font_path}" type="text" class="form-control properties-input" property="font">
                </div>
            </div>
            
            <div class="form-row">
                <div class="form-group col-md-12">
                    <div class="form-row">
                        <div class="form-group col-md-12">
                            <label for="element-text">Is InputBox</label>
                            <input type="checkbox" class="form-control" id="isinputbox" style="float: right;" ${selectedElement.isinputbox ? "checked" : ""}>
                        </div>
                    </div>
                    <div id="input-properties" ${selectedElement.isinputbox ? "" : "hidden"}>
                        <div class="form-row">
                            <div class="form-group col-md-12">
                                <label for="element-text">Validation</label>
                                <input value="${selectedElement.isinputbox ? selectedElement.input_data.validation : ""}" type="text" class="form-control properties-input" property="input-validation">
                            </div>
                        </div>
                        <div class="form-row">
                            <div class="form-group col-md-12">
                                <label for="element-text">Obscure_text</label>
                                <input type="checkbox" class="form-control" id="input-obscure" style="float: right;" ${selectedElement.isinputbox ? (selectedElement.input_data.obscure_text ? "checked" : "") : ""}>
                            </div>
                        </div>
                        <div class="form-row">
                            <div class="form-group col-md-12">
                                <label for="element-text">Interactable Tag</label>
                                <input value="${selectedElement.isinputbox ? selectedElement.input_data.interactable_tag : ""}" type="text" class="form-control properties-input" property="input-tag">
                            </div>
                        </div>
                    </div>
                </div>
            </div>
            `;
    }

    if (selectedElement.type == "UIImage") {
        events += `
        <div class="form-row">
        <div class="form-group col-md-12">
            <label for="element-text">Image</label>
            <input value="${selectedElement.assetName}" type="text" class="form-control properties-input" property="assetname">
        </div>
    </div>
        <div class="form-row">
            <div class="form-group col-md-12">
               <p>Is Interactable <input type="checkbox" class="form-control" id="interactable" style="float: right;"/></p>
                <p>Events <button id="show-add-event" style="float: right;" class="btn btn-primary"><i class="fas fa-plus"></i> Add</button></p>
                <ul class="list-group">

                    <a class="list-group-item list-group-item-action" data-toggle="collapse" href="#collapse1" role="button" aria-expanded="false" aria-controls="collapse1">
                        OnButtonPressed
                    </a>
                    <div>
                        ${module.exports.loadEvents(selectedElement, "OnButtonPressed")}
                    </div>

                    <a class="list-group-item list-group-item-action" data-toggle="collapse" href="#collapse2" role="button" aria-expanded="false" aria-controls="collapse2">
                        OnButtonReleased
                    </a>
                    <div>
                        ${module.exports.loadEvents(selectedElement, "OnButtonReleased")}
                    </div>

                    <a class="list-group-item list-group-item-action" data-toggle="collapse" href="#collapse3" role="button" aria-expanded="false" aria-controls="collapse3">
                        OnFocussed
                    </a>
                    <div>
                        ${module.exports.loadEvents(selectedElement, "OnFocussed")}
                    </div>

                    <a class="list-group-item list-group-item-action" data-toggle="collapse" href="#collapse4" role="button" aria-expanded="false" aria-controls="collapse4">
                        OnFocusLost 
                    </a>
                    <div>
                        ${module.exports.loadEvents(selectedElement, "OnFocusLost")}
                    </div>
                </ul>
            </div>
        </div>`;
    }

    const markup = `
        <div style="height: 900px;" class="list-group-item overflow-auto">
            <div class="form-row">
                <div class="form-group col-md-12">
                    <label>Type: ${selectedElement.type}</label></br>
                    <label>ID: ${selectedElement.Id}</label>
                </div>
            </div>


            <div class="form-row">
            <div class="form-group col-md-12">
                <label for="element-rotation">Name</label>
                <input value="${selectedElement.name}" type="text" class="form-control properties-input" property="name">
            </div>
        </div>
        <div class="form-row">
        <label>Tag</label>
        <div class="input-group mb-3">
  <div class="input-group-prepend">
    <button class="btn btn-outline-secondary dropdown-toggle" type="button" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Choose Tag</button>
    <div class="dropdown-menu">
    <a class="dropdown-item select-tag" href="#">MonsterHealthBar</a>
    <a class="dropdown-item select-tag" href="#">MonsterHealthText</a>
    <a class="dropdown-item select-tag" href="#">MonsterArmorText</a>
    <a class="dropdown-item select-tag" href="#">MonsterIcon</a>
    <a class="dropdown-item select-tag" href="#">MonsterNamec
    <div role="separator" class="dropdown-divider"></div>
<a class="dropdown-item select-tag" href="#">PlayerHealthBar</a>
<a class="dropdown-item select-tag" href="#">PlayerHealthText</a>
<a class="dropdown-item select-tag" href="#">PlayerAmorText</a>
<a class="dropdown-item select-tag" href="#">PlayerIcon</a>
<a class="dropdown-item select-tag" href="#">PlayerName</a>
<a class="dropdown-item select-tag" href="#">PlayerDiscardText</a>
<a class="dropdown-item select-tag" href="#">PlayerDeckText</a>
<a class="dropdown-item select-tag" href="#">PlayerCardBox</a>
 <a class="dropdown-item select-tag" href="#">PlayerFirstTag</a>
 <div role="separator" class="dropdown-divider"></div>
 <a class="dropdown-item select-tag" href="#">RivalHealthBar</a>
<a class="dropdown-item select-tag" href="#">RivalHealthText</a>
<a class="dropdown-item select-tag" href="#">RivalArmorText</a>
<a class="dropdown-item select-tag" href="#">RivalIcon</a>
<a class="dropdown-item select-tag" href="#">RivalName</a>
<a class="dropdown-item select-tag" href="#">RivalFirstTag</a>
<div role="separator" class="dropdown-divider"></div>
<a class="dropdown-item select-tag" href="#">TimerText</a>
<a class="dropdown-item select-tag" href="#">RivalDeckText</a>
<a class="dropdown-item select-tag" href="#">RivalDiscardText</a>
<a class="dropdown-item select-tag" href="#">RivalHandText (this is for the hand size of the rival)</a>

    </div>
  </div>
  <input type="text" class="form-control properties-input" property="tag" value="${selectedElement.tag}">
</div>
    </div>
    <div class="form-row">
    <div class="form-group col-md-12">
    <button type="button" class="btn btn-danger" id="delete-element">Delete</button>
    <button type="button" class="btn btn-warning" id="hide-element">Hide (Only In Editor)</button>
    </div>
</div>


            ${text}
            
            <div class="form-row">
                <div class="form-group">
                    <label for="element-position-x">Position</label>
                    <div class="input-group">
                        <div class="input-group-prepend">
                            <div class="input-group-text" id="center-x">x
                            </div>
                        </div>
                        <input value="${selectedElement.x}" type="number" class="form-control properties-input" property="x">
                    </div>
                    <div class="input-group">
                        <div class="input-group-prepend">
                            <div class="input-group-text" id="center-y">y
                            </div>
                        </div>
                        <input value="${selectedElement.y}" type="number" class="form-control properties-input" property="y">
                    </div>
                </div>
            </div>

            <div class="form-row">
                <div class="form-group">
                <label for="element-size-x">Size</label>
                    <div class="input-group">
                        <div class="input-group-prepend">
                            <div class="input-group-text">x
                            </div>
                        </div>
                        <input value="${selectedElement.width}" type="number" class="form-control properties-input" property="width">
                    </div>
                    <div class="input-group">
                        <div class="input-group-prepend">
                            <div class="input-group-text">y
                            </div>
                        </div>
                        <input value="${selectedElement.height}" type="number" class="form-control properties-input" property="height">
                    </div>
                </div>
            </div>

            <div class="form-row">
                <div class="form-group">
                    <label for="element-position-x">Z-Depth</label>
                    <div class="input-group">
                        <div class="input-group-prepend">
                            <div class="input-group-text">z
                            </div>
                        </div>
                        <input value="${selectedElement.depth}" type="number" step="0.01" min=0 max=1 class="form-control properties-input" property="depth">
                    </div>
                </div>
            </div>

            ${events}

        </div>
    `;

    propertiesList.insertAdjacentHTML("beforeend", markup);
}

module.exports.loadEvents = (target, type) => {
    var html = ``;

    if (target.type == "text") return html;

    target.events.forEach(element => {
        if (element.type == type) {
            html += `<li class="list-group-item d-flex justify-content-between">
                        <p>${element.id}</p>
                        <p>${element.action}</p>
                        <p>${element.data}</p>
                        <a style="padding: 5px;" elementId="${element.id}" class="edit-event-btn text-danger" href="#"><i class="fas fa-trash-alt"></i></a>
                    </li>`;
        }
    });

    if (html == ``) return html;

    var final = `<div class="card card-body">
                    <ul class="list-group list-group-flush">
                        ${html}
                    </ul>
                </div>`;
    return final;
}

module.exports.deleteEvent = (id) => {
    var temp = [];
    selectedElement.events.forEach(element => {
        if (element.id != id) {
            temp.push(element);
        }
    });
    selectedElement.events = temp;
}

module.exports.LoadUIFiles = () => {
    var dropdown = document.getElementsByClassName('ui-file-select');
    for (let item of dropdown) {
        item.innerHTML = "";
    }
    fs.readdir(p.join(Project.path, "client/data/ui"), { withFileTypes: true }, (err, files) => {
        files.forEach(element => {
            if (!element.isDirectory()) {
                for (let item of dropdown) {
                    item.insertAdjacentHTML("beforeend", '<option value="' + element.name + '">' + element.name + '</option>');
                }
            }
        });
    });
}

module.exports.LoadAudioFiles = () => {
    var dropdown = document.getElementsByClassName('audio-select');
    for (let item of dropdown) {
        item.innerHTML = "";
    }
    fs.readdir(p.join(Project.path, "client/data/audio"), { withFileTypes: true }, (err, files) => {
        files.forEach(element => {
            if (!element.isDirectory()) {
                for (let item of dropdown) {
                    item.insertAdjacentHTML("beforeend", '<option value="' + element.name + '">' + element.name + '</option>');
                }
            }
        });
    });
}

module.exports.LoadImages = () => {
    var dropdown = document.getElementsByClassName('image-select');
    for (let item of dropdown) {
        item.innerHTML = "";
    }
    fs.readdir(p.join(Project.path, "client/data/Textures"), { withFileTypes: true }, (err, files) => {
        files.forEach(element => {
            if (!element.isDirectory()) {
                for (let item of dropdown) {
                    item.insertAdjacentHTML("beforeend", '<option value="' + element.name + '">' + element.name + '</option>');
                }
            }
        });
    });
}

// creates a texture for an text box
function CreateText(textObject) {
    var bitmap = document.createElement('canvas');
    var g = bitmap.getContext('2d');
    bitmap.width = textObject.size.width;
    bitmap.height = textObject.size.height;
    //bitmap.
    g.font = textObject.typable_data.font_size + 'px Arial';
    g.fillStyle = 'rgba(' + textObject.typable_data.color.r * 255 + ',' + textObject.typable_data.color.g * 255 + ',' + textObject.typable_data.color.b * 255 + ',' + textObject.typable_data.color.a + ')';
    g.textAlign = 'center';
    g.textBaseline = 'middle';
    g.fillText(textObject.typable_data.text, textObject.size.width / 2, textObject.size.height / 2, textObject.size.width);
    // canvas contents will be used for a texture
    return g.getImageData(0, 0, textObject.size.width, textObject.size.height)
}

function CreateTextRegen(textObject) {
    var bitmap = document.createElement('canvas');
    var g = bitmap.getContext('2d');
    bitmap.width = textObject.width;
    bitmap.height = textObject.height;
    //bitmap.
    g.font = textObject.font_size + 'px Arial';
    g.fillStyle = textObject.color;
    g.textAlign = 'center';
    g.textBaseline = 'middle';
    g.fillText(textObject.text, textObject.width / 2, textObject.height / 2, textObject.width);
    return g.getImageData(0, 0, textObject.width, textObject.height)
}


module.exports.AddChild = (parent, children) => {
    children.forEach(ChildElement => {

        if (ChildElement.type == "UIText") {

            var asset = CreateText(ChildElement, parent)
            createImageBitmap(asset).then(function (value) {

                var text = UI.createSprite(value);
                text.type = "UIText";
                text.text = ChildElement.typable_data.text;
                text.name = ChildElement.name;
                text.anchor.x = ThreeUI.anchors.left;
                text.anchor.y = ThreeUI.anchors.bottom;
                text.pivot.x = 0;
                text.pivot.y = 1;
                text.textX = asset.textX;
                text.textY = asset.textY;
                text.width = ChildElement.size.width;
                text.height = ChildElement.size.height;
                text.parent = parent;
                text.color = 'rgba(' + ChildElement.typable_data.color.r * 255 + ',' + ChildElement.typable_data.color.g * 255 + ',' + ChildElement.typable_data.color.b * 255 + ',' + ChildElement.typable_data.color.a + ')';
                text.tag = (ChildElement.tag == undefined) ? null : ChildElement.tag;
                text.horizontal_alignment = ChildElement.typable_data.horizontal_alignment;
                text.vertical_alignment = ChildElement.typable_data.vertical_alignment;
                text.font_path = ChildElement.typable_data.font_path;
                text.font_size = ChildElement.typable_data.font_size;
                text.interactable = ChildElement.typable_data.interactable;
                text.isinputbox = ChildElement.typable_data.input == undefined ? false : true;
                text.input_data = ChildElement.typable_data.input;

                text.x = ChildElement.position.x;
                text.y = ChildElement.position.y;
                text.depth = ChildElement.position.depth ? ChildElement.position.depth : 0.00;
                //text.asset = ;

                text.onMousedown(() => {
                    if (!isUILock) {
                        selectedElement = text;
                        module.exports.LoadProperties(text);
                    }
                    text.isLocked = isUILock;
                });

                text.onMouseup(() => {
                });
                parent.children.push(text);
            })

        } else if (ChildElement.type == "UIImage") {
            var sprite = CreateSprite(ChildElement);
            sprite.parent = parent;
            parent.children.push(sprite);
        }

    });
}


module.exports.NewUI = () => {
    currentUIFile = {
        name: "",
        location: ""
    };
    UI.reset();
}

var isUILock = false;

module.exports.LockUI = function () {
    isUILock = (isUILock) ? false : true;
    UI.displayObjects.forEach(function (value, index) {
        if (value.type == "UIText") {
            value.isLocked = isUILock
        }
    })
    return isUILock;
}

module.exports.SaveUI = () => {
    //var canvasElements = this.canvas.toJSON(["id", "name", "children", "parent"]).objects;
    var json = CreateJsonFromUI()
    function save(filePath) {
        if (currentUIFile != undefined && filePath != "") {
            let name = p.basename(filePath).replace(".ui", "");
            json.name = name;
            fs.writeFileSync(filePath, JSON.stringify(json, null, 4), function (err) {
                if (err) {
                    console.log(err);
                }
            });
            currentUIFile = {
                name: p.basename(filePath),
                location: filePath
            }
            $("#preview-element-btn").attr("data-file", p.basename(filePath));
            Changes = [];
            THEHUB.NotifSuccess("Saved", "UI was saved")
        } else {
            THEHUB.NotifFail("Saved", "UI was not saved")
            console.log("NOT SAVED");
        }

    }
    if (currentUIFile == undefined || currentUIFile.location == "") {
        dialog.showSaveDialog({
            filters: [
                { name: 'UI Files', extensions: ["ui"] }
            ]
        }, save)
    } else {
        save(currentUIFile.location);
    }
    return;
}



module.exports.AddShape = (name, shape, value = undefined) => {
    if (shape == "image") {

        console.log("[UI EDIT] Adding Image");

        var sprite = UI.createSprite(p.join(Project.path, '/Client/data/Textures', value));
        sprite.name = name;
        sprite.alpha = 1;
        sprite.x = 1920 / 2;
        sprite.y = 1080 / 2;
        sprite.depth = 0;
        sprite.width = 500;
        sprite.height = 500;
        sprite.pivot.x = 0;
        sprite.pivot.y = 1;
        sprite.visible = true;
        sprite.del = false;
        sprite.anchor.x = ThreeUI.anchors.left;
        sprite.anchor.y = ThreeUI.anchors.bottom
        sprite.interactable = false;
        sprite.assetName = value;
        sprite.events = [];

        sprite.onMousedown(() => {
            selectedElement = sprite;
            module.exports.LoadProperties(sprite);
        });

        sprite.onMouseup(() => {
        });
        freeElements.push(sprite)
        CurrentScene.push(sprite);
        AddChanges(sprite);

    } else if (shape == "text") {

        var tmp = {
            name: name,
            size: {
                width: 500,
                height: 500
            }

        }
        tmp.type = "UIText";
        var color = 'rgba(0, 0, 0, 1)'
        var colors = color.split(/[(),]+/);
        tmp.typable_data = {
            text: value,
            horizontal_alignment: 1,
            vertical_alignment: 1,
            font_path: "Indie Flower",
            font_size: 32,
            interactable: false,
            color: {
                r: parseInt(colors[1]) / 255,
                g: parseInt(colors[2]) / 255,
                b: parseInt(colors[3]) / 255,
                a: parseInt(colors[4])
            }
        }

        var asset = CreateText(tmp)
        createImageBitmap(asset).then(function (value) {

            var text = UI.createSprite(value);
            text.type = "UIText";
            text.text = tmp.typable_data.text;
            text.name = tmp.name;
            text.tag = null;
            text.anchor.x = ThreeUI.anchors.left;
            text.anchor.y = ThreeUI.anchors.bottom;
            text.pivot.x = 0;
            text.pivot.y = 1;
            text.textX = asset.textX;
            text.textY = asset.textY;
            text.width = tmp.size.width;
            text.height = tmp.size.height;
            text.visible = true;
            text.del = false;
            text.color = 'rgba(' + tmp.typable_data.color.r * 255 + ',' + tmp.typable_data.color.g * 255 + ',' + tmp.typable_data.color.b * 255 + ',' + tmp.typable_data.color.a + ')';
            text.tag = (tmp.tag == undefined) ? null : tmp.tag;
            text.horizontal_alignment = tmp.typable_data.horizontal_alignment
            text.vertical_alignment = tmp.typable_data.vertical_alignment
            text.font_path = tmp.typable_data.font_path
            text.font_size = tmp.typable_data.font_size
            text.interactable = tmp.typable_data.interactable
            text.isinputbox = false;
            text.x = 1920 / 2;
            text.y = 1080 / 2;
            text.depth = 0;
            text.onMousedown(() => {
                if (!isUILock) {
                    selectedElement = text;
                    module.exports.LoadProperties(text);
                }
                text.isLocked = isUILock;
            });

            text.onMouseup(() => {
            });
            freeElements.push(text)
            CurrentScene.push(text);
            AddChanges(text);
        })


    }
}

module.exports.AddEvent = (type, action, data = undefined) => {
    var event = {
        id: selectedElement.events.length > 0 ? selectedElement.events[selectedElement.events.length - 1].id + 1 : 0,
        type: type,
        action: action,
        data: data
    };
    selectedElement.events.push(event);
    module.exports.LoadProperties(selectedElement);
}
module.exports.HideElement = function () {
    selectedElement.visible = (selectedElement.visible) ? false : true;
}

module.exports.DeleteElement = function () {
    selectedElement.visible = false;
    selectedElement.del = true;
    console.log(selectedElement.children)
    selectedElement = undefined;
}

module.exports.CenterElement = (axis) => {
    if (axis == "x") {
        if (selectedElement.parent) selectedElement.x = 0;
        else selectedElement.x = 1920 / 2 - selectedElement.width / 2;
    } else if (axis == "y") {
        if (selectedElement.parent) selectedElement.y = 0;
        else selectedElement.y = 1080 / 2 - selectedElement.height / 2;
    }
    module.exports.LoadProperties(selectedElement);
    AddChanges(selectedElement);
}

module.exports.UpdateProperty = (property, value) => {
    if (property == "x") {
        selectedElement.x = Math.round(parseInt(value) * 10) / 10;
    } else if (property == "y") {
        selectedElement.y = Math.round(parseInt(value) * 10) / 10;
    } else if (property == "depth") {
        if (value > 0.99) selectedElement.depth = 0.99;
        else selectedElement.depth = Math.round(parseFloat(value) * 100) / 100;
    } else if (property == "width") {
        selectedElement.width = Math.round(parseInt(value) * 10) / 10;
    } else if (property == "height") {
        selectedElement.height = Math.round(parseInt(value) * 10) / 10;
    } else if (property == "name") {
        selectedElement.name = value;
    } else if (property == "color") {
        selectedElement.color = value;
        createImageBitmap(CreateTextRegen(selectedElement)).then(function (v) {
            selectedElement.asset = v;
            console.log(value)
        });
    } else if (property == "text") {

        selectedElement.text = value;
        createImageBitmap(CreateTextRegen(selectedElement)).then(function (v) {
            selectedElement.asset = v;
            console.log(value)
        });

    } else if (property == "font") {

        selectedElement.font_path = value;
        createImageBitmap(CreateTextRegen(selectedElement)).then(function (v) {
            selectedElement.asset = v;
            console.log(value)
        });

    } else if (property == "tag") {
        console.log("TAG: " + value)
        selectedElement.tag = value;
    } else if (property == "textsize") {
        selectedElement.font_size = Math.round(parseInt(value) * 10) / 10;
        createImageBitmap(CreateTextRegen(selectedElement)).then(function (v) {
            selectedElement.asset = v;
        });
    } else if (property == "interactable") {
        selectedElement.interactable = value;
    } else if (property == "isinputbox") {
        selectedElement.isinputbox = value;
    } else if (property == "assetname") {
        selectedElement.setAssetPath(p.join(Project.path, '/Client/data/Textures', value))
        selectedElement.assetName = value;
    } else if (property == "input-validation") {
        selectedElement.input_data.validation = value;
    } else if (property == "input-obscure") {
        selectedElement.input_data.obscure_text = value;
    } else if (property == "input-tag") {
        selectedElement.input_data.interactable_tag = value;
    }
    console.log(property + ": " + value);
    UI.shouldReDraw = true;
    CreateList();
    AddChanges(selectedElement)
}