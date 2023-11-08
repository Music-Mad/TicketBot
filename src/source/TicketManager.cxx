#include "../headers/TicketManager.h"

TicketManager::TicketManager()
{};

const std::string TicketManager::compileBody(const dpp::snowflake& client_id, const int ticketIndex) {
    if ((tickets.find(client_id) != tickets.end()) && (ticketIndex < tickets.at(client_id).size())) {
        return tickets.at(client_id)[ticketIndex].compileBody();
    } else {
        return "";
    }
};

const std::string TicketManager::compileAttatchments(const dpp::snowflake& client_id, const int ticketIndex){
    if ((tickets.find(client_id) != tickets.end()) && (ticketIndex < tickets.at(client_id).size())) {
        return tickets.at(client_id)[ticketIndex].compileAttachments();
    } else {
        return "";
    }
};

const bool TicketManager::userHasTktGenerating(const dpp::user& user) {
    if (tickets.find(user.id) == tickets.end()) {
        return false;
    }
    for (int i = 0; i < tickets.at(user.id).size(); i++) {
        if (tickets[user.id][i].isGenerating()) {
            return true;
        }
    }
    return false;
};

const bool TicketManager::userHasTktEditing(const dpp::user& user) {
    if (tickets.find(user.id) == tickets.end()) {
        return false;
    }
    for (int i = 0; i < tickets.at(user.id).size(); i++) {
        if (tickets[user.id][i].isEditing()) {
            return true;
        }
    }
    return false;
};


const bool TicketManager::listTickets(const dpp::user& client, dpp::cluster& bot) {
    if(tickets.find(client.id) != tickets.end()) {
        if (userHasTktGenerating(client)) {
            bot.direct_message_create(client.id, dpp::message("Finish creating your ticket or use /cancel before entering the /status menu."));
            return true;
        } else if (userHasTktEditing(client)) {
            bot.direct_message_create(client.id, dpp::message("You cannot use this command right now. Finish editing the current ticket first."));
            return true;
        }
        try {
            for (int i = 0; i < tickets.at(client.id).size(); ++i) {
                //cache ticket
                Ticket t = tickets.at(client.id)[i];
                if (t.isGenerating()) {
                    std::cout << "skipped " + t.getName() << std::endl;
                    continue; //skip if generating
                }
                //generate msg
                dpp::message msg("### " + std::to_string(i + 1) + ". " + t.getName() + "\n--------------------" + "\n" + t.compileBody() + "\n" + t.compileAttachments());
                //add btn
                msg.add_component(
                    dpp::component().add_component(
	                dpp::component().set_label("Edit ticket #" + std::to_string(i + 1)).
	                set_type(dpp::cot_button).
                    set_emoji(u8"✍️").
	                set_style(dpp::cos_primary).
	                set_id("btn_status_edit_" + std::to_string(i))
                    ) 
                );
                //send
                bot.direct_message_create(client.id, msg);
            }
            return true;
        } catch (const std::exception& e) {
            std::cout << "Error: listTickets exception" << std::endl;
        }
    } 
    return false;
};

void TicketManager::addTicket(const dpp::user& client) {
    std::string emptyString = "";
    Ticket t(emptyString, emptyString, emptyString, client, true);
    tickets[client.id].insert(tickets.at(client.id).begin(), t);
};


bool TicketManager::cancelTicket(const dpp::user& client) {
    if (!userHasTktGenerating(client)) {
        return false;
    }

    tickets.at(client.id).erase(tickets.at(client.id).begin());
    return true;
};

bool TicketManager::createTicketThread(const dpp::user& client, dpp::cluster& bot) {
    
    bot.thread_create(tickets[client.id].front().getName(), PRIVATE_CHANNEL_ID, 60, dpp::CHANNEL_PUBLIC_THREAD, false, 0, [&bot, this, client] (auto callback){
        if (!callback.is_error())
        {   
            //retrieve thread
            dpp::thread myThread = std::get<dpp::thread>(callback.value);
            //generate ticket
            Ticket& t = tickets.at(client.id)[0];
            dpp::message msg(myThread.id, t.compileBody() + "\n" + t.compileAttachments());
            //send
            bot.message_create(msg);
            return true;
        } else {
            dpp::utility::log_error();
            std::string errorMsg = callback.get_error().message;
            std::cout << errorMsg << std::endl;
            return false;
        }
    });
    return false;
};

bool TicketManager::saveResponse(const dpp::message& response, int ticketIndex, dpp::cluster& bot) {
    //cache client and ticket
    dpp::user client = response.author;
    if (tickets.find(client.id) == tickets.end()) { 
        return false; //if ticket doesn't exist
    }
    Ticket& request = tickets[client.id][ticketIndex];
    return request.storeResponse(response, bot);
};

const bool TicketManager::reviewTicket(const dpp::user& client, int ticketIndex, dpp::cluster& bot) { 
    //check map entry exists
    if (tickets.find(client.id) == tickets.end()) {
        return false;
    }
    //cache ticket
    Ticket& request = tickets.at(client.id)[ticketIndex];
    request.setIsEditing(true);
    //generate responses
    dpp::message msg("## " + request.getName() + " ##\n" + request.compileBody() + request.compileAttachments());
    msg.add_component(
        dpp::component().add_component(
	    dpp::component().set_label("Edit budget").
	    set_type(dpp::cot_button).
	    set_style(dpp::cos_primary).
	    set_id("btn_edit_budget_" + std::to_string(ticketIndex))
        )
        //desc
        .add_component(
	    dpp::component().set_label("Edit description").
	    set_type(dpp::cot_button).
	    set_style(dpp::cos_primary).
	    set_id("btn_edit_description_" + std::to_string(ticketIndex))
        )
        //title
        .add_component(
	    dpp::component().set_label("Edit title").
	    set_type(dpp::cot_button).
	    set_style(dpp::cos_primary).
	    set_id("btn_edit_title_" + std::to_string(ticketIndex))
        )
        //delete
        .add_component(
	    dpp::component().set_label("Delete ticket").
	    set_type(dpp::cot_button).
        set_emoji(u8"❌").
	    set_style(dpp::cos_primary).
	    set_id("btn_edit_delete_" + std::to_string(ticketIndex))
        )
        //back
        .add_component(
	    dpp::component().set_label("Back").
	    set_type(dpp::cot_button).
        set_emoji(u8"🔙").
	    set_style(dpp::cos_primary).
	    set_id("btn_edit_back_" + std::to_string(ticketIndex))
        ) 
    );
    bot.direct_message_create(client.id, msg);
    return true;
};

std::string TicketManager::handleBtnPress(dpp::cluster& bot, const dpp::button_click_t& event) {
    //cache common data
    const bool tktGenerating = userHasTktGenerating(event.command.usr);
    const bool tktEditing = userHasTktEditing(event.command.usr);
    const dpp::snowflake& usrId = event.command.usr.id;

    if (tktGenerating)
    {
        if (event.custom_id == "btn_submit") {
            tickets.at(usrId)[0].setIsGenerating(false);
            dpp::message confirmationMsg("Thank you for your request! Your response has been saved and sent to our verified creators for review. You will recieve a DM from a creator if they would like to fulfill your commission. After one week of inactivity, your request will automatically be closed. Please reach out to an Ink Overflow admin if you have any questions");
            bot.direct_message_create(usrId, confirmationMsg);
            createTicketThread(event.command.usr, bot);
        } else if (event.custom_id == "btn_cancel") {
            cancelTicket(event.command.usr);
            return "ticket_confirmation_cancelled";
        } else if (event.custom_id == "btn_change_info") {

        } else
        {
            if (tickets.at(usrId)[0].handleBtnPress(bot, event)) {
                return "stage_ticket_response";
            }
        }
    }

    
    if (event.custom_id.substr(0, 16) == "btn_status_edit_") {
        if (tktGenerating || tktEditing) {
            bot.direct_message_create(usrId, dpp::message("You must finish creating your ticket before using this button. If you want to cancel your current ticket, use /cancel"));
        } else {
            reviewTicket(event.command.usr, std::stoi(event.custom_id.substr(16, 1)), bot);
            return "status_edit_ticket";
        }
    }

    if (event.custom_id.substr(0,9) == "btn_edit_") {
        if (tktEditing) {
            if (event.custom_id.substr(0, 13) == "btn_edit_back") {
                int ticketIndex = std::stoi(event.custom_id.substr(13, 1));
                Ticket& t = tickets.at(usrId)[ticketIndex];
                t.setIsEditing(false);
                return "edit_back";
            }
        } else {
            if (tktGenerating) {
                return "edit_failed_generating";
            }
            return "edit_failed_unknown";
        }

    }


    return "";
};

