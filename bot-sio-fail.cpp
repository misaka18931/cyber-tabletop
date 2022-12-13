#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <sio_client.h>

using json = nlohmann::json;

#include "def.h"

sio::client io;

void to_json(json& j, const msg& p) {
  j = json{{"public", p.is_public},
           {"user", p.user},
           {"event", p.event},
           {"content", p.content}};
}

void from_json(const json& j, msg& p) {
  j.at("public").get_to(p.is_public);
  j.at("user").get_to(p.user);
  j.at("event").get_to(p.event);
  j.at("content").get_to(p.content);
}

inline msg getmsg(std::istream& inf) {
  json o;
  inf >> o;
  msg m = o.get<msg>();
  return m;
}

// inline void sendmsg(const msg& m) { std::cout << json(m) << std::endl; }
inline void sendmsg(const msg& m) { io.socket()->emit("reply", json(m).dump()); }

void sendpub(const std::string& s, const std::string& u) {
  msg m = {.is_public = true, .event = "reply", .user = u, .content = s};
  sendmsg(m);
}

void sendpriv(const std::string& u, const std::string& s) {
  msg m = {.is_public = false, .event = "reply", .user = u, .content = s};
  sendmsg(m);
}

#ifndef DEBUG
std::mt19937_64 rng(
    std::chrono::steady_clock::now().time_since_epoch().count());
#else
std::mt19937_64 rng(114514);
#endif

int main(int argc, char** argv) {
  io.connect("http://127.0.0.1:3333");
  while (true) {
    try {
      auto msg = getmsg(std::cin);
      auto event = handlers.find(msg.event);
      if (event == handlers.end())
        throw std::string("undefined event: " + msg.event);
      if (msg.is_public) {
        if (event->second.first) {
          event->second.first(msg.user, msg.content);
        }
      } else {
        if (event->second.second) {
          event->second.second(msg.user, msg.content);
        }
      }
    } catch (json::exception e) {
      std::cerr << "[main loop]: json processing error: " << e.what() << '\n';
    } catch (std::string e) {
      std::cerr << "[main loop]: " << e << '\n';
    } catch (const char* const e) {
      std::cerr << "[main loop]: " << e << '\n';
    }
  }
  return 0;
}