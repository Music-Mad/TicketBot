#include <dpp/dpp.h>
#include <string>
#include <variant> //for std::get
#include "../headers/TicketManager.h"
#include "../headers/Responder.h"
//for reading config json
#include <fstream>
#include "rapidjson/document.h" 

//constants
const std::string GUILD_ID = "976104010924847137";
const std::string VERIFS_ID = "1139260164906700920";
const std::string ADMIN_ID = "1138102566476058735";
const std::string PUBLIC_CATEGORY_ID = "1164614806502899845";
const std::string PRIVATE_CATEGORY_ID = "1164614854125027328";

//default callback for error debugging
void defaultCallback(dpp::confirmation_callback_t callback) {
    if (callback.is_error()) {
        dpp::utility::log_error();
        std::string errorMsg = callback.get_error().message;
        std::cout << errorMsg << std::endl;
    }
};

TicketManager tktManager;

int main() {

    std::ifstream config("/root/TicketBot/config.json");
    if (!config.is_open()) {
        std::cout << "Cannot open config.json" << std::endl;
        return 0;
    }

    std::string botToken;
    std::string jsonContent((std::istreambuf_iterator<char>(config)), std::istreambuf_iterator<char>());
    config.close();
    // Parse the JSON content
    rapidjson::Document doc;
    doc.Parse(jsonContent.c_str());
    // Check if parsing was successful
    if (!doc.HasParseError() && doc.IsObject()) {
        // Access the API key
        if (doc.HasMember("dpp_key") && doc["dpp_key"].IsString()) {
            botToken = doc["dpp_key"].GetString();
        } else {
            std::cerr << "Error: Missing or invalid 'api_key' in the configuration file." << std::endl;
            return 0;
        }
    } else {
        std::cerr << "Error: Invalid JSON format in the configuration file." << std::endl;
        return 0;
    }

    /* Create bot cluster */
    dpp::cluster bot(botToken, dpp::i_message_content | dpp::i_direct_messages | dpp::i_guild_message_reactions);
    Responder responder(bot);
    
    /* Output simple log messages to stdout */
    bot.on_log(dpp::utility::cout_logger());

    //handle button press
    bot.on_button_click([&bot, responder] (const dpp::button_click_t & event) mutable {
        const dpp::snowflake& usrId = event.command.usr.id;
        if (tktManager.handleBtnPress(bot, event)){
            responder.generateTicketResponse(event.command.msg, event.command.usr, tktManager);
        }
        event.reply();
    });

    //handle slash commands
    bot.on_slashcommand([&bot, responder](const dpp::slashcommand_t& event) mutable {
        if (event.command.get_command_name() == "request") {
            if (!tktManager.userHasTktGenerating(event.command.usr)) {
                responder.generateTicketResponse(event.command.msg, event.command.usr, tktManager);
                event.reply(dpp::message("Check your DM's!").set_flags(dpp::m_ephemeral));
            } else {
                event.reply(dpp::message("You already have a ticket being created! Use /cancel if you want to reset that request.").set_flags(dpp::m_ephemeral));
            }
        } else if (event.command.get_command_name() == "cancel") {
            if (tktManager.cancelTicket(event.command.usr)) {
                event.reply(dpp::message("Your ticket has been successfully canceled. Please use /request if you would ever like to create a new ticket.").set_flags(dpp::m_ephemeral));
            } else {
                event.reply(dpp::message("You aren't currently creating a ticket. Use /status if you want to delete a ticket you've already submitted.").set_flags(dpp::m_ephemeral));
            }
        } else if (event.command.get_command_name() == "open_ticket") {
            tktManager.publishTicket(event.command.channel, bot);
            event.reply(dpp::message("This ticket is now public.").set_flags(dpp::m_ephemeral));
        } else if (event.command.get_command_name() == "close_ticket") {

            if (event.command.channel.parent_id != dpp::snowflake(PUBLIC_CATEGORY_ID) && event.command.channel.parent_id != dpp::snowflake(PRIVATE_CATEGORY_ID)) {
                event.reply(dpp::message("This channel is not an open ticket").set_flags(dpp::m_ephemeral));
                return;
            }

            auto& userRoles = event.command.member.get_roles();
            if (std::find(userRoles.begin(), userRoles.end(), dpp::snowflake(VERIFS_ID)) != userRoles.end()) { //If user has verif role, cmd fails
                if (std::find(userRoles.begin(), userRoles.end(), dpp::snowflake(ADMIN_ID)) == userRoles.end()) { //If user isnt admin
                    event.reply(dpp::message("You can't close someone else's commission.").set_flags(dpp::m_ephemeral));
                    return;
                }
            } 

            event.reply(dpp::message("Closing ticket...").set_flags(dpp::m_ephemeral));
            tktManager.closeTicketChannel(event.command.channel, bot);
        } 
        else {
            std::cout << "Command " << event.command.get_command_name() << " not recognized\n";
        }
    });

    //recieve DMs
    bot.on_message_create([&bot, responder](const dpp::message_create_t& event) mutable {
        //cache author ID
        dpp::snowflake author_id = event.msg.author.id;

        //if author not bot
        if (author_id != bot.me.id) {
            //if user is generating a ticket
            if (event.msg.content == "/request") {
                if (!tktManager.userHasTktGenerating(event.msg.author)) {
                    responder.generateTicketResponse(event.msg, event.msg.author, tktManager);
                } else {
                    event.reply(dpp::message("You already have a ticket being created! Use /cancel if you want to reset that request.").set_flags(dpp::m_ephemeral));
                }
            } else if (event.msg.content == "/cancel") {
                if (tktManager.cancelTicket(event.msg.author)) {
                    bot.direct_message_create(event.msg.author.id, dpp::message("Your ticket has been successfully canceled. Please use /request if you would ever like to create a new ticket."));
                } else {
                    bot.direct_message_create(event.msg.author.id, dpp::message("You aren't currently creating a ticket. Use /status if you want to delete a ticket you've already submitted."));
                }
            } else if (tktManager.userHasTktGenerating(event.msg.author)) {
                if (tktManager.saveResponse(event.msg, bot)){
                    responder.generateTicketResponse(event.msg, event.msg.author, tktManager);
                }
            }
        }
    });

    /* Register slash command here in on_ready */
    bot.on_ready([&bot](const dpp::ready_t& event) {
        /* Wrap command registration in run_once to make sure it doesnt run on every full reconnection */
        if (dpp::run_once<struct register_bot_commands>()) {

            dpp::slashcommand requestCommand("request", "Create a commission request", bot.me.id);
            dpp::slashcommand openTicket("open_ticket", "Open a commission for all verified creators", bot.me.id);
            dpp::slashcommand cancelRequest("cancel", "cancel a ticket in-progress", bot.me.id);
            dpp::slashcommand closeTicket("close_ticket", "Close a ticket thread", bot.me.id);

            bot.guild_command_create(requestCommand, GUILD_ID);
            bot.guild_command_create(openTicket, GUILD_ID);
            bot.guild_command_create(cancelRequest, GUILD_ID);
            bot.guild_command_create(closeTicket, GUILD_ID);
        }
    });
    /* Start the bot */
    bot.start(dpp::st_wait);
    return 0;
};