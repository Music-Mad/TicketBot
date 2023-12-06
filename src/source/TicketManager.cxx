#include "../headers/TicketManager.h"

TicketManager::TicketManager()
{};

const std::string TicketManager::compileBody(const dpp::snowflake& client_id) {
    if (ticketsGenerating.find(client_id) != ticketsGenerating.end()) {
        return ticketsGenerating.at(client_id).compileBody();
    } else {
        return "";
    }
};

const std::string TicketManager::compileAttatchments(const dpp::snowflake& client_id){
    if (ticketsGenerating.find(client_id) != ticketsGenerating.end()) {
        return ticketsGenerating.at(client_id).compileAttachments();
    } else {
        return "";
    }
};

const bool TicketManager::userHasTktGenerating(const dpp::user& user) {
    if (ticketsGenerating.find(user.id) == ticketsGenerating.end()) {
        return false;
    }
    
    return true;
};

bool TicketManager::addTicket(const dpp::user& client) {
    if (ticketsGenerating.find(client.id) != ticketsGenerating.end()) {
        return false;
    }
    Ticket t("", "", "", client, true);
    ticketsGenerating[client.id] = t;
    return true;
};

bool TicketManager::cancelTicket(const dpp::user& client) {
    if (!userHasTktGenerating(client)) {
        return false;
    }

    ticketsGenerating.erase(client.id);
    return true;
};

bool TicketManager::createTicketChannel(const dpp::user& client, dpp::cluster& bot) {
    try {
        //cache ticket
        const Ticket& t = ticketsGenerating.at(client.id);
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

bool TicketManager::closeTicketChannel( const dpp::channel& channel, dpp::cluster& bot) {
    try {
        bot.channel_delete(channel.id);
        return true;
    } catch (...) {
        std::cout << "ERROR: Failed to delete channel" << std::endl;
        return false;
    }
};

bool TicketManager::saveResponse(const dpp::message& response, dpp::cluster& bot) {
    //cache client and ticket
    dpp::user client = response.author;
    if (ticketsGenerating.find(client.id) == ticketsGenerating.end()) { 
        return false; //if ticket doesn't exist
    }
   
    Ticket& request = ticketsGenerating[client.id];
    return request.storeResponse(response, bot);
};

bool TicketManager::handleBtnPress(dpp::cluster& bot, const dpp::button_click_t& event) {
    //cache common data
    const bool tktGenerating = userHasTktGenerating(event.command.usr);
    const dpp::snowflake& usrId = event.command.usr.id;

    if (!tktGenerating)
    {
        return false;
    }

    if (event.custom_id == "btn_submit") {
            if (!createTicketChannel(event.command.usr, bot)) {
                dpp::message errorMsg("Failed to generate ticket. Please submit a bug report at https://forms.gle/NL8JgbAS13BXEBJD6");
                bot.direct_message_create(usrId, errorMsg);
                return false;
            }
            cancelTicket(event.command.usr);
            dpp::message confirmationMsg("Thank you for your request! Your response has been saved and sent to our verified creators for review. You will recieve a DM from a creator if they would like to fulfill your commission. After one week of inactivity, your request will automatically be closed. Please reach out to an Ink Overflow admin if you have any questions");
            bot.direct_message_create(usrId, confirmationMsg);
    } else if (event.custom_id == "btn_cancel") {
            cancelTicket(event.command.usr);
            bot.direct_message_create(usrId, dpp::message("Your ticket has been successfully deleted. Please use /request if you would like to create a new ticket"));
            return false;
    } else if (event.custom_id == "btn_change_info") {
            return false;
    } else {
        return ticketsGenerating.at(usrId).handleBtnPress(bot, event);
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

