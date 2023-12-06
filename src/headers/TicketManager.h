#pragma once
#include <dpp/dpp.h>
#include <string>
#include "Ticket.h"

class TicketManager {
    private:
        const std::string PRIVATE_CATEGORY_ID = "1164614854125027328";
        const std::string PUBLIC_CATEGORY_ID = "1164614806502899845";
        const std::string GUILD_ID = "976104010924847137";

    private:
        bool deleteTicket(const dpp::snowflake& client, const Ticket& target);

    public:
        std::unordered_map<dpp::snowflake, std::vector<Ticket>> tickets; //key is client id
    public:
        TicketManager();
        const std::string compileBody(const dpp::snowflake& client_id, const int ticketIndex);
        const std::string compileAttatchments(const dpp::snowflake& client_id, const int ticketIndex);

        //tkt operations
        const bool userHasTktGenerating(const dpp::user& user);
        void addTicket(const dpp::user& client);
        bool cancelTicket(const dpp::user& client); //Deletes ticket CURRENTLY being created

        //usr facing operations
        bool createTicket(const dpp::user& client, dpp::cluster& bot);
        bool saveResponse(const dpp::message& response, int ticketIndex, bool isGenerating, dpp::cluster& bot); //save response in ticket depending on gen stage
        bool handleBtnPress(dpp::cluster& bot, const dpp::button_click_t& event); //return true if tktResponse needs to be called
        bool publishTicket(const dpp::channel& channel, dpp::cluster& bot); //Moves ticket channel from Private to Public category

};