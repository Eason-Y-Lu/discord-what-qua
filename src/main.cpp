// SPDX-License-Identifier: BSD-3-Clause-No-Nuclear-License-2014
#include "ins.h"
#include <csignal>
#include <dpp/dpp.h>
#include <fstream>
#include <mutex>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <string>
#include <vector>

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

std::mutex on_message_received_mutex;

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
  std::vector<what_channel_blob> channel_data;
};

void add_message_to_db(const dpp::snowflake guild_id,
                       const dpp::snowflake channel_id,
                       const dpp::snowflake author_id,
                       const std::string &message,
                       std::vector<what_server_blob> &main_what_db_vector) {
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

lookup_result lookup_msg(std::vector<what_server_blob> &main_what_db_vector,
                         const dpp::snowflake channel_id,
                         const dpp::snowflake guild_id,
                         const dpp::snowflake author_id) {
  for (auto &server_blob : main_what_db_vector) {
    if (server_blob.server_id == guild_id) {
      for (auto &channel_blob : server_blob.channel_data) {
        if (channel_blob.channel_id == channel_id) {
          if (channel_blob.message_1.author_id == author_id &&
              channel_blob.message_2.author_id != 0) {
            channel_blob.message_2.asked_times++;
            return {channel_blob.message_2.message,
                    channel_blob.message_2.asked_times};
          } else if (channel_blob.message_2.author_id == author_id &&
                     channel_blob.message_1.author_id != 0) {
            channel_blob.message_1.asked_times++;
            return {channel_blob.message_1.message,
                    channel_blob.message_1.asked_times};
          } else if (channel_blob.message_1.author_id == author_id &&
                     channel_blob.message_2.author_id == 0) {
            return {"I don't know.", 0};
          } else if (channel_blob.message_2.author_id == author_id &&
                     channel_blob.message_1.author_id == 0) {
            return {"I don't know.", 0};
          } else {
            channel_blob.message_1.asked_times++;
            return {channel_blob.message_1.message,
                    channel_blob.message_1.asked_times};
          }
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
  std::vector<what_server_blob> *main_what_db_vector_ptr =
      new std::vector<what_server_blob>();
  std::vector<what_server_blob> &main_what_db_vector = *main_what_db_vector_ptr;
  std::signal(SIGINT, endSignalHandler);
  dpp::cluster bot(read_token(),
                   dpp::i_guild_messages | dpp::i_message_content);
  bot.on_log(dpp::utility::cout_logger());

  bot.on_ready([&bot](const dpp::ready_t &event) {
    bot.log(dpp::loglevel::ll_info, "Logged in as " + bot.me.username);
    bot.log(dpp::loglevel::ll_info, "Bot ID is " + bot.me.id.str());
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
    std::lock_guard<std::mutex> lock(on_message_received_mutex);
    if (content == "what" || content == "what?" || content == "qua" ||
        content == "qua?" || content == "w:info") {
      if (content == "w:info") {
        event.reply("This bot is created by aelnosu, licensed under the "
                    "`BSD-3-Clause-No-Nuclear-License-2014`.\nThe code can be "
                    "found at https://github.com/Eason-Y-Lu/discord-what-qua.");
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
