#include "../headers/Responder.h"

Responder::Responder(dpp::cluster& bot) 
    :
    bot(bot)
{

};

const bool Responder::sendInitResponse(const dpp::user& client) {
     try {
        dpp::message msg("Welcome to the Ink Overflow community marketplace! If you encounter any problems with our service please use this form to report them: https://forms.gle/NL8JgbAS13BXEBJD6. If you would like to cancel your request at any point, type '**/cancel.**' \n Before publishing your commission, we would like to learn a little bit more about your request. Firstly, will you be paying in Rec Room tokens or USD (US Dollar)?"); 
        msg.add_component(
            dpp::component().add_component(
	        dpp::component().set_label("USD").
	        set_type(dpp::cot_button).
	        set_emoji(u8"💸").
	        set_style(dpp::cos_primary).
	        set_id("btn_usd")
            )
        );
        msg.add_component(
            dpp::component().add_component(
	        dpp::component().set_label("Tokens").
	        set_type(dpp::cot_button).
	        set_emoji(u8"🪙").
	        set_style(dpp::cos_primary).
	        set_id("btn_tokens")
            )
        );
        //response
        bot.direct_message_create(client.id, msg);
        return true;
    } catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
};

const void Responder::generateTicketResponse(const dpp::message& msgToRespond, const dpp::user& client, TicketManager& tktManager) {
    //if ticket has not yet been created, respond with initial prompt and initialize ticket
    if(!tktManager.userHasTktGenerating(client)) {
        sendInitResponse(client);
        tktManager.addTicket(client);
        return;
    } 
    Ticket& request = tktManager.ticketsGenerating[client.id];
    if (!request.isBudgetInitialized()) { 
        if (request.isBudgetTokens()) { //token response
            bot.direct_message_create(client.id, dpp::message("Ok! Please type out the amount of tokens you would like to pay. Please note that you must type out the full amount. Ex: Instead of 100K, you must write 100,000."));
        } else { //usd response
            bot.direct_message_create(client.id, dpp::message("Ok! Submit a budget for your request in USD:"));
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
        dpp::message msg( "Ok! Please confirm all of the following information is correct: \n" + tktManager.compileBody(client.id) + "\n" + tktManager.compileAttatchments(client.id));
        //add buttons
        msg.add_component(
            dpp::component().add_component(
	        dpp::component().set_label("Confirm and submit").
	        set_type(dpp::cot_button).
	        set_emoji(u8"✅").
	        set_style(dpp::cos_primary).
	        set_id("btn_submit")
            )
        );
        msg.add_component(
            dpp::component().add_component(
	        dpp::component().set_label("Change information").
	        set_type(dpp::cot_button).
	        set_emoji(u8"🖊️").
	        set_style(dpp::cos_primary).
	        set_id("btn_change_info")
            )
        );
        msg.add_component(
            dpp::component().add_component(
	        dpp::component().set_label("Cancel ticket").
	        set_type(dpp::cot_button).
	        set_emoji(u8"⛔").
	        set_style(dpp::cos_primary).
	        set_id("btn_cancel")
            ) 
        );
        bot.direct_message_create(client.id, msg);

    } else {

    }
};
