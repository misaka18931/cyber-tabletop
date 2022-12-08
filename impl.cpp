#include "def.h"
#include <fmt/format.h>

const int player_cnt_min = 2;
const int player_cnt_max = 4;
/*
4 人游戏，游戏开始时为每名玩家随机分配 13
张手牌，打出的手牌在不被质疑的情况下不会被其他玩家直接知晓。
声明：一名玩家向所有玩家公告自己所打出所有牌的大小都是相同的某个值。声明可以为假也就是说谎，如打出
6QK 并声明 6，则此声明是假的。 牌池：一名玩家打出的手牌会立刻进入牌池。 出牌
(LEAD)：开始新回合，然后打出任意张牌，做出任意声明。 跟牌
(FOLLOW)：打出任意张牌，做出与本回合 LEADER 相同的声明。 过
(PASS)：不打出任何牌，不做出任何声明。 质疑
(QUEST)：你展示本回合上一名打出手牌的玩家 (LEADER/FOLLOWER)
打出的牌，该玩家不存在则不能质疑。若其声明为假，则该玩家获得牌池中所有牌，你
LEAD；否则你获得牌池中所有牌，该玩家 LEAD。你不能 QUEST 自己。
游戏流程：玩家的顺序为固定的环，在发牌时随机生成，游戏以第一名玩家的 LEAD
开始。对于一次出牌，在下名玩家 FOLLOW 或 PASS 之前所有玩家可以 QUEST。若连续 3
个人 PASS，则下一个人 LEAD。打完手牌时不被成功质疑的第一名玩家胜利。
*/

const std::string cardname = "123456789XJQK";
const int8_t asso_values[] = {
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 0,  1,  2,  3,  4,  5,  6,  7,
    8,  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 10, 12,
    16, 16, 16, 16, 16, 11, 16, 16, 16, 16, 16, 16, 9,  16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 10, 12, 16, 16, 16, 16, 16, 11,
    16, 16, 16, 16, 16, 16,  9, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16};

struct attr {
  int8_t card[13] = {};
  int8_t size = 0;
  // inline void push(int8_t u) {
  //   ++size;
  //   ++card[u];
  // }
  inline std::string to_str() {
    std::string ret;
    for (int i = 0; i < 13; ++i) {
      for (int k = 0; k < card[i]; ++k) {
        ret.push_back(cardname[i]);
      }
    }
    return ret;
  }
  attr(const std::string &s) {
    size = s.size();
    for (auto c : s) {
      if (asso_values[c] > 12) throw "invalid card.";
      ++card[asso_values[c]];
    }
  }
  attr operator+(const attr &rhs) {
    attr ret(*this);
    ret.size += rhs.size;
    for (int i = 0; i < 13; ++i)
      ret.card[i] += rhs.card[i];
    return ret;
  }
  attr operator-(const attr &rhs) {
    attr ret(*this);
    ret.size -= rhs.size;
    for (int i = 0; i < 13; ++i) {
      if (ret.card[i] < rhs.card[i]) throw "insufficient cards.";
      ret.card[i] -= rhs.card[i];
    }
    return ret;
  }
  attr() = default;
};

attr last_move, pool;
int8_t card_of_the_round;
bool terminal_mode;
bool round_started_flag = false;

inline void print_cards(const player* p) {
  sendpriv(p->id, fmt::format("您的手牌: {}", p->x->to_str()));
}

void init_game() {
  player_cnt = players.size();
  last_move = {};
  pool = {};
  card_of_the_round = -1;
  terminal_mode = false;
  for (auto &[id, p] : players) {
    ring.push_back(&p);
  }
  std::shuffle(begin(ring), end(ring), rng);
  std::string full;
  for (int t = 0; t < player_cnt; ++t) full += cardname;
  shuffle(begin(full), end(full), rng);
  std::string msg = "顺序： ";
  for (size_t i = 0; i < ring.size(); ++i) {
    ring[i]->ord = i;
    msg += "@" + ring[i]->id + " ";
    ring[i]->x = new attr(full.substr(13*i, 13));
    print_cards(ring[i]);
  }
  sendpub("游戏开始！");
  sendpub(msg);
}


void game_interrupt(player *p, json) {
  terminal_mode = false;
  if (card_of_the_round != -1 && p->ord != (curr - 1) % player_cnt) {
    const player *defendent = ring[(curr - 1) % player_cnt];
    sendpub(fmt::format("@{}发出质疑！\n@{}翻开{}张牌，是{}", p->id, defendent->id, last_move.size, last_move.to_str()));
    if (last_move.card[card_of_the_round] != last_move.size) {
      // success
      sendpub(fmt::format("质疑成功！\n@{}拿回{}张牌！", defendent->id, pool.size));
      print_cards(defendent);
      *defendent->x = *defendent->x + pool;
      pool = {};
      round_end(p->ord);
    } else {
      // fail
      sendpub(fmt::format("质疑失败！\n@{}拿回{}张牌！", p->id, pool.size));
      print_cards(p);
      *p->x = *p->x + pool;
      pool = {};
      if (defendent->x->size)
        round_end(curr - 1);
      else
        terminate_game(defendent);
    }
  }
}

void game_advance(player *p, json req) {
  if (terminal_mode) return;
  if (p->ord == host && req["cmd"] == "lead") {
    try {
      std::string declstr = req["decl"];
      if (declstr.size() != 1)
        throw "invalid decl";
      int8_t _card = asso_values[declstr[0]];
      if (_card > 12) {
        throw "invalid decl";
      }
      card_of_the_round = _card;
      attr _move(req["cards"].get<std::string>());
      if (!_move.size) throw "empty move.";
      *p->x = *p->x - _move;
      pool = pool + _move;
      last_move = _move;
      sendpub(fmt::format("@{}打出了{}张{}.", p->id, _move.size, cardname[card_of_the_round]));
      print_cards(p);
      ++curr;
      if (!p->x->size) {
        sendpub(fmt::format("@{}跑了，现在只允许质疑", p->id));
        terminal_mode = true;
      } else if (curr != host + player_cnt) {
        sendpriv(ring[curr % player_cnt]->id, fmt::format("FOLLOW {}", cardname[card_of_the_round]));
      }
    } catch (json::exception e) {
      sendpriv(p->id, e.what());
    } catch (std::string e) {
      sendpriv(p->id, e);
    } catch (const char *const e) {
      sendpriv(p->id, e);
    }
  } else if (p->ord != host && req["cmd"] == "follow") {
    try {
      attr _move = (req["cards"].get<std::string>());
      if (!_move.size) throw "empty move.";
      *p->x = *p->x - _move;
      pool = pool + _move;
      last_move = _move;
      sendpub(fmt::format("@{}打出了{}张{}.", p->id, _move.size, cardname[card_of_the_round]));
      print_cards(p);
      ++curr;
      if (!p->x->size) {
        sendpub(fmt::format("@{}跑了，现在只允许质疑", p->id));
        terminal_mode = true;
      } else if (curr != host + player_cnt) {
        sendpriv(ring[curr % player_cnt]->id, fmt::format("FOLLOW {}", cardname[card_of_the_round]));
      }
    } catch (json::exception e) {
      sendpriv(p->id, e.what());
    } catch (std::string e) {
      sendpriv(p->id, e);
    } catch (const char *const e) {
      sendpriv(p->id, e);
    }
  } else if (p->ord != host && req["cmd"] == "pass") {
    sendpub(fmt::format("@{} passed.", p->id));
    ++curr;
  }
}

void round_start() {
  if (round_started_flag) {
    return;
  }
  round_started_flag = true;
  sendpriv(ring[host]->id, "LEAD");
  sendpub(fmt::format("@{} leads.", ring[host]->id));
}

void round_end(int nxt_host) {
  if (terminal_mode) return;
  sendpub("回合结束");
  last_move = {};
  card_of_the_round = -1;
  curr = host = nxt_host % player_cnt;
  round_started_flag = false;
}

void terminate_game(const player* winner) {
  sendpub(fmt::format("@{}获胜", winner->id));
  cleanup();
}

void abort_game() {
  cleanup();
}

void cleanup() {
  for (auto [u, p] : players) {
    delete p.x;
  }
  running = false;
}