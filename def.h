#pragma once

#include <vector>
#include <string>
#include <map>
#include <random>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

struct attr;

/*  */
enum action {
  GAME_PRGS          = 0x00,
  GAME_INTR          = 0x01,
  GAME_ABRT          = 0x02,
  GAME_INIT          = 0x10,
  REGISTER_PLAYER    = 0x11,
  REMOVE_PLAYER      = 0x12,
  SETTING            = 0x13,
  SENT               = 0xff
};

class player;

extern const int player_cnt_max;
extern const int player_cnt_min;
extern std::map<std::string, player> players;
extern std::vector<player*> ring;
extern std::mt19937_64 rng;
extern int host;
extern int player_cnt;
extern int curr;
extern bool running;


struct msg {
  bool is_public;
  action cmd;
  player *p;
  std::string user;
  json content = json::object();
  inline void load_player();
};

void to_json(json& j, const msg& p);

void from_json(const json& j, msg& p);

class player {
public:
  int ord;
  std::string id;
  attr *x;
};

inline void sendmsg(const msg&);
void sendpub(const std::string&);
void sendpriv(const std::string&, const std::string&);

void init_game();
void abort_game();
/* a call to round_start signals the start of a round */
void round_start();
/* a call to round_end signals the end of a round
   it is responsible of setting up the game for the next round */ 
void round_end(int);
void game_advance(player*, json);
void game_interrupt(player*, json);
/* terminate the game and announce winnet */
void terminate_game(const player*);
void cleanup();
