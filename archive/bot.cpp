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
std::vector<player*> ring;

void to_json(json& j, const msg& p) {
  j = json{
    {"public", p.is_public},
    {"user", p.user},
    {"action", p.cmd},
    {"content", p.content}
  };
}

void from_json(const json& j, msg& p) {
  j.at("public").get_to(p.is_public);
  j.at("user").get_to(p.user);
  j.at("action").get_to(p.cmd);
  j.at("content").get_to(p.content);
}

void msg::load_player() {
  auto iter = players.find(this->user);
  this->p = iter == players.end() ? nullptr : &iter->second;
}

inline void sendmsg(const msg& m) {
  std::cout << json(m) << std::endl;
}

void sendpub(const std::string& s) {
  msg m = {
    .is_public = true,
    .cmd = action::SENT,
    .content = s
  };
  sendmsg(m);
}

void sendpriv(const std::string &u, const std::string& s) {
  msg m = {
    .is_public = false,
    .cmd = action::SENT,
    .user = u,
    .content = s
  };
  sendmsg(m);
}

inline msg getmsg(std::istream &inf) {
  json o;
  inf >> o;
  msg m = o.get<msg>();
  m.load_player();
  return m;
}

void run_game() {
  running = true;
  while (running) {
    try {
      round_start();
      auto msg = getmsg(std::cin);
      if (!msg.p) continue; // player not in the game cannot interact
      switch (msg.cmd) {
        case action::GAME_INTR:
          if (msg.is_public)
            game_interrupt(msg.p, msg.content);
          continue;
        case action::GAME_PRGS:
          if (msg.p->ord == curr % player_cnt)
            game_advance(msg.p, msg.content);
          continue;
        case action::GAME_ABRT:
          abort_game();
          continue;
      }
      if (curr == host + player_cnt) {
        round_end(curr + 1);
      }
    } catch (json::exception e) {
      std::cerr << "[in game]: json processing error: " << e.what() << '\n';
    }
  }
}

// std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
std::mt19937_64 rng(114514);

int main(int argc, char **argv) {
  while (true) {
    try {
      const auto msg = getmsg(std::cin);
      if (!msg.is_public) {
        continue;
      }
      switch (msg.cmd) {
        case action::GAME_INIT:
          if (players.size() > player_cnt_min) {
            init_game();
            run_game();
          }
          else sendpub("insufficient players.");
          continue;
        ;;
        case action::REGISTER_PLAYER:
          if (players.size() < player_cnt_max) {
            players[msg.user] = player{ .id = msg.user };
            sendpub(fmt::format("{}/{} joined.", players.size(), player_cnt_max));
          }
          else sendpub("room full");
          continue;
        case action::REMOVE_PLAYER:
          if (players.erase(msg.user))
            sendpub(fmt::format("player \"{}\" removed.", msg.user));
          else
            sendpub("no such player");
          continue;
        default:
          throw "undefined action";
        ;;
      }
    } catch (json::exception e) {
      std::cerr << "[main loop]: json processing error: " << e.what() << '\n';
    } catch (std::string e) {
      std::cerr << "[main loop]: " << e << '\n';
    }
  }
  return 0;
}