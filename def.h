#pragma once

#include <vector>
#include <string>
#include <map>
#include <random>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/* RNG for the bot */

const size_t MSG_MAX_LENGTH = 65536;

extern std::mt19937_64 rng;

typedef void (*event_handler)(const std::string &, const json &);

const event_handler null_event_handler = nullptr;

extern const std::map<std::string, std::pair<event_handler, event_handler>> handlers;

struct msg {
  bool is_public;
  std::string event;
  std::string user;
  json content = json::object();
};

std::tuple<std::string, json> msg_parser(const std::string &m);

void to_json(json& j, const msg& p);

void from_json(const json& j, msg& p);

inline void sendmsg(const msg&);

void sendpub(const std::string&, const std::string& u = "");
void sendpriv(const std::string&, const std::string&);