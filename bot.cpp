#include <fmt/format.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <queue>
#include <string>
#include <vector>

using json = nlohmann::json;

#include "def.h"

int socket_fd, conn;
std::string socket_path;

void close_socket_bind(int sig) {
  unlink(socket_path.c_str());
  exit(sig);
}

void to_json(json& j, const msg& p) {
  j = json{{"public", p.is_public}, {"user", p.user}, {"content", p.content}};
}

void from_json(const json& j, msg& p) {
  j.at("public").get_to(p.is_public);
  j.at("user").get_to(p.user);
  j.at("content").get_to(p.content);
  std::tie(p.event, p.content) = msg_parser(j.at("content").get<std::string>());
}

inline msg getmsg(std::istream& inf) {
  json o;
  inf >> o;
  msg m = o.get<msg>();
  return m;
}

inline void sendmsg(const msg& m) {
  if (!socket_fd)
    std::cout << json(m) << std::endl;
  else {
    std::string str = json(m).dump();
#ifdef DEBUG
    std::cerr << "message sent: " << str << "\n";
#endif
    str.push_back('\0');  // null separation
    if (send(conn, str.c_str(), str.length(), 0) == -1) {
      perror("send error");
    }
  }
  // TODO: message separator
  // sleep(1);
}

inline bool getmsg_socket(msg& m) {
  static std::queue<msg> msg_queue;
  if (!msg_queue.empty()) {
    m = msg_queue.front();
    msg_queue.pop();
    return true;
  }
  static char buf[MSG_MAX_LENGTH];
  int len = recv(conn, buf, MSG_MAX_LENGTH, 0);
  if (len <= 0) {
    return false;
  }
#ifdef DEBUG
  std::cerr << "received message: " << buf << "\n";
#endif
  for (int i = 0, p = 0; i < len; ++i) {
    if (!buf[i]) {
      try {
        if (!p) {
          m = json::parse(buf + p, buf + i).get<msg>();
        } else {
          msg_queue.push(json::parse(buf + p, buf + i));
        }
      } catch (json::exception e) {
        std::cerr << "[main loop][getmsg_socket]: json processing error: "
                  << e.what() << '\n';
      } catch (std::string e) {
        std::cerr << "[main loop][getmsg_socket]: " << e << '\n';
      } catch (const char* const e) {
        std::cerr << "[main loop][getmsg_socket]: " << e << '\n';
      }
      p = i + 1;
    }
  }
  memset(buf, 0, len);
  return true;
}

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
  if (argc > 2) {
    std::cerr << argv[0] << ": 0 or 1 arguments required.\n";
    exit(1);
  } else if (argc == 2) {
    // IPC mode
    if ((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
      perror("socket error");
      exit(-1);
    }
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, argv[1], sizeof(addr.sun_path) - 1);
    if (bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr))) {
      perror("bind error");
      exit(-1);
    }
    socket_path = std::string(argv[1]);
    signal(SIGINT, close_socket_bind);
    signal(SIGTERM, close_socket_bind);
    if (listen(socket_fd, 1) == -1) {
      perror("listen error");
      exit(-1);
    }
    while (true) {
      conn = accept(socket_fd, nullptr, nullptr);
      if (conn == -1) {
        perror("accept error");
        continue;
      }
      for (msg m; getmsg_socket(m);) {
        try {
          auto event = handlers.find(m.event);
          if (event == handlers.end())
            throw std::string("undefined event: " + m.event);
          if (m.is_public) {
            if (event->second.first) {
              event->second.first(m.user, m.content);
            }
          } else {
            if (event->second.second) {
              event->second.second(m.user, m.content);
            }
          }
        } catch (json::exception e) {
          std::cerr << "[main loop]: json processing error: " << e.what()
                    << '\n';
        } catch (std::string e) {
          std::cerr << "[main loop]: " << e << '\n';
        } catch (const char* const e) {
          std::cerr << "[main loop]: " << e << '\n';
        }
      }
    }
    close(conn);
  } else {
    while (true) {
      try {
        auto m = getmsg(std::cin);
        auto event = handlers.find(m.event);
        if (event == handlers.end())
          throw std::string("undefined event: " + m.event);
        if (m.is_public) {
          if (event->second.first) {
            event->second.first(m.user, m.content);
          }
        } else {
          if (event->second.second) {
            event->second.second(m.user, m.content);
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
  }
  return 0;
}
