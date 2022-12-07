#include <algorithm>
#include <fmt/format.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "def.h"

int player_cnt;
int host;
int curr;
bool running;

std::map<std::string, player> players;
std::vector<player*> player_ord;

inline json getmsg(std::istream &inf) {
  json ret;
  inf >> ret;
  return ret;
}

void senderr(player* p, std::string errmsg) {
  json err = {
    {"player", p ? p->id : ""},
    {"status", "error"},
    {"what", errmsg},
  };
  std::cout << err << std::endl;
}

void sendmsg(player* p, std::string errmsg) {
  json err = {
    {"player", p ? p->id : ""},
    {"status", "ok"},
    {"what", errmsg},
  };
  std::cout << err << std::endl;
}

bool round_startd_flag = false;

void run_game() {
  running = true;
  while (running) {
    try {
      if (curr == host && ! round_startd_flag) {
        round_start();
        round_startd_flag = true;
      }
      auto msg = getmsg(std::cin);
      switch (msg["action"].get<int>()) {
        case action::GAME_INTR:
          game_interrupt(&players[msg["player"]], msg["content"]);
          continue;
        case action::GAME_PRGS:
          game_advance(&players[msg["player"]], msg["content"]);
          continue;
        case action::GAME_ABRT:
          abort_game();
          continue;
      }
      if (curr == host + player_cnt) {
        round_end(curr + 1);
        round_startd_flag = false;
      }
    } catch (...) {

    }
  }
}

int main(int argc, char **argv) {
  while (true) {
    auto msg = getmsg(std::cin);
    if (msg["type"] == "private") {
      continue;
    }
    try {
      switch (msg["action"].get<int>()) {
        case action::GAME_INIT:
          init_game();
          run_game();
        ;;
        case action::REGISTER_PLAYER:
          if (players.size() < player_cnt_max) {
            players[msg["player"]] = player();
            sendmsg(nullptr, fmt::format("{}/{} joined.", players.size(), player_cnt_max));
          }
          else senderr(nullptr, "room full");
          continue;
        case action::REMOVE_PLAYER:
          if (players.erase(msg["player"]))
            sendmsg(nullptr, fmt::format("player \"{}\" removed.", msg["player"]));
          else
            senderr(nullptr, "no such player");
          continue;
        default:
          throw;
        ;;
      }
    } catch (...) {
      
    }
  }
  return 0;
}