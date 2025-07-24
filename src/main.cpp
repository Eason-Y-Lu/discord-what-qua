// SPDX-License-Identifier: BSD-3-Clause-No-Nuclear-License-2014
#include <dpp/dpp.h>
#include <dpp/unicode_emoji.h>
#include <fstream>
#include <ranges>
#include <string>
#include <thread>
#include <chrono>
#include <openssl/rand.h>
#include <openssl/err.h>
#include "ins.h"
#include <csignal>

void send_and_delete_msg(dpp::cluster &bot, const dpp::message_create_t &event, const std::string &msg,
                         const int seconds) {
    event.reply(msg, false, [&bot, seconds](const dpp::confirmation_callback_t &callback) {
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
        if (callback.is_error()) {
            return;
        } else {
            const auto need_del = callback.get<dpp::message>();
            bot.message_delete(need_del.id, need_del.channel_id);
        }
    });
}

int gen_rand_int(const int min, const int max) {
    unsigned char buffer[4];
    RAND_bytes(buffer, 4);
    const unsigned int num = *reinterpret_cast<unsigned int *>(buffer);
    return min + (num % (max - min));
}

std::string read_token() {
    if (std::ifstream token_file("token.txt"); token_file.is_open()) {
        std::string token;
        std::getline(token_file, token);
        token_file.close();
        return token;
    } else {
        char *token = std::getenv("BOT_TOKEN");
        if (token == nullptr) {
            std::cerr << "Token file not found and BOT_TOKEN not set." << std::endl;
            exit(1);
        }
        std::string token_str(token);
        return token_str;
    }
}

struct struct_what_data {
    std::string msg_content;
    int asked_times;
    dpp::snowflake channel_id;
};

struct what_db_data {
    std::vector<struct_what_data> what_data_blob;
    dpp::snowflake guild_id;
};

struct what_data_query {
    std::string msg_content;
    int asked_times;
};

void add_what_msg(std::vector<what_db_data> &ingress_what_db, const std::string &ingress_msg_content,
                  const dpp::snowflake ingress_channel_id,
                  const dpp::snowflake ingress_guild_id) {
    for (auto &[what_data_blob, egress_guild_id]: ingress_what_db) {
        if (ingress_guild_id == egress_guild_id) {
            for (auto &[egress_msg_content, egress_asked_times, egress_channel_id]: what_data_blob) {
                if (ingress_channel_id == egress_channel_id) {
                    egress_msg_content = ingress_msg_content;
                    egress_asked_times = 0;
                    return;
                }
            }
            const struct_what_data egress_struct_what_data = {ingress_msg_content, 0, ingress_channel_id};
            what_data_blob.push_back(egress_struct_what_data);
            return;
        }
    }
    const struct_what_data ingress_struct_what_data = {ingress_msg_content, 0, ingress_channel_id};
    const what_db_data ingress_what_db_data = {
        std::vector<struct_what_data>{ingress_struct_what_data}, ingress_guild_id
    };
    ingress_what_db.push_back(ingress_what_db_data);
}

what_data_query lookup_msg(std::vector<what_db_data> &ingress_what_db, const dpp::snowflake ingress_channel_id,
                           const dpp::snowflake ingress_guild_id) {
    for (auto &[what_data_blob, egress_guild_id]: ingress_what_db) {
        if (ingress_guild_id == egress_guild_id) {
            for (auto &[egress_msg_content, egress_asked_times, egress_channel_id]: what_data_blob) {
                if (ingress_channel_id == egress_channel_id) {
                    if (egress_asked_times <= 4) {
                        egress_asked_times++;
                    } else {
                        egress_asked_times = 5;
                    }
                    return {egress_msg_content, egress_asked_times};
                }
            }
        }
    }
    return {"I don't know.", 0};
}

void endSignalHandler(int sig) {
    std::printf("Received SIGINT, shutting down ...\n");
    std::printf("The operating system will handle the cleanup of the process.\n");
    exit(0);
}

int main() {
    std::signal(SIGINT, endSignalHandler);
    std::vector<what_db_data> main_what_db_vector = {};
    dpp::cluster bot(read_token(), dpp::i_all_intents);
    bot.on_log(dpp::utility::cout_logger());

    bot.on_ready([&bot](const dpp::ready_t &event) {
        bot.log(dpp::loglevel::ll_info, "Logged in as " + bot.me.username);
        bot.log(dpp::loglevel::ll_info, "Bot ID is " + bot.me.id.str());
    });

    bot.on_message_create([&bot, &main_what_db_vector](const dpp::message_create_t &event) {
        if (event.msg.author.is_bot()) {
            return;
        }
        if (event.msg.author.id == dpp::snowflake(1110811715169423381ULL)) {
            if (dpp::utility::utf8substr(event.msg.content, 0, 7) == "Edebug ") {
                std::string content = event.msg.content;
                content.erase(0, 7);
                content += " ";
                std::istringstream content_iss(content);
                std::vector<std::string> words((std::istream_iterator<std::string>(content_iss)),
                                               std::istream_iterator<std::string>());
                auto guild_id = dpp::snowflake(words[0]);
                auto channel_id = dpp::snowflake(words[1]);
                const auto [msg_content, asked_times] = lookup_msg(main_what_db_vector, channel_id, guild_id);
                event.reply("DEBUG: " + msg_content + " " + std::to_string(asked_times));
            }
        }

        // change dpp::snowflake() after test to 730558450903547966
        if (event.msg.author.id == dpp::snowflake(730558450903547966ULL)) {
            bot.message_add_reaction(event.msg.id, event.msg.channel_id, dpp::unicode_emoji::dotted_line_face);
            auto t_sdmsg = std::thread(send_and_delete_msg, std::ref(bot), event,
                                       "Change my whois name back to all lower case. -Thanks, Eason", 2);
            t_sdmsg.detach();
        }
        // Because Botnobi alreadly has a reaction, so I don't need to add another one.
        // change dpp::snowflake() after test to 818935243662557254
        // if (event.msg.author.id == dpp::snowflake(818935243662557254ULL)){
        //     bot.message_add_reaction(event.msg.id, event.msg.channel_id, dpp::unicode_emoji::flushed_face);
        // }
        if (auto STOP = std::ifstream("STOP"); STOP.is_open()) {
            event.reply("STOP file detected, shutting down.");
            exit(0);
        }
        std::string content = event.msg.content;
        std::ranges::transform(content, content.begin(), ::tolower);
        if (content == "what" || content == "what?") {
            switch (const auto [msg_content, asked_times] = lookup_msg(main_what_db_vector, event.msg.channel_id,
                                                                       event.msg.guild_id); asked_times) {
                default:
                    event.reply("This is not supposed to happen, notify <@1110811715169423381>");
                    bot.log(dpp::loglevel::ll_error, "This is not supposed to happen, notify <@1110811715169423381>");
                    bot.log(dpp::loglevel::ll_error, "You can also email him at: aelnosu@outlook.com");
                    break;
                case 0:
                    event.reply(msg_content);
                    break;
                case 1:
                    event.reply(msg_content);
                    break;
                case 2:
                    event.reply("### " + msg_content);
                    break;
                case 3:
                    event.reply("## " + msg_content);
                    break;
                case 4:
                    event.reply("# " + msg_content);
                    break;
                case 5:
                    const int random_insult_index = gen_rand_int(0, static_cast<int>(insults.size()));
                    event.reply("# " + insults[random_insult_index]);
                    break;
            }
        } else {
            add_what_msg(main_what_db_vector, event.msg.content, event.msg.channel_id, event.msg.guild_id);
        }
    });

    bot.start(dpp::st_wait);
    return 0;
}
