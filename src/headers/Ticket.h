#pragma once
#include <string>
#include <vector>
#include <dpp/dpp.h>
#include "CurrencyFormatter.h"

class Ticket {
private:
    //Ticket info
    std::string budget;
    std::string description;
    std::string name;
    std::vector<dpp::attachment> attachments;
    dpp::user client;
    bool budgetIsTokens;
    //Vars to track ticket status
    bool generating; // if ticket is being generated by user 
    bool editing; //if ticket is being edited by user
    std::string editingExpectation; //Field that ticket is expecting to edit. String is empty when not expecting edits
    bool currencyBtnsDisabled;
    bool attachmentsSubmitted;

public:
    Ticket(const std::string& budget_in, const std::string& description_in, const std::string& name_in, const dpp::user& client_in, bool isGenerating_in);
    bool storeResponse(const dpp::message& response, dpp::cluster& bot);
    bool handleBtnPress(dpp::cluster& bot, const dpp::button_click_t& event);
    const std::string compileBody();
    const std::string compileAttachments();
    
    //getters
    const std::string getName();
    const bool isBudgetTokens();

    //setters
    void setIsGenerating(bool val);
    void setEditingExpectation(std::string val);
    void setIsEditing(bool val);
    
    //for getting ticket gen state
    const bool isGenerating();
    const bool isCurrencyBtnsDisabled();
    const bool isAttachmentsSubmitted();
    const bool isBudgetInitialized();
    const bool isDescInitialized();
    const bool isNameInitialized();
    const bool isEditing();

    //Operators
    const bool operator==(const Ticket rhs);
};