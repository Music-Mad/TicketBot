#pragma once
#include <dpp/dpp.h>
#include <string>
#include "Ticket.h"

class TicketManager {
    private:
        const std::string PRIVATE_CATEGORY_ID = "1164614854125027328";
        const std::string PUBLIC_CATEGORY_ID = "1164614806502899845";
        const std::string GUILD_ID = "976104010924847137";

    public:
        std::unordered_map<dpp::snowflake, Ticket> ticketsGenerating; //Stores tickets being created. Key is client id
    public:
        TicketManager();
        const std::string compileBody(const dpp::snowflake& client_id);
        const std::string compileAttatchments(const dpp::snowflake& client_id);

        //tkt operations
        const bool userHasTktGenerating(const dpp::user& user);
        bool addTicket(const dpp::user& client); //adds entry to ticket gen map
        bool cancelTicket(const dpp::user& client); //Deletes ticket CURRENTLY being created

        //usr facing operations
        bool createTicketChannel(const dpp::user& client, dpp::cluster& bot);
        bool saveResponse(const dpp::message& response, dpp::cluster& bot); //save response in generating ticket depending on gen stage
        bool handleBtnPress(dpp::cluster& bot, const dpp::button_click_t& event); //return true if tktResponse needs to be called
        bool publishTicket(const dpp::channel& channel, dpp::cluster& bot); //Moves ticket channel from Private to Public category
        
};