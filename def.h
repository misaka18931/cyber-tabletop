#pragma once

#include <vector>
#include <string>
#include <map>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

/*  */
enum action {
  GAME_PRGS          = 0x00,
  GAME_INTR          = 0x01,
  GAME_ABRT          = 0x02,
  GAME_INIT          = 0x10,
  REGISTER_PLAYER    = 0x11,
  REMOVE_PLAYER      = 0x12,
  SETTING            = 0x13
};

class player;

extern const int player_cnt_max;
extern const int player_cnt_min;
extern std::map<std::string, player> players;
extern std::vector<player*> player_ord;
extern int host;
extern int player_cnt;
extern int curr;
extern bool running;

class attr {

};

class player {
public:
  int ord;
  std::string id;
  attr x;
};

void senderr(player*, std::string);
void sendmsg(player*, std::string);

void init_game();
void abort_game();
/* a call to round_start signals the start of a round */
void round_start();
/* a call to round_end signals the end of a round
   it is responsible of setting up the game for the next round */ 
void round_end(int);
void game_advance(player*, json);
void game_interrupt(player*, json);
/* a call to terminate_game ends the game normally */
void terminate_game();
