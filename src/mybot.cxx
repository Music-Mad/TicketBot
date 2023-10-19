#include <dpp/dpp.h>
#include <string>
#include <future>
#include <variant> // for std::get
//for currency formatting
#include <iomanip>
#include <locale>
#include <sstream>

//to use bot in sever, copy Admin ID and Creator ID below
const std::string ADMIN_ID = "1138102566476058735";
const std::string CREATOR_ID = "1139260164906700920"; 
const std::string BOT_ID = "975895127224635432";
const std::string PRIVATE_FORUM_ID = "1164615636337557594";
const std::string PUBLIC_FORUM_ID = "1164615615147954277";
const std::string GUILD_ID = "976104010924847137";


//store bot token
const std::string    BOT_TOKEN = "OTc1ODk1MTI3MjI0NjM1NDMy.GU41IO.mSyyBcIcgkZ1Wkoyhv8XPDfHUWs35y8aRxzU7E";

//create strucutre to format requests
struct request {
    int stage;
    std::string budget;
    bool isTokens;
    std::string description;
    std::vector<dpp::attachment> images;
};

//Map for tracking tickets being created in DMs
std::unordered_map<dpp::snowflake, request> ticketsProcessing; 
//call back for various functions
void myCallback(dpp::confirmation_callback_t callback)
{
    dpp::utility::log_error();
};

//convert token amount to USD
float toUSD (int tokens)
{
    float wrkVal = tokens * .7f;
    float USD = wrkVal/2500.0f;
    return USD;
}

//cancels a ticket in creation process
bool cancelTicket(dpp::snowflake key, dpp::cluster& bot)
{
    try
    {
        if (ticketsProcessing.find(key) == ticketsProcessing.end())
        {
            return false;
        }
        else
        {
            ticketsProcessing.erase(key);
            bot.direct_message_create(key, dpp::message("Your ticket has been successfully deleted. Please use /request in the Ink Overflow discord if you ever would like to create a new request."));
            return true;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
};














//process a string value and saves it to appropriate ticketsProcessing
std::string formatPrice(std::string str)
{
    std::string processedPrice;
    std::string processedPrice2;
    bool dashFound = false;
    int dashIndex;

    for (char c: str)
    {
        //ignore space entries
        if (!(c == ' ') && !(c == ','))
        {
            //save digits
            if (isdigit(c))
            {
                if (!dashFound) {
                    processedPrice += c;
                }
                else {
                    processedPrice2 += c;
                }
            }
            else if (c == '-')
            {
                if (!dashFound) {
                    dashFound = true;
                    dashIndex = processedPrice.length() - 1;
                }
            }
            else 
            {
                return "";
            }

        }
    }
    

    std::string priceCommaed = processedPrice;

    for (int i = 1; i < processedPrice.length(); ++i)
    {
        int backI = processedPrice.length() - i;
        if ((i % 3) == 0) {
            if (processedPrice[i] == ' ' || processedPrice[i] == '-')
            {

            }
            else
            {
                priceCommaed.insert(backI, ",");
            }
        }

    }

    if (dashFound)
    {
        std::string priceCommaed2 = processedPrice2;

        for (int i = 1; i < processedPrice2.length(); ++i)
        {
            int backI = processedPrice2.length() - i;
            if ((i % 3) == 0) {
                if (processedPrice2[i] == ' ' || processedPrice2[i] == '-')
                {

                }
                else
                {
                    priceCommaed2.insert(backI, ",");
                }
            }
        }

        return priceCommaed + " - " + priceCommaed2;
    }

    return priceCommaed;
};

//for converting strings to integers
int stringToInt(std::string str) {
    return std::stoi(str);
};

//function used for compiling a string of data from TicketsProcessing
std::string compileTicket(const dpp::user& client)
{
    std::string currency;
    if (ticketsProcessing[client.id].isTokens)
    {
    }
    else
    {
        currency = "USD";
    }

    
    //compile ticket and image strings
    std::string ticket = R"(
User - )" + client.get_mention() + R"(
Budget - )" +  ticketsProcessing[client.id].budget + " " + currency + R"(
Description - )" + ticketsProcessing[client.id].description;

    return ticket;
};

//typically called with compileTicket. Compiles a string the links to images
std::string compileAttatchments(const dpp::user& client)
{
    std::string imgs;

    for (dpp::attachment att : ticketsProcessing[client.id].images)
        {
            imgs += " " + att.url;
        }
    
    return imgs;
};

//create ticket after data has been compilied in DMs. Return true upon sucsess
void createTicket(const dpp::user& client, dpp::cluster& bot, bool hasAttatchments)
{
    std::string ticket = compileTicket(client);

    std::string imgs;

    if (hasAttatchments)
    {
        imgs = compileAttatchments(client);
    }

    dpp::message tktMsg(ticket);

    //create ticket thread
    bot.thread_create_in_forum(client.username + "'s request", PRIVATE_FORUM_ID, tktMsg, dpp::auto_archive_duration_t::arc_1_week, 60, std::vector<dpp::snowflake>(), [&bot, ticket, &client, imgs] (auto callback){
                                
        if (!callback.is_error())
        {   
            //retrive thread
            dpp::thread& myThread = std::get<dpp::thread>(callback.value);

            //send img in thread
            if (!(imgs == ""))
            {
                dpp::message img(myThread.id, imgs);
                bot.message_create(img);
            }
        }
        else
        {
            dpp::utility::log_error();

        }
    }); 
};


























//sends appropriate msgs to user depending on stage index. Returns true upon msg sucsess
bool stageTicket(dpp::cluster& bot, int stage, const dpp::user& targetUsr, const dpp::message msg, bool msgHasAttatchments)
{
    switch (stage)
        {
            //currency type submitted
            case 0:
            if (ticketsProcessing[targetUsr.id].isTokens) {
                bot.direct_message_create(targetUsr.id, dpp::message("Got it! Next, type out the amount of tokens you would like to pay. Please note that you must type out the full amount. Ex: Instead of 100K, you must write 100,000."));
            } else {
                bot .direct_message_create(targetUsr.id, dpp::message("Got it! Next, submit a budget for your request:"));
            }
                ticketsProcessing[targetUsr.id].stage++;
                return true;
            //currency amount submitted
            case 1:
                ticketsProcessing[targetUsr.id].budget = msg.content;
            //currency amount is tokens
                if (ticketsProcessing[targetUsr.id].isTokens)
                {   
                    //if input is valid
                    if (formatPrice(msg.content) != "")
                    {
                    //initialize useful variables
                    std::string input = ticketsProcessing[targetUsr.id].budget;
                    std::string formattedInput = formatPrice(input);
                    ticketsProcessing[targetUsr.id].budget = formattedInput;
                    //convert token amount to USD. 
                    try
                    {
                        //remove non numerical entries from input and split values if applicable
                        bool dashFound = false;
                        std::string secondVal;
                        input.erase(std::remove(input.begin(), input.end(), ','), input.end());
                        input.erase(std::remove(input.begin(), input.end(), ' '), input.end());

                        for (char c : input)
                        {
                            if (dashFound)
                            {
                                secondVal += c;
                            }
                            if (!std::isdigit(c))
                            {   
                                if (c == '-') {
                                    dashFound = true;
                                }
                            }
                        }




                        if (std::stoi(input) >= 10000)
                        {
                            float amount1;
                            float amount2;
                            std::string format1;
                            std::string format2;
                            //convert input into a USD amount
                            amount1 = toUSD(std::stoi(input));
                            //create a string stream to hold the formatted string
                            std::stringstream ss;
                            //set locale
                            ss.imbue(std::locale::classic());
                            //format string
                            ss << std::fixed << std::setprecision(2);
                            ss << "$" << amount1;
                            //save formatted string
                            format1 = ss.str();

                            //repeat for second val
                            if (dashFound)
                            {
                                amount2 = toUSD(std::stoi(secondVal));
                                std::stringstream ss2;
                                ss2.imbue(std::locale::classic());
                                ss2 << std::fixed << std::setprecision(2);
                                ss2 << "$" << amount2;
                                format2 = ss2.str();
                            }
                 
                        
                            //compile token discolusure msg
                            std::string msg1 = "Because you have selected tokens as your payment method, we would like to remind you of our token policy.";
                            std::string msg2 = "**Commissions must be a minimum of 10,000 tokens**. For reference: *1,250 Rec Room tokens = 1 US dollar*.";
                            std::string msg3 = "Tokens are *not* a prefered payment method as they are confusing and inefficient. Additionally, commissions made in USD are more likely to be accepted by our creators.";
                            dpp::message msg4("Your token payment of " + ticketsProcessing[targetUsr.id].budget + " will be worth: " + format1 + " USD. Is this okay?");
                            dpp::message msg4_alt("Your token payment of " + ticketsProcessing[targetUsr.id].budget + " will be worth: " + format1 + " - " + format2 + " USD. Are you sure you would like to continue?");
                            //add buttons to msg4
                            msg4.add_component(    
                                dpp::component().add_component(
	                            dpp::component().set_label("Yes").
	                            set_type(dpp::cot_button).
	                            set_emoji(u8"✅").
	                            set_style(dpp::cos_primary).
	                            set_id("btn_accept_payment")
                                )   
                            );  
                            msg4.add_component(
                                dpp::component().add_component(
	                            dpp::component().set_label("No. I would like to change my payment information").
	                            set_type(dpp::cot_button).
	                            set_emoji(u8"⛔").
	                            set_style(dpp::cos_primary).
	                            set_id("btn_decline_payment")
                                )
                            );
                            msg4_alt.add_component(    
                                dpp::component().add_component(
	                            dpp::component().set_label("Yes").
	                            set_type(dpp::cot_button).
	                            set_emoji(u8"✅").
	                            set_style(dpp::cos_primary).
	                            set_id("btn_accept_payment")
                                )   
                            );  
                            msg4_alt.add_component(
                                dpp::component().add_component(
	                            dpp::component().set_label("No. I would like to change my payment information").
	                            set_type(dpp::cot_button).
	                            set_emoji(u8"⛔").
	                            set_style(dpp::cos_primary).
	                            set_id("btn_decline_payment")
                                )
                            );
                            bot.direct_message_create(targetUsr.id, msg1);
                            bot.direct_message_create(targetUsr.id, msg2);
                            bot.direct_message_create(targetUsr.id, msg3);
                            if (dashFound)
                            {
                                bot.direct_message_create(targetUsr.id, msg4_alt);
                                ticketsProcessing[targetUsr.id].budget += " tokens (" + format1 + "-" + format2 + ") ";
                            }
                            else
                            {
                                bot.direct_message_create(targetUsr.id, msg4);
                                ticketsProcessing[targetUsr.id].budget += " tokens (" + format1 + ") ";
                            }
                       
                            ticketsProcessing[targetUsr.id].stage++;
                            return true;
                        }
                        else
                        {
                            bot.direct_message_create(targetUsr.id, dpp::message("The minimum amount for token requests is 10,000. Please try submitting a higher amount:"));
                            return false;
                        }
                    } catch (const std::invalid_argument& e)
                    {
                        bot.direct_message_create(targetUsr.id, dpp::message("That is not a valid amount, please try submitting again"));
                        return false;
                    } catch (const std::out_of_range& e)
                    {
                        bot.direct_message_create(targetUsr.id, dpp::message("This value is out of range. Please try submitting something less extreme:"));
                        return false;
                    } catch(const std::exception& e)
                    {
                        bot.direct_message_create(targetUsr.id, dpp::message("Error"));
                        return false;
                    }
                    }
                    else
                    {
                        bot.direct_message_create(targetUsr.id, dpp::message("That is not a valid token amount! Try submitting another value. As a reminder, this value should not contain any letters, special characters, or decimals."));
                        return false;
                    }
                }
                //currency amount is USD
                else
                {
                    for (char c : msg.content)
                    {
                        if (!std::isdigit(c))
                        {
                            bot.direct_message_create(targetUsr.id, dpp::message("Please submit a value that contains only numbers"));
                            return false;
                        }
                    }
                    ticketsProcessing[targetUsr.id].stage++;
                }
            //currency amount confirmed/submitted
            case 2:
                bot.direct_message_create(targetUsr.id, dpp::message("Now, type out a description of what you would like done. You should include as much detail in your request as possible to help creators understand your request."), myCallback);
                ticketsProcessing[targetUsr.id].stage++;
                return true;
            //description submitted
            case 3:
                ticketsProcessing[targetUsr.id].description = msg.content;       
                bot.direct_message_create(targetUsr.id, dpp::message("Lastly, attatch any images you would like to include in your request. This will help us better understand what you are asking for.").add_component(
	                dpp::component().add_component(
	                dpp::component().set_label("Press to submit your ticket without a photo").
	                set_type(dpp::cot_button).
	                set_emoji(u8"✅").
	                set_style(dpp::cos_primary).
	                set_id("btn_skip_image")
                    )
                ));
                ticketsProcessing[targetUsr.id].stage++;
                return true;
            case 4: 
            {  
                if (msgHasAttatchments)
                {
                    //save images and compile url string for ticket
                    for (dpp::attachment att : msg.attachments)
                    {
                        ticketsProcessing[targetUsr.id].images.push_back(att);
                    }
                }
                
                std::string tkt = compileTicket(targetUsr);
                dpp::message imgs(compileAttatchments(targetUsr));
                imgs.add_component(
                    dpp::component().add_component(
	                dpp::component().set_label("Confirm and submit").
	                set_type(dpp::cot_button).
	                set_emoji(u8"✅").
	                set_style(dpp::cos_primary).
	                set_id("btn_submit")
                    )
                );
                imgs.add_component(
                    dpp::component().add_component(
	                dpp::component().set_label("Change information").
	                set_type(dpp::cot_button).
	                set_emoji(u8"🖊️").
	                set_style(dpp::cos_primary).
	                set_id("btn_change_info")
                    )
                );
                imgs.add_component(
                    dpp::component().add_component(
	                dpp::component().set_label("Cancel ticket").
	                set_type(dpp::cot_button).
	                set_emoji(u8"⛔").
	                set_style(dpp::cos_primary).
	                set_id("btn_cancel")
                    ) 
                );
                bot.direct_message_create(targetUsr.id, dpp::message("Please confirm all of the following information is correct: "));
                bot.direct_message_create(targetUsr.id, dpp::message(tkt));
                bot.direct_message_create(targetUsr.id, imgs);
                ticketsProcessing[targetUsr.id].stage++;
                return true;
            }
            case 5:
                //create ticket and break
                createTicket(targetUsr, bot, !(ticketsProcessing[targetUsr.id].images.empty()));
                dpp::message confirmationMsg("Thank you for your request! Your response has been saved and sent to our verified creators for review. You will recieve a DM from a creator if they would like to fulfill your commission. After one week of inactivity, your request will automatically be closed. Please reach out to an Ink Overflow admin if you have any questions");
                bot.direct_message_create(targetUsr.id, confirmationMsg);
                ticketsProcessing.erase(targetUsr.id);
                return true;

            defualt:
                return false;
        }

        return false;
}



































int main()
{
    /* Create bot cluster */
    dpp::cluster bot(BOT_TOKEN, dpp::i_message_content | dpp::i_direct_messages | dpp::i_guild_message_reactions);

    /* Output simple log messages to stdout */
    bot.on_log(dpp::utility::cout_logger());


    /* Handle request command */
    bot.on_slashcommand([&bot](const dpp::slashcommand_t& event) {
        if (event.command.get_command_name() == "request") {

                //check if user already has an open ticket to avoid storing two tickets under the same key
                if (ticketsProcessing.find(event.command.usr.id) != ticketsProcessing.end())
                {   
                    //error reply
                    dpp::message threadReply("You already have a request in processing, please check your DM's!");
                    event.reply(threadReply.set_flags(dpp::m_ephemeral));
                }
                else
                {
                    //store new ticket
                    ticketsProcessing[event.command.usr.id] = request{0};
                    //notify user
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
                    bot.direct_message_create(event.command.usr.id, msg1);
                    bot.direct_message_create(event.command.usr.id, msg2);
                    //reply to command
                    dpp::message threadReply("Please check your DM's!");
                    event.reply(threadReply.set_flags(dpp::m_ephemeral));
                }
            }
        else if (event.command.get_command_name() == "cancel")
        {
            if (cancelTicket(event.command.usr.id, bot))
            {
                dpp::message reply("Your ticket has been cancelled. Check your DM's!");
                event.reply(reply.set_flags(dpp::m_ephemeral));
            }
            else
            {
                dpp::message reply("You have no ticket actively being created. If you would like to create a new request, use /request.");
                event.reply(reply.set_flags(dpp::m_ephemeral));
            }
   
        }
        });































    //respond to DMs and compile tickets
    bot.on_message_create([&bot](const dpp::message_create_t& event){

        //cache author ID
        dpp::snowflake authorId = event.msg.author.id;
        //if msg is DM, not from bot, AND user has an open ticket, stage ticket
        if (event.msg.is_dm())
        {
            if (authorId == dpp::snowflake(BOT_ID))
            {
                
            }
            else
            {
                //if message is not cancel cmd
                if (event.msg.content != "/cancel")
                {
                    if (ticketsProcessing.find(authorId) == ticketsProcessing.end())
                    {
                   
                    }
                    else
                    {
                        //msgs are not allowed at stage 0 or 2. Bot is expecting button response
                        if (ticketsProcessing[authorId].stage == 0)
                        {
                            event.reply("Please select either button: 'USD' or 'Tokens'");
                        }
                        else if (ticketsProcessing[authorId].stage == 2)
                        {
                            event.reply("Please press either the yes or no buttons above");
                        }
                        else if (ticketsProcessing[authorId].stage == 4)
                        {
                            if (event.msg.attachments.empty())
                            {
                                bot.direct_message_create(authorId, dpp::message("Please submit a message with images attatched OR press the skip button above to submit your ticket with no images"));
                            }
                            else
                            {
                                stageTicket(bot, ticketsProcessing[authorId].stage, event.msg.author, event.msg, !event.msg.attachments.empty());
                            }
                        }
                        else if (ticketsProcessing[authorId].stage == 5)
                        {
                            event.reply("Please select one of the buttons above");
                        }
                        else
                        {
                           stageTicket(bot, ticketsProcessing[authorId].stage, event.msg.author, event.msg, !event.msg.attachments.empty());
                        }
                    }
                }
                else
                {
                    cancelTicket(authorId, bot);
                }
                
            }   
        }
    });






























    //manage buttons
    bot.on_button_click([&bot](const dpp::button_click_t & event) {
    //cache user id
    dpp::snowflake authorId = event.command.usr.id;
    //handle skip image button press
    if(ticketsProcessing.find(authorId) != ticketsProcessing.end())
    {
        if (event.custom_id == "btn_skip_image")
        {  
            if (ticketsProcessing[authorId].stage == 4)
            {
                stageTicket(bot, ticketsProcessing[authorId].stage, event.command.usr, dpp::message(""), false);
                event.reply();
            }
            else
            {
                event.reply("This action cannot be processed right now");
            }
        }
        else if (ticketsProcessing[authorId].stage == 5)
        {
            if  (event.custom_id == "btn_submit")
            {
                stageTicket(bot, 5, event.command.usr, dpp::message(""), false);
                event.reply();
            }
            else if (event.custom_id == "btn_change_info")
            {
                ticketsProcessing[authorId].stage = 0;
                ticketsProcessing[authorId].images = std::vector<dpp::attachment>();
                dpp::message msg("Ok! Firstly, would you like to pay with Rec Room tokens or USD (U.S. Dollar)");
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
                bot.direct_message_create(event.command.usr.id, msg);
                event.reply();
            }
            else if (event.custom_id == "btn_cancel")
            {

                cancelTicket(authorId, bot);
                event.reply();
            }
        }
        else if (ticketsProcessing[authorId].stage == 2)
        {
            if (event.custom_id == "btn_accept_payment")
            {
                bot.direct_message_create(authorId, dpp::message("Confirmed!"));
                stageTicket(bot, ticketsProcessing[authorId].stage, event.command.usr, dpp::message(""), false);
                event.reply();
            }
            else if (event.custom_id == "btn_decline_payment")
            {   
                dpp::message msg("Ok! Firstly, would you like to pay in Rec Room tokens or USD (United States Dollar)");
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
                bot.direct_message_create(authorId, msg);
                ticketsProcessing[authorId].stage = 0;
                event.reply();
            }
        }
        else if (ticketsProcessing[authorId].stage == 0)
            {
                if (event.custom_id == "btn_usd")
                {
                    ticketsProcessing[authorId].isTokens = false;
                    stageTicket(bot, 0, event.command.usr, dpp::message(""), false);
                    event.reply();
                }
                else if (event.custom_id == "btn_tokens")
                {
                    ticketsProcessing[event.command.usr.id].isTokens = true;
                    stageTicket(bot, 0, event.command.usr, dpp::message(""), false);
                    event.reply();
                }
            }
        }
        else
        {
            event.reply("This ticket has already been closed. Please use /request in the Ink Overflow server to submit a new one.");
        }
    });

    /* handle open and close commission commands*/
    bot.on_slashcommand([&bot](const dpp::slashcommand_t& event){
        if (event.command.get_command_name() == "open_ticket")
        {
            //get thread from command
            bot.thread_get(event.command.channel_id, [&bot, event](const dpp::confirmation_callback_t& callback) {
                if(callback.is_error()) {
                    bot.log(dpp::ll_warning, "Failed to get thread.");
                    return;
                }

                //move thread to public forum
                callback.get<dpp::thread>().set_parent_id(PUBLIC_FORUM_ID);
            });

            //generate reply
            dpp::message reply("Commission published!");
            event.reply(reply.set_flags(dpp::m_ephemeral));
        
        }
        else if (event.command.get_command_name() == "close_ticket") 
        {
            dpp::snowflake id_channel = event.command.channel_id;
            bot.channel_delete(id_channel);
        }
    });


    bot.on_slashcommand([&bot](const dpp::slashcommand_t& event){
        if (event.command.get_command_name() == "lock_thread") 
        {   
            bot.thread_get(event.command.channel_id, [&bot, event](const dpp::confirmation_callback_t& callback) {
                if(callback.is_error()) {
                    bot.log(dpp::ll_warning, "Failed to get thread.");
                    return;
                }

                auto thread = callback.get<dpp::thread>();

                thread.metadata.locked = true;
                
                bot.thread_edit(thread, [event](const dpp::confirmation_callback_t& callback2) {
                    if(callback2.is_error()) {
                        event.reply("Failed to lock the thread.");
                        return;
                    }

                    event.reply("Locked the thread.");
                });
            });
        }
    });








    /* Register slash command here in on_ready */
    bot.on_ready([&bot](const dpp::ready_t& event) {
        /* Wrap command registration in run_once to make sure it doesnt run on every full reconnection */
        if (dpp::run_once<struct register_bot_commands>()) {

            dpp::slashcommand requestCommand("request", "Create a commission request", bot.me.id);
            dpp::slashcommand openCommission("open_ticket", "Open a commission for all verified creators", bot.me.id);
            dpp::slashcommand cancelRequest("cancel", "cancel a ticket in-progress", bot.me.id);
            dpp::slashcommand closeTicket("close_ticket", "Close a ticket thread", bot.me.id);

            bot.guild_command_create(requestCommand, GUILD_ID);
            bot.guild_command_create(openCommission, GUILD_ID);
            bot.guild_command_create(cancelRequest, GUILD_ID);
            bot.guild_command_create(closeTicket, GUILD_ID);
        }
    });

    /* Start the bot */
    bot.start(dpp::st_wait);

    return 0;
};
