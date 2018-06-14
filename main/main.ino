#ifdef ESP8266
  extern "C" {
    #include "user_interface.h"
  }
#endif
#include <vector>
#include <map>
#define string String

namespace project {
  enum stage_t {
    PROTOTYPE,
    SKELETON,
    PRE_ALPHA,
    ALPHA,
    PRE_BETA,
    BETA,
    PRE_RELEASE,
    RELEASE,
    FINAL,
    END_OF_PROJECT
  };
  static const string str(const stage_t&_stage) {
    switch(_stage) {
      case PROTOTYPE: return "prototype";
      case SKELETON: return "skeleton";
      case PRE_ALPHA: return "pre_alpha";
      case ALPHA: return "alpha";
      case PRE_BETA: return "pre_beta";
      case BETA: return "beta";
      case PRE_RELEASE: return "pre_release";
      case RELEASE: return "release";
      case FINAL: return "final";
      case END_OF_PROJECT: return "end_of_project";
    }
    return "*";
  }
  static const string _name = "ALCOPYTHON";
  static const stage_t _stage = PRE_ALPHA;
  static const uint8_t _release = 1;
}
namespace core {
  static const float _version = 0.91;
  class Formatter {
    private:
      static std::vector<string> _values;
      static bool _begin_reason;
    public:
      static void begin() {
        if(_begin_reason) return;
        _begin_reason = true;
      }
      static void end() {
        if(!_begin_reason) return;
        _begin_reason = false;
        _values.clear();
      }
      template <typename T> static void add(const T&_t) {
        if(!_begin_reason) return;
        _values.push_back(string(_t));
      }
      static const string format(const string&_pattern) {
        if(!_begin_reason) return _pattern;
        string _out, _num;
        int _mode = 0;
        for(int i=0;i<_pattern.length(); i++) {
          switch(_mode) {
            case 0:
              if(_pattern[i] == '[') {
                _mode = 1;
                continue;
              }
              _out += _pattern[i];
              break;
            case 1:
              if(_pattern[i] == ']') {
                int _idx = _num.toInt();
                if(_idx < 0 || _idx >= _values.size()) _out += "[#]";
                else _out += _values[_idx];
                _mode = 0;
                _num = "";
                continue;
              }
              if(_pattern[i] >= '0' && _pattern[i] <= '9') {
                _num += _pattern[i];
                continue;
              }
              _mode = 0;
              _out += "[";
              _out += _num;
              i--;
              break;
          }
        }
        return _out;
      }
  };
  std::vector<string> Formatter::_values;
  bool Formatter::_begin_reason = false;
  class Log {
    private:
      static const long _init_stamp;
      Log() {}
      template <typename T> static void echo(const char&_service, const T&_t) {
        Formatter::begin();
        Formatter::add((float)(millis() - _init_stamp) / 1000);
        Formatter::add(_service);
        Formatter::add(_t);
        Serial.println(Formatter::format("[[0]][[1]]\t[2]"));
        Formatter::end();
      }
    public:
      template <typename T> static void info(const T&_t) {
        echo('I', _t);
      }
      template <typename T> static void error(const T&_t) {
        echo('E', _t);
      }
      template <typename T> static void debug(const T&_t) {
        echo('D', _t);
      }
  };
  const long Log::_init_stamp = millis();
}
namespace net {
  static const float _version = 0.0;
}
namespace extra {
  static const float _version = 0.0;
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();
  Serial.println("Board starting");
  #ifdef ESP8266
    core::Formatter::begin();
    core::Formatter::add("------------------------------------------");
    core::Formatter::add(project::_name);
    core::Formatter::add(core::_version);
    core::Formatter::add(net::_version);
    core::Formatter::add(extra::_version);
    core::Formatter::add(project::str(project::_stage));
    core::Formatter::add(project::_release);
    Serial.println(core::Formatter::format("[0]\n[1] version [2]c[3]n[4]e-[5]_[6]\n[0]"));
    core::Formatter::end();
  #else
    core::Log::error("Sorry, but this sketch only for ESP8266 boards!");
  #endif
}

void loop() {
  // put your main code here, to run repeatedly:

}
