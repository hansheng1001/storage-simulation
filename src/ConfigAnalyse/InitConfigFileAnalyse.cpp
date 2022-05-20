#include "InitConfigFileAnalyse.h"
#include "ServerConfigMapStoreBuffer.h"
#include "CPUAndMemoryConfigAnalyse.h"
#include "DiskConfigAnalyse.h"
#include "NetDriverConfigAnalyse.h"
#include <iostream>

bool ConfigFileAnalyse::Init()
{
    std::ifstream ifs{ ConfigFileName_ };
    if (!ifs.is_open())
        return false;

    rapidjson::IStreamWrapper isw(ifs);

    Document_.ParseStream(isw);

    ifs.close();

    this->AddMapPropertyToExtractFunc("CPUAndMemory", CPUAndMemoryConfigAnalyseAndStore::Analyse);
	this->AddMapPropertyToExtractFunc("Disk", DiskConfigAnalyseAndStore::Analyse);
    this->AddMapPropertyToExtractFunc("NetDriver", NetDriverConfigAnalyseAndStore::Analyse);
    
    return true;
}

bool ConfigFileAnalyse::Init(std::string ConfigFileName)
{
    ConfigFileName_ = ConfigFileName;
    return Init();
}

bool ConfigFileAnalyse::AddMapPropertyToExtractFunc(std::string Property, ExtractFuncType ExtractFunc)
{
    MapPropertyToExtractFunc_[Property] = ExtractFunc;
    return true;
}

void ConfigFileAnalyse::Analyse()
{
    std::string ServerClassName;
    
    for (auto ServerClass = Document_.MemberBegin();ServerClass != Document_.MemberEnd();ServerClass++)
    {
        if (ServerClass->name.IsString())
        {
            ServerClassName = std::string{ ServerClass->name.GetString() };
            if (ServerClass->value.IsObject())
            {
                auto ServerConfigObj = ServerClass->value.GetObject();
                for (auto& AnalyseNameFuncPair : MapPropertyToExtractFunc_)
                {
                    if (ServerConfigObj.HasMember(AnalyseNameFuncPair.first.c_str()))
                    {
                        AnalyseNameFuncPair.second(ServerConfigObj, ServerClassName);
                    }
                    else
                    {
                        /*在没有该类型参数时，是否使用默认设置？*/
                        std::cout << __FILE__ << " : " << __LINE__ << "json file error, " << ServerClassName << " doesn't has Object - " << AnalyseNameFuncPair.first.c_str() << std::endl;
                        abort();
                    }
                }
            }
            else
            {
                std::cout << __FILE__ << " : " << __LINE__ << "json file error, ServerClass->value is not an Object" << std::endl;
            }
        }
        else
        {
            std::cout << __FILE__ << " : " << __LINE__ << "json file error, ServerClass is not a string" << std::endl;
        }
    }
}