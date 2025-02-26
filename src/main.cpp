// SPDX-License-Identifier: BSD-3-Clause-No-Nuclear-License-2014
#include <dpp/dpp.h>
#include <dpp/unicode_emoji.h>
#include <fstream>
#include <ranges>
#include <string>
#include "ins.h"
#include <openssl/rand.h>
#include <openssl/err.h>

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
        throw std::runtime_error("Failed to open token.txt, make sure it exists and is readable.");
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

int main() {
    std::vector<what_db_data> main_what_db_vector = {};
    dpp::cluster bot(read_token(), dpp::i_all_intents);
    bot.on_log(dpp::utility::cout_logger());

    bot.on_ready([&bot](const dpp::ready_t &event) {
        bot.log(dpp::loglevel::ll_info, "Logged in as " + bot.me.username);
        bot.log(dpp::loglevel::ll_info, "Bot ID is " + bot.me.id.str());
    });

    bot.on_message_create([&bot, &main_what_db_vector](const dpp::message_create_t &event) {
        auto *yolocowow_ptr = dpp::find_user(dpp::snowflake(1110811715169423381));
        // remove this block after testing
        if (yolocowow_ptr != nullptr) {
            if (event.msg.author.id == yolocowow_ptr->id) {
                bot.message_add_reaction(event.msg.channel_id, event.msg.id, dpp::unicode_emoji::flushed);
            }
        }
        // end of block
        if (event.msg.author.is_bot()) {
            return;
        }
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
