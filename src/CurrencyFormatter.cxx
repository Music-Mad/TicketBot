#include "CurrencyFormatter.h"
#include <variant>
#include <iomanip>
#include <iostream>
#include <sstream>


const float CurrencyFormatter::toUSD (int tokens)
{
    float wrkVal = tokens * .7f;
    float USD = wrkVal/2500.0f;
    return USD;
};

const int CurrencyFormatter::stringToInt(const std::string& str) {
    return std::stoi(str);
};

const std::string CurrencyFormatter::tokenStringIsValid(const std::string& str) //determines if string is valid and reformats
{
    try {
        std::string proccessedPrice;
        for (char c: str)
        {
            if (!(c == ' ') && !(c == ','))  //ignore space and comma entries
            {
                //save digits
                if (isdigit(c))
                {
                    proccessedPrice += c;
                }
                else 
                {
                    return "invalid_character";
                }
            }
        }
        int tokens = stringToInt(proccessedPrice);
        std::string usd;
        if (stringToInt(proccessedPrice) < 10000) {
            return "under_minimum";
        } else {   //Save USD conversion
            float amount;
            amount = toUSD(tokens);
            //create a string stream to hold the formatted string
            std::stringstream ss;
            //set locale
            ss.imbue(std::locale::classic());
            //format string
            ss << std::fixed << std::setprecision(2);
            ss << "$" << amount;
            //save formatted string
            usd = ss.str();
        }
        std::string priceCommaed = proccessedPrice;
        for (int i = 1; i < proccessedPrice.length(); ++i)
        {
            int backI = proccessedPrice.length() - i;
            if ((i % 3) == 0) {
                priceCommaed.insert(backI, ",");
            }
        }
        return priceCommaed + " Tokens (" + usd + " USD)";
    } catch (const std::out_of_range& e) {
        return "out_of_range";
    } catch (const std::exception& e) {
        return "exception_error";
    }
};


const std::string CurrencyFormatter::usdStringIsValid(const std::string& str) {
    for (char c : str) {
        if (!std::isdigit(c)) {
            return "invalid_character";
        }
    }
    return str;
};