const { remote } = require('electron');
const Store = require('electron-store');
const g = require('../js/google-utils.js');
const THEHUB = require('../js/thehub.js');
const config = require('../js/config.js');
const projectConfig = require('../js/project_config.js');

var URL = "https://docs.google.com/spreadsheets/d/"
const store = new Store();
function Open(onclosed) {
    const Editor = new remote.BrowserWindow({

        width: 1024,
        height: 764,
        show: true,
        frame: false,
    })
    Editor.webContents.openDevTools();
    Editor.on('closed', () => {
        THEHUB.NotifInfo("Stopped Editing", "Closed the card Editor");
        onclosed()
    })
    Editor.loadFile("src/content/sheet_editor.html")
}

function LoadGoogleDocument() {
    projectConfig.LoadProjectConfig(THEHUB.GetCurrentProject().path);
    var webview = document.querySelector("webview");
    console.log(projectConfig.ProjectConfig)
    webview.src = URL + projectConfig.ProjectConfig.file + "/";;
    webview.addEventListener('dom-ready', () => {
        webview.classList.remove("d-none");
        var loader = document.getElementById("loader");
        var dbtn = document.getElementById("download-btn");
        dbtn.classList.remove("d-none");
        if (loader)
            loader.parentNode.removeChild(loader);
        webview.openDevTools()
    });
    webview.addEventListener('will-navigate', (event) => {
        //webview.src = URL;
        //TODO: DO something
    })
}

function parseValue(value) {
    if (value.hasOwnProperty("effectiveValue")) {
        var val = value.effectiveValue;
        if (val.hasOwnProperty("stringValue"))
            return val.stringValue;
        if (val.hasOwnProperty("numberValue"))
            return val.numberValue;
        if (val.hasOwnProperty("formattedValue"))
            return val.formattedValue;
    }
    return null;
}
function makeKey(str) {
    return str.replace(" ", "");
}
function ParseRewards(data) {
    var objectNames = data[0].values;
    var output = [];
    data.splice(0, 1)
    data.forEach((val, key) => {
        var object = {};
        if (val.values == undefined) return;
        val.values.forEach((v, k) => {
            if (k <= 4) {
                var value = parseValue(v);
                if (value != null)
                    object[makeKey(parseValue(objectNames[k]))] = value;
            }
        });
        output.push(object)
    });
    return output;

}

function ParseCards(data, OutputJson) {
    //card types
    OutputJson.CardTypes = [];
    data[0].values.splice(0, 2);
    data[0].values.forEach((val, key) => {
        OutputJson.CardTypes.push(parseValue(val));
    });
    //meta
    var metaKeys = [];
    var effects = [];
    var ids = [];
    var isMeta = true;
    var isEffect = true;
    var isIds = true;
    var effectIndex = 0;
    var idIndex = 0;
    data[2].values.splice(2, 1);
    data[2].values.forEach((v, k) => {
        if (parseValue(v) == null) {
            return;
        }

        if (parseValue(v).replace(" ", "") == "Id") {
            isEffect = false;
            isMeta = true;
            isIds = true;
            idIndex = k;
        }
        if (parseValue(v).replace(" ", "") == "Effects") {
            isEffect = true;
            isMeta = false;
            isIds = false;
            effectIndex = k;
        }

        if (isMeta)
            metaKeys.push(parseValue(v).replace(" ", ""));
        else if (isEffect)
            effects.push(parseValue(v).replace(" ", ""));
        else if (isIds)
            ids.push(parseValue(v).replace(" ", ""));

    });
    var cards = [];
    data.splice(0, 3);
    data.forEach((card, k) => {
        var cardObject = {};
        cardObject.Meta = {};
        cardObject.Effects = {}
        cardObject.Pos = []
        cardObject.id = 0
        cardObject.Deleted = false;
        card.values.splice(2, 1);
        card.values.forEach((propertry, idx) => {
            var value = parseValue(propertry);
            if (0 == idx && value == "X" || value == "x") {
                cardObject.Deleted = true;
            }
            if (idIndex == idx) {
                cardObject.id = parseInt(value)
                cardObject.Pos = [idx, k + 4]
            }
            if (effectIndex > idx) {
                cardObject.Meta[metaKeys[idx]] = value;
            } else {
                var newIndex = idx - effectIndex;
                cardObject.Effects[effects[newIndex]] = value;
            }
        });
        if (card.values.length > 2 && cardObject.Meta.Name != null) {
            cards.push(cardObject);
        }
    });
    return cards;
}

function ParseMonster(data, OutputJson) {
    //card types
    var isData = false
    var cardDataIndex = 0;
    var metaKeys = []
    var dataKeys = []
    data[1].values.splice(1, 1);
    data[1].values.forEach((v, k) => {
        if (parseValue(v) == null) {
            return;
        }
        if (parseValue(v).replace(" ", "") == "Id") {
            return;
        }
        if (parseValue(v).replace(" ", "") == "CardData") {
            isData = true;
            cardDataIndex = k;
        } else {
            if (!isData)
                metaKeys.push(parseValue(v).replace(" ", ""));
            else
                dataKeys.push(parseValue(v).replace(" ", ""));
        }
    });

    var cards = [];
    data.splice(0, 2);
    data.forEach((card, k) => {
        var cardObject = {};
        cardObject.Meta = {};
        cardObject.Data = {}
        cardObject.Pos = []
        cardObject.id = 0
        cardObject.Deleted = false;

        card.values.splice(2, 1);
        card.values.forEach((propertry, idx) => {
            var value = parseValue(propertry);
            if (idx == 1 && value == null) {
                cardObject.Pos = [idx, k + 3]
                return;
            }
            if (1 == idx) {
                cardObject.id = parseInt(value)
                cardObject.Pos = [idx, k + 3]
            }
            if (0 == idx && value == "X" || value == "x") {
                cardObject.Deleted = true;
            }
            if (idx != cardDataIndex) {
                if (idx > cardDataIndex) {
                    cardObject.Data[dataKeys[idx - cardDataIndex - 1]] = value
                } else {
                    if (metaKeys[idx - 1] != undefined)
                        cardObject.Meta[metaKeys[idx - 1]] = value
                }

            }
        });
        if (card.values.length > 2 && cardObject.Meta.Name != null) {

            cards.push(cardObject);
        }
    });
    return cards;
}


function ParseDecks(data, OutputJson) {
    var output = [];
    data.splice(0, 1);
    data.forEach((deck, k) => {
        var Deck = {
            name: "",
            Cards: []
        }
        Deck.name = parseValue(deck.values[0]);
        deck.values.splice(0, 1);
        deck.values.forEach((propertry, idx) => {
            Deck.Cards.push(parseValue(propertry));
        })
        output.push(Deck);
    })
    return output;
}

function ParseRarity(data, OutputJson) {
    var keys = [];
    data[0].values.forEach((v, k) => {
        if (parseValue(v) == null) {
            return;
        } else {
            keys.push(parseValue(v))
        }
    });
    var output = [];
    data.splice(0, 1)
    data.forEach((val, key) => {
        var object = {};
        val.values.forEach((v, k) => {
            if (k < 3) {
                var value = parseValue(v);
                if (value != null)
                    object[keys[k]] = value;
            }
        });
        output.push(object)
    });
    return output;
}

var Parser = {
    Rewards: ParseRewards,
    Cards: ParseCards,
    Decks: ParseDecks,
    Rarities: ParseRarity,
    Monster: ParseMonster,
    UX: () => { },
};

function Download(callback, err) {
    var OutputJson = {};
    g.GetSpreetSheet(projectConfig.ProjectConfig.file, (res) => {
        res.data.sheets.forEach((val, key) => {
            OutputJson[val.properties.title] = Parser[val.properties.title](val.data[0].rowData, OutputJson);
        })
        callback(OutputJson);
    }, err);
}

function Save(data) {
    var projectName = THEHUB.GetCurrentProject().realname;
    var assetDir = config.MakeAssetDir(projectName);
    THEHUB.WriteObjectToFile(assetDir.server.assets + "/cards.json", data);
    THEHUB.NotifSuccess("Downloaded", "Saved");
    var dbtn = document.getElementById("download-btn");
    dbtn.classList.remove("d-none");
    var downloader = document.getElementById("downloader");
    downloader.classList.add("d-none");
}

function UpdateSheetCell(id, range, values, dimension) {
    if (dimension == undefined) dimension = "ROWS"
    var params = {
        spreadsheetId: id,
        range: range,
        valueInputOption: "RAW",
        requestBody: {
            majorDimension: dimension,
            values: values
        }
    }
    var googleToken = new Store().get("googleToken");
    g.oauth2Client.setCredentials(googleToken);
    const sheets = g.google.sheets({
        version: 'v4',
        auth: g.oauth2Client
    });

    sheets.spreadsheets.values.update(params, (res, error) => {
        console.log(res)
        console.log(error)
    })
}

function BatchUpdate(id, update) {
    var params = {
        spreadsheetId: id,
        valueInputOption: "USER_ENTERED",
        requestBody: {
            data:update
        }
    }
    var googleToken = new Store().get("googleToken");
    g.oauth2Client.setCredentials(googleToken);
    const sheets = g.google.sheets({
        version: 'v4',
        auth: g.oauth2Client
    });

    sheets.spreadsheets.values.batchUpdate(params, (res, error) => {
        console.log(res)
        console.log(error)
    })
}

function CopySheetToSpreadsheet(spreadsheetId, sheetId, destinationSpreadsheetId, callback) {
    var googleToken = new Store().get("googleToken");
    g.oauth2Client.setCredentials(googleToken);
    const sheets = g.google.sheets({
        version: 'v4',
        auth: g.oauth2Client
    });
    var request = {
        spreadsheetId: spreadsheetId,
        sheetId: sheetId,
        resource: {
            destinationSpreadsheetId: destinationSpreadsheetId
        }
    };
    sheets.spreadsheets.sheets.copyTo(request, function (err, response) {
        if (err) {
            console.error(err);
            return;
        }
        callback(response);
    });
}

function ClearCells(id, range) {
    var params = {
        spreadsheetId: id,
        range: range,
        requestBody: {
        }
    }
    var googleToken = new Store().get("googleToken");
    g.oauth2Client.setCredentials(googleToken);
    const sheets = g.google.sheets({
        version: 'v4',
        auth: g.oauth2Client
    });

    sheets.spreadsheets.values.clear(params, (res, error) => {
        console.log(res)
        console.log(error)
    })
}
module.exports.BatchUpdate = BatchUpdate;
module.exports.ClearCells = ClearCells
module.exports.Open = Open;
module.exports.LoadGoogleDocument = LoadGoogleDocument;
module.exports.Download = Download;
module.exports.UpdateSheetCell = UpdateSheetCell;
module.exports.CopySheetToSpreadsheet = CopySheetToSpreadsheet;