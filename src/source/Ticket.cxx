#include "../headers/Ticket.h"

Ticket::Ticket(const std::string& budget_in, const std::string& description_in, const std::string& name_in, const dpp::user& client_in, bool isGenerating_in) {
    budget = budget_in;
    description = description_in;
    name = name_in;
    generating = isGenerating_in;
    client = client_in;
    budgetIsTokens = false;
    currencyBtnsDisabled = false;
    attachmentsSubmitted = false;
    editing = false;
};

bool Ticket::storeResponse(const dpp::message& response, dpp::cluster& bot) {
    if (!currencyBtnsDisabled) {
        bot.direct_message_create(client.id, dpp::message("Select one of the buttons above!"));
    } else if (budget == "") {
        if (budgetIsTokens) {
            std::string formattedVal = CurrencyFormatter::tokenStringIsValid(response.content);
            if (formattedVal == "invalid_character") {
                bot.direct_message_create(client.id, dpp::message("That is not a valid token amount! Try submitting another value. As a reminder, this value should not contain any letters, special characters, or decimals."));
                return false;
            } else if (formattedVal == "under_minimum") {
                bot.direct_message_create(client.id, dpp::message("The minimum amount for token requests is 10,000. Please try submitting a higher amount:"));
                return false;
            } else if (formattedVal == "out_of_range") {
                bot.direct_message_create(client.id, dpp::message("That value is out of range. Try submitting something less extreme:"));
                return false;
            } else if (formattedVal == "exception_error") {
                bot.direct_message_create(client.id, dpp::message("Error. Please try again"));
                return false;
            }
            else {
                budget = formattedVal;
                return true;
            }
        } else {
            std::string formattedVal = CurrencyFormatter::usdStringIsValid(response.content);
            if (formattedVal == "invalid_character") {
               bot.direct_message_create(client.id, dpp::message("USD Amounts can only contain numbers. Try again:"));
               return false;     
            } else {
                budget = formattedVal;
                return true;
            }
        }
    } else if (!isDescInitialized()) {
        description = response.content;
        return true;
    } else if (!attachmentsSubmitted) {
        if (response.attachments.size() > 0) {
            for (dpp::attachment att : response.attachments) {
                attachments.push_back(att);
            }
            attachmentsSubmitted = true;
            return true;
        } else {
            bot.direct_message_create(client.id, dpp::message("You can only submit images here! If you are trying to create a request without any images, use the button above."));
            return false;
        }
    } else if (name == "") {
        name = response.content;
        return true;
    }
    return false;
};

bool Ticket::handleBtnPress(dpp::cluster& bot, const dpp::button_click_t& event) {
    if (!currencyBtnsDisabled) {
        if (event.custom_id == "btn_tokens") {
            budgetIsTokens = true;
            currencyBtnsDisabled = true;
            return true;
        } else if (event.custom_id == "btn_usd") {
            currencyBtnsDisabled = true;
            return true;
        }
    } else if (!attachmentsSubmitted) {
        if (event.custom_id == "btn_skip_image") {
            attachmentsSubmitted = true;
            return true;          
        }
    }
    return false;
};

const std::string Ticket::compileBody() {
    std::string currency;
    if (budgetIsTokens)
    {
    }
    else
    {
        currency = "USD";
    }

    
    //compile ticket and image strings
    std::string ticket = R"(
User - )" + client.get_mention() + R"(
Budget - )" +  budget + " " + currency + R"(
Description - )" + description;

    return ticket;
};

const std::string Ticket::compileAttachments() {
    std::string imgs = "";
    for (dpp::attachment att : attachments)
        {
            imgs += " " + att.url;
        }
    return imgs;
};

const std::string Ticket::getName() {
    return name;
};

const bool Ticket::isBudgetTokens() {
    return budgetIsTokens;
}

void Ticket::setIsGenerating(bool val) {
    generating = val;
}

void Ticket::setIsEditing(bool val) {
    editing = val;
}

const bool Ticket::isGenerating() {
    return generating;
};

const bool Ticket::isCurrencyBtnsDisabled() {
    return currencyBtnsDisabled;
};

const bool Ticket::isAttachmentsSubmitted() {
    return attachmentsSubmitted;
};

const bool Ticket::isBudgetInitialized() {
    return budget != "";
}

const bool Ticket::isDescInitialized() {
    return description != "";
}

const bool Ticket::isNameInitialized() {
    return name != "";
}

const bool Ticket::isEditing() {
    return editing;
}