#pragma once
#include <string>
#include "dpp/dpp.h"
#include "../headers/TicketManager.h"


class Responder {
private:
    dpp::cluster& bot; //local cluster cache
    const bool sendInitResponse(const dpp::user& client);
public:
    Responder(dpp::cluster& bot);
    const void generateTicketResponse(const dpp::message& msgToRespond, const dpp::user& client, TicketManager& tktManager);
};