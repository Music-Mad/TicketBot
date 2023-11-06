#pragma once
#include <string>

class CurrencyFormatter {
public:
    const static std::string tokenStringIsValid(const std::string& str); //determines if string is valid and reformats

    const static std::string usdStringIsValid(const std::string& str);
private:
    const static float toUSD (int tokens);

    const static int stringToInt(const std::string& str);
};