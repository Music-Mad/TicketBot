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

Ticket* TicketManager::getEditingTicket(const dpp::snowflake& clientId) {
    for (Ticket& t : tickets.at(clientId)) {
        if (t.isEditing()) {
            return &t;
        }
    }
    return nullptr;
};

const bool TicketManager::listTickets(const dpp::user& client, dpp::cluster& bot) {
    if(tickets.find(client.id) != tickets.end()) {
        if (userHasTktGenerating(client)) {
            bot.direct_message_create(client.id, dpp::message("Finish creating your ticket or use /cancel before entering the /status menu."));
            return false;
        } else if (userHasTktEditing(client)) {
            bot.direct_message_create(client.id, dpp::message("You cannot use this command right now. Finish editing the current ticket first."));
            return false;
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
            return false;
        }
    } else {
        bot.direct_message_create(client.id, dpp::message("You have no active tickets! Create one with /request!"));
        return false;
    }
    return false;
};

void TicketManager::addTicket(const dpp::user& client) {
    std::string emptyString = "";
    Ticket t(emptyString, emptyString, emptyString, client, true);
    tickets[client.id].insert(tickets.at(client.id).begin(), t);
};

bool TicketManager::deleteTicket(const dpp::snowflake& client, const Ticket& target) {
    if (tickets.find(client) == tickets.end()) {
        return false;
    } else {
        for (int i = 0; i < tickets.at(client).size(); ++i) {
            if(tickets.at(client)[i] == target) {
                tickets.at(client).erase(tickets.at(client).begin() + i);
                if (tickets.at(client).size() <= 0) {
                    tickets.erase(client);
                }
                return true;
            }
        }
    }
    return false;
};

bool TicketManager::cancelTicket(const dpp::user& client) {
    if (!userHasTktGenerating(client)) {
        return false;
    }

    tickets.at(client.id).erase(tickets.at(client.id).begin());
    if (tickets.at(client.id).size() <= 0) {
        tickets.erase(client.id);
    }
    return true;
};

bool TicketManager::createTicket(const dpp::user& client, dpp::cluster& bot) {
    try {
        //cache ticket
        const Ticket& t = tickets.at(client.id)[0];
        //create channel for thread
        dpp::channel c;
        c.set_guild_id(GUILD_ID);
        c.set_parent_id(PRIVATE_CATEGORY_ID);
        c.set_name(t.getName());
        bot.channel_create(c, [t, &bot](const dpp::confirmation_callback_t& callback) {
            //get channel id and send ticket data
            dpp::channel new_channel = std::get <dpp::channel>(callback.value);
            dpp::message msg(new_channel.id, t.compileBody() + "\n" + t.compileAttachments());
            bot.message_create(msg);
        });
        return true;
    } catch (...) {
        return false;
    }
};

bool TicketManager::saveResponse(const dpp::message& response, int ticketIndex, bool isGenerating, dpp::cluster& bot) {
    //cache client and ticket
    dpp::user client = response.author;
    if (tickets.find(client.id) == tickets.end()) { 
        return false; //if ticket doesn't exist
    }
    if (isGenerating) {
        Ticket& request = tickets[client.id][ticketIndex];
        return request.storeResponse(response, bot);
    } else if (userHasTktEditing(client))
    {
        Ticket& request = *getEditingTicket(client.id);
        if (request.storeResponse(response, bot)) {
            auto it = std::find(tickets.at(client.id).begin(), tickets.at(client.id).end(), request);
            int i = it - tickets.at(client.id).begin();
            reviewTicket(client, i, bot);
            return true;
        }
        return false;
    } 
    return false;
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
	    set_id("btn_edit_budget")
        )
        //desc
        .add_component(
	    dpp::component().set_label("Edit description").
	    set_type(dpp::cot_button).
	    set_style(dpp::cos_primary).
	    set_id("btn_edit_description")
        )
        //title
        .add_component(
	    dpp::component().set_label("Edit title").
	    set_type(dpp::cot_button).
	    set_style(dpp::cos_primary).
	    set_id("btn_edit_title")
        )
        //delete
        .add_component(
	    dpp::component().set_label("Delete ticket").
	    set_type(dpp::cot_button).
        set_emoji(u8"❌").
	    set_style(dpp::cos_primary).
	    set_id("btn_edit_delete")
        )
        //back
        .add_component(
	    dpp::component().set_label("Back").
	    set_type(dpp::cot_button).
        set_emoji(u8"🔙").
	    set_style(dpp::cos_primary).
	    set_id("btn_edit_back")
        ) 
    );

    bot.direct_message_create(client.id, msg);
    return true;
};

bool TicketManager::handleBtnPress(dpp::cluster& bot, const dpp::button_click_t& event) {
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
            createTicket(event.command.usr, bot);
        } else if (event.custom_id == "btn_cancel") {
            cancelTicket(event.command.usr);
            bot.direct_message_create(usrId, dpp::message("Your ticket has been successfully deleted. Please use /request if you would like to create a new ticket"));
        } else if (event.custom_id == "btn_change_info") {

        } else
        {
            return tickets.at(usrId)[0].handleBtnPress(bot, event);
        }
    }

    
    if (event.custom_id.substr(0, 16) == "btn_status_edit_") {
        if (tktGenerating) {
            bot.direct_message_create(usrId, dpp::message("You must finish creating your ticket before using this button. If you want to cancel your current ticket, use /cancel"));
        } else if (tktEditing) {
            bot.direct_message_create(usrId, dpp::message("You must finish editing your ticket before selecting another one."));
        } else {

            reviewTicket(event.command.usr, std::stoi(event.custom_id.substr(16, 1)), bot);
        }
    }

    if (event.custom_id.substr(0,9) == "btn_edit_") {
        if (tktEditing) {
            //find target
            Ticket& target = *getEditingTicket(usrId);
            
            if (event.custom_id == "btn_edit_back") {
                target.setIsEditing(false);
                listTickets(event.command.usr, bot);
            } else if (event.custom_id == "btn_edit_description") {
                target.setEditingExpectation("description");
                bot.direct_message_create(usrId, dpp::message("Type a new description below:"));
            } else if (event.custom_id == "btn_edit_budget") {
                target.setEditingExpectation("budget");
                bot.direct_message_create(usrId, dpp::message("Type a new budget below:"));
            } else if (event.custom_id == "btn_edit_title") {
                target.setEditingExpectation("name");
                bot.direct_message_create(usrId, dpp::message("Type a new title below:"));
            } else if (event.custom_id == "btn_edit_delete") {
                deleteTicket(usrId, target);
                bot.direct_message_create(usrId, dpp::message("Ticket deleted!"));
            }
        } else {
            if (tktGenerating) {
                bot.direct_message_create(usrId, dpp::message("You cannot edit a ticket while creating a new one. Use /cancel to cancel the creation process."));
            }
            bot.direct_message_create(usrId, dpp::message("That action cannot be processed at this time."));
        }
    }


    return false;
};

bool TicketManager::publishTicket(const dpp::channel& channel, dpp::cluster& bot) {
    //if ticket is already published return
    if (channel.parent_id == dpp::snowflake(PUBLIC_CATEGORY_ID)) {
        return false;
    }

    dpp::channel c = channel;
    try { 
        c.set_parent_id(PUBLIC_CATEGORY_ID);
        bot.channel_edit(c,[](auto const& callback) {});
    } catch( ... ) {
        std::cout << "ERROR: Failed to publish ticket\n";
        return false;
    }

    //retrieve usr
    bot.message_get(c.last_message_id, c.id, [&bot, c](const dpp::confirmation_callback_t& callback) {
        if (callback.is_error()) {
            std::cout << callback.get_error().message << std::endl;
            return false;
        }
    
        std::string usrId = "";
        try {
            const dpp::message& m = std::get<dpp::message>(callback.value);
            for (int i = 0; i < m.content.length(); ++i ) {
                char c = m.content[i];
                if (c == '@') {
                    usrId = m.content.substr(i + 1, 18);
                }
            }
        } catch (...) {
            std::cout << "ERROR: Failed tor retrieve usrId from ticket content\n";
            return false;
        }

        try {
            bot.channel_edit_permissions(c, dpp::snowflake(usrId), uint64_t(dpp::p_view_channel), uint64_t(0), true);
            dpp::message notif(c.id, "<@" + usrId + "> Your commission has been approved and is now available to all Verified Creators. Please use this channel to coordinate with verifs and elaborate on your request.");
            notif.set_allowed_mentions(true, true, true, true, std::vector<dpp::snowflake>(), std::vector<dpp::snowflake>());
            bot.message_create(notif);
        } catch (...) {
            std::cout << "ERROR: Failed to add usr to channel";
            return false;
        }

        return true;
    });


    return true;
};
