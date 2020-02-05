const fs = require('fs');
const p = require('path');
const THEHUB = require('../js/thehub.js');
const child_process = require('child_process');
var G = undefined;
var SHEETS = undefined;
var sheets = undefined;

function Init(g, gsheets, sheetss) {
    G = g;
    SHEETS = gsheets;
    sheets = sheetss;
}

function SimulateGames(argumentList, callback, finished) {
        var serverPath = p.join(THEHUB.GetCurrentProject().path, "Server");
        console.log(argumentList.join(" "))
        //child_process.execSync('start /D "' + serverPath + '" Server.exe --simulate-game 1  2  ExampleMonsterDeck  TestDeck1  TestDeck2', []);
        var sp = child_process.spawn(p.join(serverPath, 'GameServer.exe'), argumentList, { cwd: serverPath }, (error, stdout, stderr) => {
            if (error) {
            }
            console.log(stderr)
            console.log(stdout)

        });
        sp.on("close", (code) => {
            finished();
        })
        sp.stdout.on('data', (data) => {
          if(data.toString().includes("SIMULATE=DONE")){
              callback();
          }else{
            document.getElementById("textOutputArea").value += "[SIMULATE GAME]:\n\n";
              document.getElementById("textOutputArea").value += data.toString()
          }

        });
}

function LoadSimulationData() {
    let files = p.join(THEHUB.GetCurrentProject().path, "Server/data/BalancingData");
   return fs.readdirSync(files, { withFileTypes: true });
}

function AddSheets(id, count, callback, total = undefined) {
    var t = total ? total : count;
    if (count > 0) {
        SHEETS.CopySheetToSpreadsheet('1vbe_vUf-hRqKfSJQrajHgEfno0PyVnN8y8cgfvLmN9Y', '1795879528', id, (response) => {
            //console.log(JSON.stringify(response, null, 2));

            var request = {
                spreadsheetId: id,
                resource: {
                    requests: [
                        {
                            "updateSheetProperties": {
                                "properties": {
                                    "sheetId": response.data.sheetId,
                                    "title": "Sample Game " + (t-count+2)
                                }, 
                                "fields": "title"
                            }
                        }
                    ]
                }
            };

            sheets.spreadsheets.batchUpdate(request, function (err, response) {
                if (err) {
                    console.error(err);
                    return;
                }

                //console.log(JSON.stringify(response, null, 2));
                module.exports.AddSheets(id, count - 1, callback, t);
            });
        });
    } else {
        callback();
    }
}

module.exports.Init = Init;
module.exports.AddSheets = AddSheets;
module.exports.SimulateGames = SimulateGames;
module.exports.LoadSimulationData = LoadSimulationData;
