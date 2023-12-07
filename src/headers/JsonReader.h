#pragma once
#include "rapidjson/document.h" 
#include <string>
#include <fstream>
#include <iostream>

class JsonReader {
     
    private:
        rapidjson::Document doc;

    public:
        JsonReader(const std::string& filePath);
        bool readStringFromJSON(const char* key, std::string& variable, const std::string& errorMsg);
};