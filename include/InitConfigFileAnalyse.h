#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <fstream>
#include <functional>
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/document.h"

using ExtractFuncType = std::function<void(rapidjson::GenericObject<false, rapidjson::Value>&, std::string)>;


class ConfigFileAnalyse
{
private:
    std::string ConfigFileName_;
    std::unordered_map<std::string, ExtractFuncType> MapPropertyToExtractFunc_;

    rapidjson::Document Document_;

public:
    explicit ConfigFileAnalyse() :
        ConfigFileName_()
    {}

    explicit ConfigFileAnalyse(std::string ConfigFileName) :
        ConfigFileName_(ConfigFileName)
    {}

    ~ConfigFileAnalyse() = default;

    bool Init();
    bool Init(std::string ConfigFileName);

    bool AddMapPropertyToExtractFunc(std::string Property, ExtractFuncType);

    void Analyse();
};