#include <fmt/format.h>

#include "def.h"

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
    16, 16, 16, 16, 16, 16, 9,  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
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
  inline std::string to_str() const {
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
  attr operator+(const attr &rhs) const {
    attr ret(*this);
    ret.size += rhs.size;
    for (int i = 0; i < 13; ++i) ret.card[i] += rhs.card[i];
    return ret;
  }
  attr operator-(const attr &rhs) const {
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

struct player {
  int id;
  std::string name;
  attr cards;
  player() = default;
  player(const std::string &s) : name(s){};
  inline std::string get_name() const {
    return fmt::format("@{}[{}] ", name, cards.size);
  }
};

std::map<std::string, player> players;
std::vector<player *> ring;

int player_cnt;
int host;
int curr;
attr last_move, pool;
int8_t card_of_the_round;
bool terminal_mode;
bool running;

inline player *get_player(const std::string &u) {
  auto iter = players.find(u);
  return iter == players.end() ? nullptr : &(iter->second);
}

inline void print_cards(const player *p) {
  sendpriv(p->name, fmt::format("您的手牌: {}", p->cards.to_str()));
}

void init_round(int _host) {
  curr = host = _host;
  last_move = {};
  card_of_the_round = -1;
  sendpub(fmt::format("@{} leads.", ring[host]->name));
  sendpriv(ring[host]->name, "LEAD");
}

void prompt_next() {
  if (curr != host + player_cnt) {
    sendpriv(ring[curr % player_cnt]->name,
             fmt::format("FOLLOW {}", cardname[card_of_the_round]));
  } else {
    init_round(host);
  }
}

void cleanup() {
  running = false;
  ring.clear();
  terminal_mode = false;
}

void terminate_game(const player *winner) {
  sendpub(fmt::format("@{}获胜", winner->id));
  cleanup();
}

void abort_game(const std::string &_u, const json &req) { cleanup(); }

void follow(const std::string &_u, const json &req) {
  if (!running || terminal_mode || curr == host) return;
  auto p = get_player(_u);
  if (!p || p->id != curr) return;
  try {
    attr _move = (req["cards"].get<std::string>());
    if (!_move.size) throw "empty move.";
    const auto curr_cards = p->cards - _move;
    pool = pool + _move;
    last_move = _move;
    sendpub(fmt::format("@{}打出了{}张{}.", p->get_name(), _move.size,
                        cardname[card_of_the_round]));
    p->cards = curr_cards;
    print_cards(p);
    ++curr;
    sendpub(fmt::format("牌堆共{}张.", pool.size));
    if (!p->cards.size) {
      sendpub(fmt::format("@{}跑了，现在只允许质疑", p->name));
      terminal_mode = true;
    } else
      prompt_next();
  } catch (json::exception e) {
    sendpriv(p->name, e.what());
  } catch (std::string e) {
    sendpriv(p->name, e);
  } catch (const char *const e) {
    sendpriv(p->name, e);
  }
}

void lead(const std::string &_u, const json &req) {
  if (!running || terminal_mode || curr != host) return;
  auto p = get_player(_u);
  if (!p || p->id != curr) return;
  try {
    std::string declstr = req["decl"];
    if (declstr.size() != 1) throw "invalid decl";
    int8_t _card = asso_values[declstr[0]];
    if (_card > 12) {
      throw "invalid decl";
    }
    card_of_the_round = _card;
    attr _move(req["cards"].get<std::string>());
    if (!_move.size) throw "empty move.";
    const auto curr_cards = p->cards - _move;
    pool = pool + _move;
    last_move = _move;
    sendpub(fmt::format("{}打出了{}张{}.", p->get_name(), _move.size,
                        cardname[card_of_the_round]));
    p->cards = curr_cards;
    print_cards(p);
    ++curr;
    sendpub(fmt::format("牌堆共{}张.", pool.size));
    if (!p->cards.size) {
      sendpub(fmt::format("@{} 跑了，现在只允许质疑", p->name));
      terminal_mode = true;
    } else
      prompt_next();
  } catch (json::exception e) {
    sendpriv(p->name, e.what());
  } catch (std::string e) {
    sendpriv(p->name, e);
  } catch (const char *const e) {
    sendpriv(p->name, e);
  }
}

void pass(const std::string &_u, const json &_arg) {
  if (!running || terminal_mode || curr == host) return;
  auto p = get_player(_u);
  if (!p || p->id != curr) return;
  sendpub(fmt::format("{} passed.", p->get_name()));
  ++curr;
  prompt_next();
}

void quest(const std::string &_u, const json &_arg) {
  if (!running || card_of_the_round == -1) return;
  auto p = get_player(_u);
  if (!p || p->id == (curr - 1) % player_cnt) return;
  player *defendent = ring[(curr - 1) % player_cnt];
  sendpub(fmt::format("{}发出质疑！\n{}翻开{}张牌，是{}", p->get_name(),
                      defendent->get_name(), last_move.size,
                      last_move.to_str()));
  if (last_move.card[card_of_the_round] != last_move.size) {
    // success
    sendpub(fmt::format("质疑成功！\n{}拿回{}张牌！", defendent->get_name(),
                        pool.size));
    print_cards(defendent);
    defendent->cards = defendent->cards + pool;
    pool = {};
    init_round(p->id);
  } else {
    // fail
    sendpub(
        fmt::format("质疑失败！\n{}拿回{}张牌！", p->get_name(), pool.size));
    print_cards(p);
    p->cards = p->cards + pool;
    pool = {};
    if (defendent->cards.size)
      init_round(curr - 1);
    else
      terminate_game(defendent);
  }
}

void init(const std::string &_u, const json &_arg) {
  if (running) return;
  if (players.size() < player_cnt_min) {
    sendpub("insufficient players");
    return;
  }
  running = true;
  player_cnt = players.size();
  last_move = {};
  pool = {};
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
    ring[i]->id = i;
    msg += "@" + ring[i]->name + " ";
    ring[i]->cards = attr(full.substr(13 * i, 13));
    print_cards(ring[i]);
  }
  sendpub("游戏开始！");
  sendpub(msg);
  init_round(0);
}

void join(const std::string &_u, const json &_arg) {
  if (running) return;
  if (players.size() >= player_cnt_max) {
    sendpub("failure: room full.", _u);
  } else if (!players.insert({_u, player(_u)}).second) {
    sendpub("failure: player exists.", _u);
  } else
    sendpub(fmt::format("success.\n{}/{} in room.", players.size(),
                        player_cnt_max));
}

void leave(const std::string &_u, const json &_arg) {
  if (running) return;
  if (!players.erase(_u)) {
    sendpub("failure: not in player list.", _u);
  } else {
    sendpub(fmt::format("success.\n{}/{} in room.", players.size(),
                        player_cnt_max));
  }
}

const std::map<std::string, std::pair<event_handler, event_handler> > handlers{
    {"follow", {null_event_handler, &follow}},
    {"lead", {null_event_handler, &lead}},
    {"init", {&init, null_event_handler}},
    {"join", {&join, null_event_handler}},
    {"leave", {&leave, null_event_handler}},
    {"pass", {null_event_handler, &pass}},
    {"quest", {&quest, &quest}},
    {"abort", {&abort_game, null_event_handler}},
};
