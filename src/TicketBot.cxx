#include <variant> //for std::get
#include "TicketManager.h"
#include "Responder.h"

//constants

std::string BOT_TOKEN;
std::string GUILD_ID;
std::string VERIFS_ID;
std::string ADMIN_ID;
std::string PUBLIC_CATEGORY_ID;
std::string PRIVATE_CATEGORY_ID;

//default callback for error debugging
void defaultCallback(dpp::confirmation_callback_t callback) {
    if (callback.is_error()) {
        dpp::utility::log_error();
        std::string errorMsg = callback.get_error().message;
        std::cout << errorMsg << std::endl;
    }
};

TicketManager tktManager("TicketBot/config.json");

int main() {

    //Read and store json data
    JsonReader reader("TicketBot/config.json");
    if (
    !reader.readStringFromJSON("api_key", BOT_TOKEN, "Error: Missing or invalid 'api_key' in the config.json file.") ||
    !reader.readStringFromJSON("guild_id", GUILD_ID, "Error: Missing or invalid 'guild_id' in the config.json file.") ||
    !reader.readStringFromJSON("public_category_id", PUBLIC_CATEGORY_ID, "Error: Missing or invalid 'public_category_id' in the config.json file.") ||
    !reader.readStringFromJSON("private_category_id", PRIVATE_CATEGORY_ID, "Error: Missing or invalid 'private_category_id' in the config.json file.") ||
    !reader.readStringFromJSON("verified_creator_id", VERIFS_ID, "Error: Missing or invalid 'verified_category_id' in the config.json file.") ||
    !reader.readStringFromJSON("admin_id", ADMIN_ID, "Error: Missing or invalid 'admin_id' in the config.json file.")) {
        std::cout << "ERROR: config.json formatted incorrectly." << std::endl;
    }

    /* Create bot cluster */
    dpp::cluster bot(BOT_TOKEN, dpp::i_message_content | dpp::i_direct_messages | dpp::i_guild_message_reactions);
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
            if (tktManager.publishTicket(event.command.channel, bot)) {
                event.reply(dpp::message("This ticket is now public.").set_flags(dpp::m_ephemeral));
            } else {
                event.reply(dpp::message("Command failed: Could not open ticket.").set_flags(dpp::m_ephemeral));
            }
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
        else if (event.command.get_command_name() == "report_bug") {
            event.reply(dpp::message("Please report bugs with this form: https://forms.gle/NL8JgbAS13BXEBJD6").set_flags(dpp::m_ephemeral));
        } else {
            std::cout << "Command " << event.command.get_command_name() << " not recognized" << std::endl;
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
            dpp::slashcommand cancelRequest("cancel", "Cancel a ticket in-progress", bot.me.id);
            dpp::slashcommand closeTicket("close_ticket", "Close a ticket", bot.me.id);
            dpp::slashcommand bugReport("report_bug", "Report a bug with our service!", bot.me.id);

            bot.guild_command_create(requestCommand, GUILD_ID);
            bot.guild_command_create(openTicket, GUILD_ID);
            bot.guild_command_create(cancelRequest, GUILD_ID);
            bot.guild_command_create(closeTicket, GUILD_ID);
            bot.guild_command_create(bugReport, GUILD_ID);
        }
    });
    /* Start the bot */
    bot.start(dpp::st_wait);
    return 0;
};