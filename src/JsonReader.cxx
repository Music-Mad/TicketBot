#include "JsonReader.h"

JsonReader::JsonReader(const std::string& filePath) {
        std::ifstream config(filePath);
        if (!config.is_open()) {
            std::cout << "CRITICAL ERROR: Cannot open config.json" << std::endl;
            std::abort();
        }

        std::string jsonContent((std::istreambuf_iterator<char>(config)), std::istreambuf_iterator<char>());
        config.close();

        doc.Parse(jsonContent.c_str());

        if (doc.HasParseError() || !doc.IsObject()) {
            std::cout << "CRITICAL ERROR: Failed to parse config.json" << std::endl;
            std::abort();
        }
};

bool JsonReader::readStringFromJSON(const char* key, std::string& variable, const std::string& errorMsg) {
        if (doc.HasMember(key) && doc[key].IsString()) {
            variable = doc[key].GetString();
            return true;
        } else {
            std::cout << errorMsg << std::endl;
            return false;
        }
    };