#include "def.h"

const int player_cnt_min = 2;
const int player_cnt_max = 4;
/*
4 人游戏，游戏开始时为每名玩家随机分配 13 张手牌，打出的手牌在不被质疑的情况下不会被其他玩家直接知晓。
声明：一名玩家向所有玩家公告自己所打出所有牌的大小都是相同的某个值。声明可以为假也就是说谎，如打出 6QK 并声明 6，则此声明是假的。
牌池：一名玩家打出的手牌会立刻进入牌池。
出牌 (LEAD)：开始新回合，然后打出任意张牌，做出任意声明。
跟牌 (FOLLOW)：打出任意张牌，做出与本回合 LEADER 相同的声明。
过 (PASS)：不打出任何牌，不做出任何声明。
质疑 (QUEST)：你展示本回合上一名打出手牌的玩家 (LEADER/FOLLOWER) 打出的牌，该玩家不存在则不能质疑。若其声明为假，则该玩家获得牌池中所有牌，你 LEAD；否则你获得牌池中所有牌，该玩家 LEAD。你不能 QUEST 自己。
游戏流程：玩家的顺序为固定的环，在发牌时随机生成，游戏以第一名玩家的 LEAD 开始。对于一次出牌，在下名玩家 FOLLOW 或 PASS 之前所有玩家可以 QUEST。若连续 3 个人 PASS，则下一个人 LEAD。打完手牌时不被成功质疑的第一名玩家胜利。
*/
void init_game() {
  if (players.size() < player_cnt_min) {
    senderr(nullptr, "insufficient players.");
  }
}

void game_interrupt(player *p, json req) {

}

void game_advance(player *p, json req) {

}

void round_start() {

}

void round_end(int nxt_host) {

}

void abort_game() {

}