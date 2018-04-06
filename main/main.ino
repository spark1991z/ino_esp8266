#ifdef ESP8266
extern "C" {
    #include "user_interface.h"
}
#endif //ESP8266
#define string String
#include<vector>

using namespace std;

/***************
  Project Stage
 ***************/
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
static const string stage2str(const stage_t&_stage) {
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
    return "unknown";
}
static const string project_name          = "ASAIOP";
static const stage_t project_stage_type   = SKELETON;
static const string project_stage() {
    string out = "-";
    out += stage2str(project_stage_type);
    return out;
}
static const string version_basic(const double&_main, const char&_type) {
    string out;
    out += _main;
    out += _type;
    return out;
}
static const string version_full(const double&_main, const long&_extra, const char&_type) {
    string out = version_basic(_main,_type);
    out += '(';
    out += _extra;
    out += ')';
    return out;
}
/*****
 Core
 *****/
namespace core {
    static const double  version_main   = 0.06,
                         version_extra  = 20180407;
    static const string core_version_basic() {
        return version_basic(version_main,'c');
    }
    static const string core_version_full() {
        return version_full(version_main,version_extra,'c');
    }
    namespace utils {
        /********
          String
         ********/
        static const int indexOf(const string&_source, const int&_start, const char&_search) {
            if(_start>=0) for(int i=_start; i<_source.length();i++)
                if(_source[i]==_search) return i;
            return -1;
        }
        static const int indexOf(const string&_source, const int&_start, const string&_search) {
            bool detect = false;
            if(_start>=0 && _search.length()>_start) for(int i=0;i<_source.length();i++) {
                if(_source[i] == _search[0]) for(int j=0;j<_search.length();j++) {
                    if(i+j == _source.length() || _source[i+j] != _search[j]) {
                        detect = false;
                        break;
                    }
                    detect = true;
                }
                if(detect) return i;
            }
            return -1;
        }
        static const string substring(const string&_source, const int&_start, const int&_end) {
            string out;
            if(_start>=0 && _start < _end && _end <= _source.length()) for(int i=_start;i<_end;i++)
                out += _source[i];
            return out;
        }
        static const vector<string> split(const string&_source, const char&_delim) {
            vector<string> out;
            string cur;
            for(int i=0;i<_source.length()+1;i++) {
                if(_source[i] == _delim || i == _source.length()) {
                    out.push_back(cur);
                    cur = "";
                    continue;
                }
                cur += _source[i];
            }
            return out;
        }
        static const string trim(const string&_source) {
            string out = _source;
            if(out[0] = ' ' || out[0] == '\r')
                out = substring(out,1,out.length());
            if(out[out.length()-1] == ' ' || out[out.length()-1] == '\r')
                out = substring(out,0,out.length()-1);
            return out;
        }
        template<typename T> static const string any2str(const T&_t) {
            return string(_t);
        }
        /********
          Number
         ********/
        static const float pow(const float&_in, const int&_pow) {
            if(_pow < 0) {
                return 1/pow(_in,-_pow);
            }
            if(_pow == 0) return 1;
            float _out = _in;
            for(int i=1;i<_pow;i++) _out *= _in;
            return _out;
        }
        static const bool isNum(const string&_source) {
            bool _out = true, _minus=false, _point = false;
            for(int i=0;i<_source.length();i++) {
                if(i == 0 && i+1 < _source.length() && _source[i] == '-') {
                    _minus = true;
                    continue;
                }
                if(_source[i] < '0' || _source[i] > '9') {
                    if(!_point && (!_minus && i>=1 || _minus && i>=2) && (_source[i] == '.' || _source[i] == ',')) {
                        _point = true;
                        continue;
                    }
                    return false;
                }
            }
            return _out;
        }
        static const float str2num(const string&_source) {
            if(!isNum(_source)) return -1;
            float _out = 0;
            bool _minus = _source[0] == '-', _end1 = false, _end2 = false;
            int idx = indexOf(_source,0,'.'), _pow = 0;
            if(idx < 0) idx = indexOf(_source,0,',');
            if(idx < 0) idx = _source.length();
            for(int i=0;;i++) {
                if(idx+1+i < _source.length()) _out += pow(0.1,_pow+1)*(_source[idx+1+i]-'0');
                else _end1 = true;
                if(idx-1-i >= _minus ? 1 : 0) _out += pow(10,_pow)*(_source[idx-1-i]-'0');
                else _end2 = true;
                if(_end1 && _end2) break;
                _pow++;
            }
            return _minus ? -_out : _out;
        }
    }
}
/*****
  Net
 *****/
namespace net {
    static const double  version_main    = 0.01,
                         version_extra   = 20180404;
    static const string net_version_basic() {
        return version_basic(version_main,'n');
    }
    static const string net_version_full() {
        return version_full(version_main,version_extra,'n');
    }
    namespace wlan {
        static void printDiag(Print&_p) {}
    }
    namespace http {
        static const string protocol_name   = "HTTP";
        static const float protocol_version = 1.1;
        static const string info() {
            string out = protocol_name;
            out += '/';
            out += protocol_version;
            return out;
        }
        enum status_t{};
        class HTTPServlet{};
        class HTTPConnection{};
        class HTTPServer{};
    }
}
/*******
  Extra
 *******/
namespace extra {
    static const double version_main    = 0.04,
                        version_extra   = 20180407;
    static const string extra_version_basic() {
        return version_basic(version_main,'e');
    }
    static const string extra_version_full() {
        return version_full(version_main,version_extra,'e');
    }
    class Formatter {
        private:
            static vector<string> _values;
            Formatter() {}
        public:
            static void reset() {
                _values.clear();
            }
            template<typename T> static void add(const T&_t) {
                _values.push_back(core::utils::any2str(_t));
            }
            static const string format(const string&_format) {
                string _out, _num;
                int _mode = 0;
                for(int i=0;i < _format.length(); i++) {
                    switch(_mode) {
                        case 0:
                            if(_format[i] == '[') {
                                _mode = 1;
                                continue;
                            }
                            _out += _format[i];
                            break;
                        case 1:
                            if(_format[i] == ']') {
                                float idx = core::utils::str2num(_num);
                                if(idx < 0 || idx > _values.size()) _out += "[#]";
                                else _out += _values[idx];
                                _mode = 0;
                                _num = "";
                                continue;
                            }
                            if(_format[i] >= '0' && _format[i] <= '9') {
                                _num += _format[i];
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
    vector<string> Formatter::_values;
    class Log {
        private:
            static const long _init_stamp;
            template<typename T> static void echo(const string&_service, const T&_t) {
                Formatter::reset();
                Formatter::add((float)(millis() - _init_stamp) / 1000);
                Formatter::add(_service);
                Formatter::add(_t);
                Serial.println(Formatter::format("[[0]][[1]] [2]"));
            }
            Log() {}
        public:
            template<typename T> static void info(const T&_t) {
                echo("info",_t);
            }
            template<typename T> static void error(const T&_t) {
                echo("error",_t);
            }
    };
    const long Log::_init_stamp = millis();
    class Led {};
    class SensorManager{};
    class Virtuino{};
    class DallasMonitor{};
    class RootIndexServlet: public net::http::HTTPServlet{};
}
/********
  Sketch
 ********/
using namespace core;
using namespace net;
using namespace extra;

static void printVersion(const bool& _full) {
    Serial.print(project_name);
    Serial.print(" v");
    Serial.print(_full ? core::core_version_full() : core_version_basic());
    Serial.print(_full ? net::net_version_full() : net_version_basic());
    Serial.print(_full ? extra::extra_version_full() : extra_version_basic());
    Serial.println(project_stage());
}

void setup() {
    Serial.begin(115200);
    printVersion(false);
    Log::info("Hello world!");
}

static const int _update_delay = 1000;
static unsigned long _next_update = millis();

void loop() {}
