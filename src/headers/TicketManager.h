#pragma once
#include <dpp/dpp.h>
#include <string>
#include "Ticket.h"

class TicketManager {
    private:
        std::string PRIVATE_CHANNEL_ID = "1164615636337557594";
        std::string PUBLIC_CHANNEL_ID = "1164615615147954277";
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
        const bool userHasTktEditing(const dpp::user& user);
        const bool listTickets(const dpp::user& client, dpp::cluster& bot);
        void addTicket(const dpp::user& client);
        bool cancelTicket(const dpp::user& client); //Deletes ticket CURRENTLY being created

        //client facing operations
        bool createTicketThread(const dpp::user& client, dpp::cluster& bot);
        bool saveResponse(const dpp::message& response, int ticketIndex, dpp::cluster& bot); //save response in ticket depending on gen stage
        bool handleBtnPress(dpp::cluster& bot, const dpp::button_click_t& event); //return true if tktResponse needs to be called
        const bool reviewTicket(const dpp::user&client, int ticketIndex, dpp::cluster& bot); //opens review/edit menu

};