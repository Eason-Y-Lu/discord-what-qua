// SPDX-License-Identifier: BSD-3-Clause-No-Nuclear-License-2014
#include "ins.h"
#include <csignal>
#include <dpp/dpp.h>
#include <fstream>
#include <oneapi/tbb.h>
#include <oneapi/tbb/concurrent_vector.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <string>
#include <vector>
#include <map>

int gen_rand_int(const int min, const int max) {
  unsigned char buffer[4];
  RAND_bytes(buffer, 4);
  const unsigned int num = *reinterpret_cast<unsigned int *>(buffer);
  return min + (num % (max - min));
}

std::string read_token() {
  char *token = std::getenv("BOT_TOKEN");
  if (token == nullptr || std::string(token).empty()) {
    if (std::ifstream token_file("token.txt"); token_file.is_open()) {
      std::string token;
      std::getline(token_file, token);
      token_file.close();
      if (token.empty()) {
        std::cerr << "Token file is empty." << std::endl;
        exit(1);
      } else {
        return token;
      }
    } else {
      std::cerr << "Token file not found and BOT_TOKEN not set." << std::endl;
      exit(1);
    }
  } else {
    std::string token_str(token);
    return token_str;
  }
}

struct what_message_blob {
  dpp::snowflake author_id;
  int asked_times;
  std::string message;
};

struct what_channel_blob {
  dpp::snowflake channel_id;
  what_message_blob message_1;
  what_message_blob message_2;
};

struct what_server_blob {
  dpp::snowflake server_id;
  tbb::concurrent_vector<what_channel_blob> channel_data;
};

void add_message_to_db(
    const dpp::snowflake guild_id, const dpp::snowflake channel_id,
    const dpp::snowflake author_id, const std::string &message,
    tbb::concurrent_vector<what_server_blob> &main_what_db_vector) {
  for (auto &[server_id, channel_data] : main_what_db_vector) {
    if (server_id == guild_id) {
      for (auto &[channel_id_in_db, message_1, message_2] : channel_data) {
        if (channel_id_in_db == channel_id) {
          // if author is already in either message_1 or message_2, update the
          // message if author is not in either message_1 or message_2,
          if (message_1.author_id == author_id) {
            message_1.message = message;
            message_1.asked_times = 0;
            return;
          } else if (message_2.author_id == author_id) {
            message_2.message = message;
            message_2.asked_times = 0;
            return;
          } else {
            message_2 = message_1;
            message_1.author_id = author_id;
            message_1.message = message;
            message_1.asked_times = 0;
            return;
          }
        }
      }
      // if channel not found, add a new channel
      what_channel_blob new_channel;
      new_channel.channel_id = channel_id;
      new_channel.message_1.author_id = author_id;
      new_channel.message_1.message = message;
      new_channel.message_1.asked_times = 0;
      new_channel.message_2.author_id = 0; // no second message
      new_channel.message_2.message = "";
      new_channel.message_2.asked_times = 0;
      channel_data.push_back(new_channel);
      return;
    }
  }
  // if server not found, add a new server
  what_server_blob new_server;
  new_server.server_id = guild_id;
  what_channel_blob new_channel;
  new_channel.channel_id = channel_id;
  new_channel.message_1.author_id = author_id;
  new_channel.message_1.message = message;
  new_channel.message_1.asked_times = 0;
  new_channel.message_2.author_id = 0; // no second message
  new_channel.message_2.message = "";
  new_channel.message_2.asked_times = 0;
  new_server.channel_data.push_back(new_channel);
  main_what_db_vector.push_back(new_server);
}

struct lookup_result {
  std::string message_content;
  int asked_times;
};

lookup_result
lookup_msg(tbb::concurrent_vector<what_server_blob> &main_what_db_vector,
           const dpp::snowflake channel_id, const dpp::snowflake guild_id,
           const dpp::snowflake author_id) {
  for (auto &server_blob : main_what_db_vector) {
    if (server_blob.server_id == guild_id) {
      for (auto &channel_blob : server_blob.channel_data) {
        if (channel_blob.channel_id == channel_id) {
          if (channel_blob.message_1.author_id == author_id &&
              channel_blob.message_2.author_id != 0) {
            if (channel_blob.message_2.asked_times < 5) {
              channel_blob.message_2.asked_times++;
            }
            return {channel_blob.message_2.message,
                    channel_blob.message_2.asked_times};
          } else if (channel_blob.message_2.author_id == author_id &&
                     channel_blob.message_1.author_id != 0) {
            if (channel_blob.message_1.asked_times < 5) {
              channel_blob.message_1.asked_times++;
            }
            return {channel_blob.message_1.message,
                    channel_blob.message_1.asked_times};
          } else if (channel_blob.message_1.author_id == author_id &&
                     channel_blob.message_2.author_id == 0) {
            return {"I don't know.", 0};
          } else if (channel_blob.message_2.author_id == author_id &&
                     channel_blob.message_1.author_id == 0) {
            return {"I don't know.", 0};
          } else {
            if (channel_blob.message_1.asked_times < 5) {
              channel_blob.message_1.asked_times++;
            }
            return {channel_blob.message_1.message,
                    channel_blob.message_1.asked_times};
          }
        }
      }
    }
  }
  return {"I don't know.", 0};
}

// Wellness resources data
struct wellness_resource {
  std::string name;
  std::string url;
  std::string description;
};

struct wellness_phone {
  std::string name;
  std::string number;
  std::string description;
};

// Wellness resources
const inline std::vector<wellness_resource> wellness_links = {
  {"National Suicide Prevention Lifeline", "https://988lifeline.org/", "24/7 crisis support and suicide prevention"},
  {"Crisis Text Line", "https://www.crisistextline.org/", "Free, 24/7 crisis support via text"},
  {"Mental Health America", "https://www.mhanational.org/", "Mental health resources and screening tools"},
  {"NAMI (National Alliance on Mental Illness)", "https://www.nami.org/", "Support, education and advocacy for mental health"},
  {"Substance Abuse and Mental Health Services Administration", "https://www.samhsa.gov/", "Treatment locator and mental health resources"},
  {"Anxiety and Depression Association of America", "https://adaa.org/", "Resources for anxiety and depression support"}
};

const inline std::vector<wellness_phone> wellness_phones = {
  {"National Suicide Prevention Lifeline", "988", "24/7 crisis support and suicide prevention"},
  {"Crisis Text Line", "Text HOME to 741741", "Free, 24/7 crisis support via text message"},
  {"SAMHSA National Helpline", "1-800-662-4357", "24/7 treatment referral service"},
  {"National Domestic Violence Hotline", "1-800-799-7233", "24/7 confidential support for domestic violence"},
  {"Trans Lifeline", "877-565-8860", "24/7 support for transgender individuals"},
  {"The Trevor Project", "1-866-488-7386", "24/7 crisis support for LGBTQ+ youth"}
};

void endSignalHandler(int sig) {
  std::printf("Received SIGINT, shutting down ...\n");
  std::printf("The operating system will handle the cleanup of the process.\n");
  exit(0);
}

int main() {
  auto *main_what_db_vector_ptr =
      new tbb::concurrent_vector<what_server_blob>();
  tbb::concurrent_vector<what_server_blob> &main_what_db_vector =
      *main_what_db_vector_ptr;
  std::signal(SIGINT, endSignalHandler);
  dpp::cluster bot(read_token(),
                   dpp::i_guild_messages | dpp::i_message_content);
  bot.on_log(dpp::utility::cout_logger());
  
  bot.on_ready([&bot](const dpp::ready_t &event) {
    bot.log(dpp::loglevel::ll_info, "Logged in as " + bot.me.username);
    bot.log(dpp::loglevel::ll_info, "Bot ID is " + bot.me.id.str());
    
    // Create wellness slash command
    if (dpp::run_once<struct register_bot_commands>()) {
      dpp::slashcommand wellness_command("wellness", "Access wellness resources and support", bot.me.id);
      
      // Add sub-commands for links and phone numbers
      wellness_command.add_option(
        dpp::command_option(dpp::co_sub_command, "links", "View helpful wellness and mental health links")
      );
      wellness_command.add_option(
        dpp::command_option(dpp::co_sub_command, "phone-numbers", "View important wellness and crisis phone numbers")
      );
      
      bot.global_command_create(wellness_command);
      bot.log(dpp::loglevel::ll_info, "Wellness command registered");
    }
  });
  
  // Handle slash command interactions
  bot.on_slashcommand([&bot](const dpp::slashcommand_t &event) {
    if (event.command.get_command_name() == "wellness") {
      std::string subcommand = event.command.get_subcommand_name();
      
      if (subcommand == "links") {
        // Create embed for wellness links
        dpp::embed embed = dpp::embed()
          .set_color(0x00ff00)
          .set_title("🌱 Wellness Resources - Helpful Links")
          .set_description("Here are some valuable mental health and wellness resources:");
        
        for (const auto& link : wellness_links) {
          embed.add_field(link.name, "[Visit Website](" + link.url + ")\n" + link.description, false);
        }
        
        embed.set_footer(dpp::embed_footer().set_text("Remember: You are not alone, and help is available."));
        
        dpp::message msg(event.command.channel_id, embed);
        event.reply(msg);
        
      } else if (subcommand == "phone-numbers") {
        // Create embed for phone numbers
        dpp::embed embed = dpp::embed()
          .set_color(0x0099ff)
          .set_title("📞 Wellness Resources - Phone Numbers")
          .set_description("Important phone numbers for crisis support and mental health:");
        
        for (const auto& phone : wellness_phones) {
          embed.add_field(phone.name, "**" + phone.number + "**\n" + phone.description, false);
        }
        
        embed.set_footer(dpp::embed_footer().set_text("These services are confidential and available 24/7."));
        
        dpp::message msg(event.command.channel_id, embed);
        event.reply(msg);
      }
    }
  });
  bot.on_message_create([&bot, &main_what_db_vector](
                            const dpp::message_create_t &event) {
    if (event.msg.author.is_bot()) {
      return;
    }
    if (auto STOP = std::ifstream("STOP"); STOP.is_open()) {
      event.reply("STOP file detected, shutting down.");
      exit(0);
    }
    std::string content = event.msg.content;
    std::ranges::transform(content, content.begin(), ::tolower);
    if (content == "what" || content == "what?" || content == "qua" ||
        content == "qua?" || content == "juyoo" || content == "w:info") {
      if (content == "w:info") {
        event.reply(
            "This bot is created by aelnosu, licensed under the "
            "`BSD-3-Clause-No-Nuclear-License-2014`.\nThe code can be "
            "found at https://github.com/Eason-Y-Lu/discord-what-qua.\n "
            "Build: v3.1.0");
      } else {
        switch (const auto [msg_content, asked_times] =
                    lookup_msg(main_what_db_vector, event.msg.channel_id,
                               event.msg.guild_id, event.msg.author.id);
                asked_times) {
        default:
          event.reply(
              "This is not supposed to happen, notify <@1110811715169423381>");
          bot.log(
              dpp::loglevel::ll_error,
              "This is not supposed to happen, notify <@1110811715169423381>");
          bot.log(dpp::loglevel::ll_error,
                  "You can also email him at: aelnosu@outlook.com");
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
          const int random_insult_index =
              gen_rand_int(0, static_cast<int>(insults.size()));
          event.reply("# " + insults[random_insult_index]);
          break;
        }
      }
    } else {
      add_message_to_db(event.msg.guild_id, event.msg.channel_id,
                        event.msg.author.id, event.msg.content,
                        main_what_db_vector);
    }
  });
  bot.start(dpp::st_wait);
  return 0;
}
