#include "../headers/Responder.h"

Responder::Responder(dpp::cluster& bot) 
    :
    bot(bot)
{

};

const bool Responder::sendInitResponse(const dpp::user& client) {
     try {
        dpp::message msg1("Welcome to the Ink Overflow community marketplace! If you ever encounter any problems with our service please use this form to report them: https://forms.gle/NL8JgbAS13BXEBJD6. If you would like to cancel your request at any point, type '**/cancel.**'"); 
        dpp::message msg2("Before publishing your commission, we would like to learn a little bit more about your request. Firstly, will you be paying in Rec Room tokens or USD (US Dollar)?");
        msg2.add_component(
            dpp::component().add_component(
	        dpp::component().set_label("USD").
	        set_type(dpp::cot_button).
	        set_emoji(u8"💸").
	        set_style(dpp::cos_primary).
	        set_id("btn_usd")
            )
        );
        msg2.add_component(
            dpp::component().add_component(
	        dpp::component().set_label("Tokens").
	        set_type(dpp::cot_button).
	        set_emoji(u8"🪙").
	        set_style(dpp::cos_primary).
	        set_id("btn_tokens")
            )
        );
        //response
        bot.direct_message_create(client.id, msg1);
        bot.direct_message_create(client.id, msg2);
        return true;
    } catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
};

const void Responder::generateTicketResponse(const dpp::message& msgToRespond, const dpp::user& client, TicketManager& tktManager) {
    //if ticket has not yet been created, respond with initial prompt and initialize ticket
    if(tktManager.tickets.find(client.id) == tktManager.tickets.end()) {
        sendInitResponse(client);
        tktManager.addTicket(client);
        return;
    } else {
        if (!tktManager.userHasTktGenerating(client)) { //if user has no ticket in progress, initialize new ticket
            sendInitResponse(client);
            tktManager.addTicket(client);
            return;
        } 
    }
    Ticket& request = tktManager.tickets[client.id].front();
    if (!request.isBudgetInitialized()) { 
        if (request.isBudgetTokens()) { //token response
            bot.direct_message_create(client.id, dpp::message("Ok! Next, type out the amount of tokens you would like to pay. Please note that you must type out the full amount. Ex: Instead of 100K, you must write 100,000."));
        } else { //usd response
            bot.direct_message_create(client.id, dpp::message("Ok! Next, submit a budget for your request:"));
        }
    } else if (!request.isDescInitialized()) { // if desc is unitialized
        bot.direct_message_create(client.id, dpp::message("Got it! Now, type out a description of what you would like done. You should include as much detail in your request as possible to help creators understand your request."));
    } else if (!request.isAttachmentsSubmitted()) { //if attatchments are unitialized
        bot.direct_message_create(client.id, dpp::message("Attatch any images you would like to include in your request. This will help creators better understand what you are asking for.").add_component(
	                dpp::component().add_component(
	                dpp::component().set_label("Press to submit your ticket without a photo").
	                set_type(dpp::cot_button).
	                set_emoji(u8"✅").
	                set_style(dpp::cos_primary).
	                set_id("btn_skip_image")
                    )
                ));
    } else if (!request.isNameInitialized()) { //if name is unitialized
        bot.direct_message_create(client.id, dpp::message("Lastly, name your commission request. This name should reflect the content of your request to help it stand out! Ex: 'Roman Empire Enviroment'"));
    } else if (request.isGenerating()) {
        //generate response
        dpp::message msg1(tktManager.compileBody(client.id, 0));
        dpp::message msg2(tktManager.compileAttatchments(client.id, 0));
        //add buttons
        msg2.add_component(
            dpp::component().add_component(
	        dpp::component().set_label("Confirm and submit").
	        set_type(dpp::cot_button).
	        set_emoji(u8"✅").
	        set_style(dpp::cos_primary).
	        set_id("btn_submit")
            )
        );
        msg2.add_component(
            dpp::component().add_component(
	        dpp::component().set_label("Change information").
	        set_type(dpp::cot_button).
	        set_emoji(u8"🖊️").
	        set_style(dpp::cos_primary).
	        set_id("btn_change_info")
            )
        );
        msg2.add_component(
            dpp::component().add_component(
	        dpp::component().set_label("Cancel ticket").
	        set_type(dpp::cot_button).
	        set_emoji(u8"⛔").
	        set_style(dpp::cos_primary).
	        set_id("btn_cancel")
            ) 
        );
        bot.direct_message_create(client.id, dpp::message("Ok! Please confirm all of the following information is correct: "));
        bot.direct_message_create(client.id, msg1);
        bot.direct_message_create(client.id, msg2);
    } else {

    }
};
