const { parse } = require('url');
const { remote } = require('electron');
const qs = require('qs');
const axios = require('axios');
const Store = require('electron-store');
const { google } = require('googleapis');
const config = require('../js/config.js');



const GOOGLE_AUTHORIZATION_URL = 'https://accounts.google.com/o/oauth2/v2/auth'
const GOOGLE_TOKEN_URL = 'https://www.googleapis.com/oauth2/v4/token'
const GOOGLE_PROFILE_URL = 'https://www.googleapis.com/userinfo/v2/me'
const GOOGLE_CLIENT_ID = config.config.google.id;
const GOOGLE_CLIENT_SECRET = config.config.google.secret;
const GOOGLE_REDIRECT_URI = config.config.google.redirect;
const GOOGLE_SCOPES =  'profile email  https://www.googleapis.com/auth/drive https://www.googleapis.com/auth/drive.file https://www.googleapis.com/auth/spreadsheets';

const oauth2Client = new google.auth.OAuth2(
  GOOGLE_CLIENT_ID,
  GOOGLE_CLIENT_SECRET,
  GOOGLE_REDIRECT_URI
);

module.exports.oauth2Client = oauth2Client;
module.exports.google = google;


async function googleSignIn(callback) {
  const store = new Store();
  var googleToken = store.get("googleToken");
  var googleUser = store.get("googleUser");

  var ok = true;

  if (googleToken == undefined) {
    const code = await signInWithPopup()
    googleToken = await fetchAccessTokens(code)
    ok = false;
  }
  if (googleToken.expires_at < (new Date().getTime() / 1000)) {
    const code = await signInWithPopup();
    googleToken = await fetchAccessTokens(code);
    ok = false;
  }
  if(!ok){
  const { id, email, name } = await fetchGoogleProfile(googleToken.access_token)
  const providerUser = {
    uid: id,
    email,
    displayName: name,
    idToken: googleToken.id_token,
  }
  var correction = Math.ceil(new Date().getTime() / 1000);
  googleToken.expires_at = correction + googleToken.expires_in;
  store.set('googleUser', providerUser);
  store.set('googleToken', googleToken);
  return callback(providerUser)
  }else{
    return callback(googleUser);
  }

}




function signInWithPopup() {
  return new Promise((resolve, reject) => {
    const authWindow = new remote.BrowserWindow({
      width: 500,
      height: 600,
      show: true,
    })
 
    const urlParams = {
      response_type: 'code',
      redirect_uri: GOOGLE_REDIRECT_URI,
      client_id: GOOGLE_CLIENT_ID,
      scope: GOOGLE_SCOPES,
    }
    const authUrl = `${GOOGLE_AUTHORIZATION_URL}?${qs.stringify(urlParams)}`

    function handleNavigation(url) {
      const query = parse(url, true).query
      if (query) {
        if (query.error) {
          reject(new Error(`There was an error: ${query.error}`))
        } else if (query.code) {
          // Login is complete
          authWindow.removeAllListeners('closed')
          setImmediate(() => authWindow.close())

          // This is the authorization code we need to request tokens
          resolve(query.code)
        }
      }

    }

    authWindow.on('closed', () => {
      // TODO: Handle this smoothly

    })

    authWindow.webContents.on('will-navigate', (event, url) => {
      handleNavigation(url)
    })

    authWindow.webContents.on('will-redirect', (event, url) => {
      handleNavigation(url)
    })

    authWindow.webContents.on('did-get-redirect-request', (event, oldUrl, newUrl) => {

      handleNavigation(newUrl)
    })

    authWindow.loadURL(authUrl)
  })
}

async function fetchAccessTokens(code) {
  const response = await axios.post(GOOGLE_TOKEN_URL, qs.stringify({
    code,
    client_id: GOOGLE_CLIENT_ID,
    client_secret: GOOGLE_CLIENT_SECRET,
    redirect_uri: GOOGLE_REDIRECT_URI,
    grant_type: 'authorization_code',
  }), {
      headers: {
        'Content-Type': 'application/x-www-form-urlencoded',
      },
    })

  return response.data
}

async function fetchGoogleProfile(accessToken) {
  try{
  const response = await axios.get(GOOGLE_PROFILE_URL, {
    headers: {
      'Content-Type': 'application/json',
      'Authorization': `Bearer ${accessToken}`,
    },
  });
  return response.data
  }catch(err){
    if(err.response.status == 401)
    {
      const store = new Store();
      if(store.get("googleToken") !== undefined){
        store.delete("googleToken");
        location.reload();
      }
      return {id:null,email:null,name:null};
    }
    return {id:null,email:null,name:null};
  }
}

async function GetSpreetSheet(id, callback,error) {
  var params = {
    spreadsheetId: id,
    includeGridData: true
  }
  var googleToken = new Store().get("googleToken");
  oauth2Client.setCredentials(googleToken);
  const sheets = google.sheets({
    version: 'v4',
    auth: oauth2Client
  });
  sheets.spreadsheets.get(params, (err, res) => {
    if (err) {
      console.error(err)
      error(err);
      if(err.response.status != 401){
        require("../js/thehub.js").NotifFail("Failed", "Could not load the file!");
      }else{
        require("../js/thehub.js").NotifFail("Google", "Session Expiered!");
      }
    } else {
      callback(res);
    }
  });
}

function GetTeamDrives(callback){
  var params = {
  }
  var googleToken = new Store().get("googleToken");
  oauth2Client.setCredentials(googleToken);
  const drive = google.drive({
    version: 'v2',
    auth: oauth2Client
  });
  drive.teamdrives.list(params, (err, res) => {
    if (err) {
      if(err.response.status != 401){
        require("../js/thehub.js").NotifFail("Failed", "...");
      }else{
        require("../js/thehub.js").NotifFail("Google", "Session Expiered!");
      }
      console.error(err);
    } else {
      callback(res);
    }
  });
}
/// help see https://developers.google.com/drive/api/v3/reference/files/list
function GetFiles(teamDriveId,callback){
  var params = {
    teamDriveId:teamDriveId,
    supportsTeamDrives:true,
    includeTeamDriveItems:true,
    driveId:teamDriveId,
    q:'mimeType = "application/vnd.google-apps.spreadsheet"',
    fields:'files',
    corpora:'drive'
  }
  var googleToken = new Store().get("googleToken");
  oauth2Client.setCredentials(googleToken);
  const drive = google.drive({
    version: 'v3',
    auth: oauth2Client
  });
  drive.files.list(params, (err, res) => {
    if (err) {
      if(err.response.status != 401){
        require("../js/thehub.js").NotifFail("Failed", "...");
      }else{
        require("../js/thehub.js").NotifFail("Google", "Session Expiered!");
      }
      console.error(err);
    } else {
     callback(res.data.files);
    }
  });
}

function CreateFile(params, options, callback) {
    var googleToken = new Store().get("googleToken");
    oauth2Client.setCredentials(googleToken);
    const drive = google.drive({
        version: 'v3',
        auth: oauth2Client
    });
    drive.files.create(params, options, (err, file) => {
        console.log(file);
        callback(err, file);
    });
}

function CopyFile(params, options, callback) {
    var googleToken = new Store().get("googleToken");
    oauth2Client.setCredentials(googleToken);
    const drive = google.drive({
        version: 'v3',
        auth: oauth2Client
    });
    drive.files.copy(params, options, (err, file) => {
        console.log(file);
        callback(err, file);
    });
}

function CheckIfTokenIsExpired(callback){
  setInterval(function(){
    const store = new Store();
    var googleToken = store.get("googleToken");
    if (googleToken.expires_at < (new Date().getTime() / 1000)) {
      callback();
    }
  }, 60000);
}

module.exports.CheckIfTokenIsExpired = CheckIfTokenIsExpired
module.exports.GetFilesFromTeamDrive = GetFiles;
module.exports.GetTeamDrives =GetTeamDrives;
module.exports.googleSignIn = googleSignIn;
module.exports.GetSpreetSheet = GetSpreetSheet;
module.exports.CreateFile = CreateFile;
module.exports.CopyFile = CopyFile;