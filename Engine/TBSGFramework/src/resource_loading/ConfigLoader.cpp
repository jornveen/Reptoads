#include "resource_loading/ConfigLoader.h"
#include "core/Config.h"
#include <rapidjson/allocators.h>
#include <rapidjson/encodings.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include "core/Assertion.h"
#include <memory/RapidJsonAllocator.h>

void tbsg::LoadConfig(const char* a_ConfigFile)
{
#ifdef PLATFORM_PS
	rapidjson::GenericDocument < rapidjson::UTF8<>,RapidJsonAllocator> configDoc;
#else
    rapidjson::GenericDocument < rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<>> configDoc;
#endif
    char readBuffer[512];
#ifdef PLATFORM_WINDOWS
    FILE* fp;
    errno_t err = fopen_s(&fp, a_ConfigFile, "r");
    ASSERT_MSG(!err, "Could not load config");
#else
    FILE* fp = fopen(a_ConfigFile, "r"); // non-Windows use "r"
    if(!fp)
        ASSERT_MSG(false, "Could not load config");
#endif
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    configDoc.ParseStream(is);
    ASSERT_MSG(!configDoc.HasParseError(),"Synatx Error");
    ASSERT_MSG(configDoc.HasMember("network"), "Could not find network config in json");
    ASSERT_MSG(configDoc.HasMember("assets"), "Could not find assets config in json");
    ASSERT_MSG(configDoc.HasMember("window"), "Could not find window config in json");
    ASSERT_MSG(configDoc.HasMember("user"), "Could not find user config in json");
    ASSERT_MSG(configDoc.HasMember("engineSubstepping"), "Could not find user config in json");
    fclose(fp);

    auto& config = Config::Get();
	config = {
		configDoc["project"].GetUint(),
		configDoc["production"].GetBool(),
	    {configDoc["network"]["address"].GetString()},
	    configDoc["network"]["port"].GetUint(),
		true,
	    configDoc["network"]["maxconnection"].GetUint(),
	    {configDoc["database"]["host"].GetString()},
	    configDoc["database"]["port"].GetUint(),
	    {configDoc["database"]["username"].GetString()},
	    {configDoc["database"]["password"].GetString()},
	    {configDoc["database"]["database"].GetString()},
		{configDoc["api"]["host"].GetString()},
		{configDoc["api"]["username"].GetString()},
		{configDoc["api"]["password"].GetString()},
		{configDoc["api"]["tokenfile"].GetString()},
	    {configDoc["assets"]["cards"].GetString()},
	    {configDoc["assets"]["scripts"].GetString()},
	    {configDoc["assets"]["profiles"].GetString()},
	    {configDoc["assets"]["shaders"].GetString()},
	    {configDoc["assets"]["shaders_bin"].GetString()},
	    {configDoc["assets"]["textures"].GetString()},
	    {configDoc["assets"]["models"].GetString()},
	    {configDoc["assets"]["ui"].GetString()},
	    {configDoc["assets"]["audio"].GetString()},
		{configDoc["assets"]["particles"].GetString()},
	    {configDoc["user"]["name"].GetString()},
	    {configDoc["assets"]["maps"].GetString()},
	    {configDoc["assets"]["aiconfig"].GetString()},
	    {configDoc["assets"]["balancingdata"].GetString()},
	    configDoc["window"]["width"].GetUint(),
	    configDoc["window"]["height"].GetUint(),
	    configDoc["engineSubstepping"]["maxsteptime"].GetDouble(),
	    configDoc["engineSubstepping"]["maxsteps"].GetUint()
    };
#ifdef PLATFORM_PS
    const auto app0 = "/hostapp/";
    config.profilesFolder = app0 + config.profilesFolder;
    config.modelsFolder = app0 + config.modelsFolder;
	config.uiFolder = app0 + config.uiFolder;
	config.audioFolder = app0 + config.audioFolder;

    config.mapsFile = app0 + config.mapsFile;
    config.scriptsFolder = app0 + config.scriptsFolder;
    config.shadersFolderBinary = app0 + config.shadersFolderBinary;
    config.shadersFolder = app0 + config.shadersFolder;
    config.texturesFolder = app0 + config.texturesFolder;
    config.particleFolder = app0 + config.particleFolder;
#endif

}

void tbsg::LoadConfig(const ptl::string& a_ConfigFile)
{
    return LoadConfig(a_ConfigFile.c_str());
}
