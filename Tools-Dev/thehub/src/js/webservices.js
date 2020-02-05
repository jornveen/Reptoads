const child_process = require('child_process');
const nodePath = require('path');
////////////// INTERNAL ///////////////////////////////////////
const config = require('./config.js').config;
var mysql = require('mysql');


function mysql_real_escape_string (str) {
  if (typeof str != 'string')
      return str;

  return str.replace(/[\0\x08\x09\x1a\n\r"'\\\%]/g, function (char) {
      switch (char) {
          case "\0":
              return "\\0";
          case "\x08":
              return "\\b";
          case "\x09":
              return "\\t";
          case "\x1a":
              return "\\z";
          case "\n":
              return "\\n";
          case "\r":
              return "\\r";
          case "\"":
          case "'":
          case "\\":
          case "%":
              return "\\"+char; // prepends a backslash to backslash, percent,
                                // and double/single quotes
      }
  });
}

class Webservice {
  constructor(con) {
    this.con = con;
    this.mysql_real_escape_string = mysql_real_escape_string
  }
  Get(sql, callback, error, eventId) {
    console.log(sql)
    return this.con.query(sql, function (err, result) {
      if (err) {
        console.error(err)
        error(err);
        return;
      } else {
        result.forEach(callback);
        if (result.length == 0) callback([]);
        if (eventId == undefined)
          eventId = "get-event"
        
          console.log(result)
        document.dispatchEvent(new CustomEvent(eventId, {
          detail: result}));
      }
    });
  }
  OnEvent(eventId, callback) {
    document.addEventListener(eventId, callback);
  }
  GetScripts(filter, callback, error,eventId) {
    let query = ""
    if (filter.getMonsters != undefined) {
      eventId = (eventId != undefined)? eventId: "get-monster-scripts"
      query = "SELECT scripts.*, monsters.name as scriptName FROM `scripts` " +
        "INNER JOIN scriptMonsterRelation ON scriptMonsterRelation.scriptId = scripts.id " +
        "INNER JOIN monsters ON scriptMonsterRelation.monsterId = monsters.id " +
        "WHERE monsters.projectId = " + filter.projectId + " AND (scripts.id,scripts.revision) IN(SELECT scripts.id, MAX(scripts.revision) revision FROM scripts GROUP BY scripts.id)"
      } else if (filter.getPlayerCards != undefined) {
      eventId =  (eventId != undefined)? eventId:  "get-player-scripts"
      query = "SELECT scripts.*, cards.name as scriptName FROM `scripts` " +
        "INNER JOIN scriptCardRelation ON scriptCardRelation.scriptId = scripts.id " +
        "INNER JOIN cards ON scriptCardRelation.cardId = cards.id " +
        "WHERE cards.projectId = " + filter.projectId + " AND (scripts.id,scripts.revision) IN(SELECT scripts.id, MAX(scripts.revision) revision FROM scripts GROUP BY scripts.id)"
    } else if (filter.getOther != undefined) {
      query =
        "SELECT scripts.*,scriptOtherRelation.name as scriptName FROM scripts " +
        "INNER JOIN scriptOtherRelation ON scripts.id = scriptOtherRelation.scriptId " +
        "WHERE scriptOtherRelation.projectId =" + filter.projectId + " AND (scripts.id,scripts.revision) IN(SELECT scripts.id, MAX(scripts.revision) revision FROM scripts GROUP BY scripts.id)"
      eventId =  (eventId != undefined)? eventId:  "get-other-scripts"
    }
    this.Get(query, callback, error,
      eventId);
  }
  GetProjectConfig(filter, callback, err) {
    let eventId = (filter.configType) ? "get-config-client" : "get-config-server";
    this.Get("SELECT * FROM `projectConfig` WHERE projectId = " + filter.projectId + " AND type = " + filter.configType, callback, err, eventId)
  }
  GetProjects(callback, error, fliter) {
    if (fliter == undefined)
      this.Get("SELECT * FROM `projects`", callback, error, "get-projects")
    else if (fliter.id != undefined) {
      this.Get("SELECT * FROM `projects` WHERE id =" + fliter.id, callback, error, "get-project")
    }

  }
  GetCards(filter, callback, error,eventId) {
    let sql = "";

    if (filter.projectId == undefined) {
      error({ err: 'No projectId' });
      return;
    } else {
      sql = ' SELECT ' +
        'cards.*, ' +
        'cardTypes.name AS cardType, ' +
        'cardRarity.name AS cardRarity ' +
        ' FROM ' +
        'cards ' +
        ' INNER JOIN cardTypes ON cards.cardTypeId = cardTypes.id ' +
        ' INNER JOIN cardRarity ON cards.cardRarityId = cardRarity.id ' +
        ' WHERE ' +
        ' cards.projectId = ' + filter.projectId;
    }

    if (filter.byDeckId != undefined) {
      sql = ' SELECT ' +
        'cards.*, ' +
        'cardTypes.name AS cardType, ' +
        'cardRarity.name AS cardRarity, ' +
        ' deckCardRelation.deckId ' +
        ' FROM ' +
        'cards ' +
        ' INNER JOIN deckCardRelation ON deckCardRelation.cardId = cards.id ' +
        ' INNER JOIN cardTypes ON cards.cardTypeId = cardTypes.id ' +
        ' INNER JOIN cardRarity ON cards.cardRarityId = cardRarity.id ' +
        ' WHERE ' +
        ' cards.projectId = ' + filter.projectId + ' AND deckCardRelation.deckId = ' + filter.byDeckId
    }
    if (filter.profileId != undefined && filter.byDeckId == undefined) {
      sql = ' SELECT ' +
        'cards.*, ' +
        'cardTypes.name AS cardType, ' +
        'cardRarity.name AS cardRarity ' +
        ' FROM ' +
        'cards ' +
        ' INNER JOIN profileCardRelation ON profileCardRelation.cardId = cards.id ' +
        ' INNER JOIN cardTypes ON cards.cardTypeId = cardTypes.id ' +
        ' INNER JOIN cardRarity ON cards.cardRarityId = cardRarity.id ' +
        ' WHERE ' +
        ' cards.projectId = ' + filter.projectId + ' AND profileCardRelation.profileId = ' + filter.profileId
    } else if (filter.profileId != undefined && filter.byDeckId != undefined) {
      sql = ' SELECT ' +
        'cards.*, ' +
        'cardTypes.name AS cardType, ' +
        'cardRarity.name AS cardRarity, ' +
        ' deckCardRelation.deckId ' +
        ' FROM ' +
        'cards ' +
        ' INNER JOIN profileCardRelation ON profileCardRelation.cardId = cards.id ' +
        ' INNER JOIN deckCardRelation ON deckCardRelation.cardId = cards.id ' +
        ' INNER JOIN cardTypes ON cards.cardTypeId = cardTypes.id ' +
        ' INNER JOIN cardRarity ON cards.cardRarityId = cardRarity.id ' +
        ' WHERE ' +
        ' cards.projectId = ' + filter.projectId + ' AND deckCardRelation.deckId = ' + filter.byDeckId + ' AND profileCardRelation.profileId = ' + filter.profileId;
    }

    if (filter.cardId != undefined) {
      sql += " AND cards.id = " + filter.cardId
    }
    eventId =  (eventId != undefined)? eventId:  "get-cards"
    this.Get(sql, callback, error, eventId)
  }
  GetMonsters(filter, callback, error,eventId) {
    let sql = "";
    if (filter.projectId == undefined) {
      error({ err: 'No projectId' });
      return;
    } else {
      sql = ' SELECT ' +
        'monsters.*, ' +
        'cardTypes.name AS cardType' +
        ' FROM ' +
        'monsters ' +
        ' INNER JOIN cardTypes ON monsters.cardTypeId = cardTypes.id '+
      ' WHERE ' +
        ' monsters.projectId = ' + filter.projectId;
    }
    if (filter.byDeckId != undefined) {
      sql = ' SELECT ' +
        'monsters.*, ' +
        'cardTypes.name AS cardType' +
        ' FROM ' +
        'monsters ' +
        ' INNER JOIN monsterDeckRelation ON monsterDeckRelation.monsterId = monsters.id ' +
        ' INNER JOIN cardTypes ON monsters.cardTypeId = cardTypes.id '+
      ' WHERE ' +
        ' monsters.projectId = ' + filter.projectId + ' AND monsterDeckRelation.deckId =' + filter.byDeckId;
    }
    
    if (filter.monsterId != undefined) {
      sql += " AND monsters.id = " + filter.monsterId
    }
    eventId =  (eventId != undefined)? eventId:  "get-monsters"
    this.Get(sql, callback, error,eventId );
  }
  GetMonsterRewards(filter, callback, error) {
    let sql = "SELECT monsterRewards.*,monsterRewardsType.enumId,monsterRewardsType.description FROM `monsterRewards` " +
      "INNER JOIN monsterRewardsType ON monsterRewardsType.id = monsterRewards.rewardTypeId " +
      "INNER JOIN monsters ON monsterRewards.cardId = monsters.id " +
      "WHERE monsters.projectId = " + filter.projectId
    if (filter.id != undefined)
      sql += " AND monsters.id =" + filter.id

    this.Get(sql, callback, error, "get-monster-rewards");
  }
  GetMonstersRewardTypes(filter, callback, error) {
    let sql = "SELECT * FROM monsterRewardsType";
    if (filter.id != undefined) {
      sql += " WHERE id = " + filter.id
    }
    this.Get(sql, callback, error, "get-monster-rewards-types");
  }
  GetMonsterDeck(filter, callback, error) {
    this.GetDecks({
      projectId: filter.projectId,
      isMonster: true
    },
      callback,
      error);
  }
  GetDecks(filter, callback, error) {
    let where = []
    let eventId = "get-decks"
    if (filter.profileId != undefined)
      where.push("profileId = " + filter.profileId)
    if (filter.projectId != undefined)
      where.push('projectId = ' + filter.projectId)
    if (where.length > 0)
      where[0] = "WHERE " + where[0];

    let sql = "SELECT * FROM decks " + where.join(" AND ")

    if (filter.isMonster != undefined) {
      sql = "SELECT * FROM monsterDecks WHERE projectId =" + filter.projectId
      eventId = "get-monster-deck"
    }

    this.Get(sql, callback, error, eventId)
  }

  GetProfiles(id, callback, error) {
    let where = [];
    let eventId = "get-profiles";
    if (id != undefined)
      where.push("id = " + id);
    if (where.length > 0)
      where[0] = "WHERE " + where[0];

    let sql = "SELECT * FROM profiles " + where.join(" AND ");

    this.Get(sql, callback, error, eventId);
  }

  InsertProfile(username, password, email, callback, error) {
    var query = {
      table: "profiles",
      data: {
        username: "'"+mysql_real_escape_string(username)+"'",
        password: "'"+mysql_real_escape_string(password)+"'",
        email: "'"+mysql_real_escape_string(email)+"'"
      }
    };
    var eventId = "insert-user";
    this.Insert(query, callback, error, eventId);
  }

  Insert(query, callback, error, eventId) {

    let sql = "INSERT INTO " + query.table + "(" + Object.keys(query.data).join(",") + ") VALUES(" + Object.values(query.data).join(",") + ")";
    console.log(sql)
    this.con.query(sql, function (err, result) {
      if (err) {
        console.error(err);
        error(err);

        return;
      }
      callback();
      document.dispatchEvent(new CustomEvent(eventId, {
        detail:
          { result: result, pos: query.eventData }
      }))
    })
  }
  Clear(table,end){
    var callback = function (err, result) {
      if (err) {
        console.error(err);
        error(err);

        return;
      }
      console.log(result)
    }
    this.con.query("DELETE FROM "+table,callback);
    this.con.query("ALTER TABLE "+table+" AUTO_INCREMENT = 1",end);
  }
  Delete(table,where,callback,eventId){
    let conds = []
    where.forEach(cond=>{
      conds.push(" "+cond.name+" = '"+cond.value+"' ");
    });
    var error = function (err, result) {
      if (err) {
        console.error(err);
        return;
      }
      callback(result);
    }
    this.con.query("DELETE FROM "+table+" WHERE "+conds.join('AND'),error);
  }
  Update(query, callback, error, eventId) {
    let change = ""
    let changes = []
    if(Array.isArray(query.changes)){
      query.changes.forEach(ch=>{
        changes.push(ch.name+ "='" + mysql_real_escape_string(ch.value) + "'")
      });
      change = changes.join(",")
    }else{ 
      change = query.changes.name + "='" + mysql_real_escape_string(query.changes.value) + "'";
    }
    let update = "UPDATE " + query.table + " SET " + change + " WHERE " + query.where;
  
    
    this.con.query(update, function (err, result) {
      if (err) {
        error(err);
        console.error(err);
        return;
      }
      callback(result);
      console.log(result)
      document.dispatchEvent(new Event(eventId))
    })
  }
  UpdateScript(revision) {
    let query = {
      table:"scripts",
      data:{
        content:"'"+mysql_real_escape_string(revision.sourceCode)+"'",
        id: revision.script.id,
        revision:revision.script.revision + 1
      }
    }
    console.log(revision)
    this.Insert(query,()=>{},(err)=>{console.log(err)},"updated-script");
  }
  UpdateConfig(config, type) {

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



    let query = {
      table: "projectConfig",
      changes: {},
      where: "projectId=" + config.project + " AND type = " + type
    }
    config.forEach((v, i) => {
      if (typeof v == "object") {
        v.forEach((vv, ii) => {
          let copy = Object.assign({}, query)
          copy.where += " AND parent = '" + i + "' AND name ='" + ii + "'";
          copy.changes = { name: "value", value: vv };
          this.Update(copy);
        })
      } else {
        let copy = Object.assign({}, query)
        copy.where += " AND parent = 'root' AND name ='" + i + "'";
        copy.changes = { name: "value", value: v };
        this.Update(copy, () => {}, (error) => {
          console.log(error)
        }, "update-config");
      }
    });
  }
  InsertCards(cards) { }
  Start(handler) {
    this.con.connect(handler);
  }
}
module.exports.Webservice = () => {
  var con = mysql.createConnection(config.mysql);
  return new Webservice(con);
}