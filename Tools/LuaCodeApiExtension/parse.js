var fs = require('fs');

fs.readFile('I:/Y2018D-Y2-DigitalDeckbuilder/Engine/GameServer/source/scripting/LuaBindingFunc.cpp', 'utf8', function(err, contents) {
    ParseBindings(contents);
});
 
function ParseBindings(file) {

    String.prototype.replaceAll = function(search, replacement) {
        var target = this;
        return target.split(search).join(replacement);
      };
 
    var splitted = [];
    var functions = {};
    var mkdocsFunction = [];

    splitted =  file.split("/**");  

    for (let index = 1; index < splitted.length; index++) {
        var comment = splitted[index].substring(0, splitted[index].indexOf("*/"));
        var returntype = splitted[index].substring(splitted[index].indexOf("<") + 1, splitted[index].indexOf("("));
        var params = splitted[index].substring(splitted[index].indexOf("(") + 1,splitted[index].indexOf(")"));
        params = params.replaceAll("ptl::","");
        params = params.replaceAll("tbsg::","");
        var name = splitted[index].substring(splitted[index].indexOf(")") + 3, splitted[index].indexOf("=") - 1);

        comment = comment.replaceAll('*', '');
        var desc = comment; //markdown
        desc = desc.replace(/(^\s+)/gm, '');
        desc = desc.replace('@param', '\n\n**Parameters:**\n-');
        comment = comment.replace('@param', '\n\n**Parameters:**\r\n      \r\n      -');
        comment = comment.replaceAll('@brief', '\n**Brief:**  ');
        desc = desc.replaceAll('@brief', '\n**Brief:**  ');
        comment = comment.replaceAll('@param', '-');
        desc = desc.replaceAll(/(^\s+)/m, '');
        desc = desc.replaceAll('@param', '-');
        comment = comment.replaceAll('@return', '\n**Returns:**  ');
        desc = desc.replaceAll('@return', '\n\n**Returns:**  ');
        desc = desc.replaceAll("ptl::","");
        desc = desc.replaceAll("tbsg::","");


        var doc = returntype + " **" + name + "**(" + params + ") \n\n" + comment;
        functions[name] = { "name": name, "type":"function", "returns":returntype, "doc":doc };
        mkdocsFunction.push({"name":name,"type":"function","returns":returntype,"brief":"","params":params,"desc":desc});
    }
    //console.log(functions);

    fs.writeFile('./data/classes.json', JSON.stringify(functions), function(err, contents) {
    });

    //markdown documentation:
    var markdownText = "# Lua API \n";
    mkdocsFunction.forEach(v=>{
        markdownText += "## "+v.type+" "+v.name+" ("+v.params+")\n";
        markdownText += v.desc;
        markdownText += "\n\n";
    })
    fs.writeFile('./data/lua_api.md',markdownText, function(err, contents) {

    });

}