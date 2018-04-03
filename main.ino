#ifndef ESP8266
    #include <iostream>
    #ifdef linux
        #include <unistd.h>
    #else
        #include <windows.h>
    #endif //linux
    #include <ctime>
    #include <sstream>
#else
    extern "C" {
        #include "user_interface.h"
    }
    #define string String
#endif //ESP8266
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
static const string stage2str(const stage_t& _stage) {
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
static const double project_stage_num     = 1.02,
                    project_version_extra = 20180404;
static const string project_stage() {
    #ifndef ESP8266
        stringstream ss;
        ss << '-' << stage2str(project_stage_type) << project_stage_num;
        return ss.str();
    #else
        string out = "-";
        out += stage2str(project_stage_type);
        out += project_stage_num;
        return out;
    #endif // ESP8266

}
static const string version_basic(const double& _main, const char& _type) {
    #ifndef ESP8266
        stringstream ss;
        ss << _main << _type;
        return ss.str();
    #else
        string out;
        out += _main;
        out += _type;
        return out;
    #endif // ESP8266

}
static const string version_full(const double& _main, const long& _extra, const char& _type) {
    #ifndef ESP8266
        stringstream ss;
        ss << version_basic(_main,_type) << '(' << _extra << ')';
        return ss.str();
    #else
        string out = version_basic(_main,_type);
        out += '(';
        out += _extra;
        out += ')';
        return out;
    #endif // ESP8266

}
/*****
 Core
 *****/
namespace core {
    static const double  version_main   = 0.02,
                         version_extra  = 20180404;
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
        static const int indexOf(const string& _source, const int& _start, const char& _search) {
            if(_start>=0) for(int i=_start; i<_source.length();i++)
                if(_source[i]==_search) return i;
            return -1;
        }
        static const int indexOf(const string& _source, const int& _start, const string& _search) {
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
        static const string substring(const string& _source, const int& _start, const int& _end) {
            string out;
            if(_start>=0 && _start < _end && _end <= _source.length()) for(int i=_start;i<_end;i++)
                out += _source[i];
            return out;
        }
        static const vector<string> split(const string& _source, const char& _delim) {
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
        static const string trim(const string& _source) {
            string out = _source;
            if(out[0] = ' ' || out[0] == '\r')
                out = substring(out,1,out.length());
            if(out[out.length()-1] == ' ' || out[out.length()-1] == '\r')
                out = substring(out,0,out.length()-1);
            return out;
        }
    }
    namespace formatter {
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
    #ifndef ESP8266
    class Print {
        public:
        Print(){}
        void write(const uint8_t& _c){}
        const uint8_t read();
        template<typename T> void print(const T& _t){}
        template<typename T> void println(const T& _t){}
        void println(){}
        void printf(const char* _format, ...){}
    };
    #endif // ESP8266
    namespace wlan {
        static void printDiag(Print& _p) {}
    }
    namespace http {
        static const string protocol_name    = "HTTP";
        static const float protocol_version = 1.1;
        static const string info() {
            #ifndef ESP8266
                stringstream ss;
                ss << protocol_name << '/' << protocol_version;
                return ss.str();
            #else
                string out = protocol_name;
                out += '/';
                out += protocol_version;
                return out;
            #endif // ESP8266
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
    static const double version_main    = 0.01,
                        version_extra   = 20180404;
    static const string extra_version_basic() {
        return version_basic(version_main,'e');
    }
    static const string extra_version_full() {
        return version_full(version_main,version_extra,'e');
    }
    namespace virtuino{}
    namespace sensorManager{}
    namespace dallasMonitor{}
    class RootIndexServlet: public net::http::HTTPServlet{};
}

using namespace core;
using namespace net;
using namespace extra;

#ifndef ESP8266
static void delay(const long& _delay) {
    #ifdef linux
        sleep(_delay);
    #else
        Sleep(_delay);
    #endif // linux
}
static long millis() {
    return time(NULL);
}
class SerialClass : public Print {
    private:
        bool _begin_reason;
    public:
        SerialClass() {
            _begin_reason = false;
        }
        void begin(const uint8_t& _speed) {
            if(_begin_reason) return;
            _begin_reason = true;
        }
        void write(const uint8_t& _c) {
            cout << _c;
        }
        const uint8_t read() {
            uint8_t _in;
            cin >> _in;
            return _in;
        }
        template<typename T> void print(const T& _t) {
            cout << _t;
        }
        void println() {
            cout << endl;
        }
        template<typename T> void println(const T& _t) {
            cout << _t << endl;
        }

};
static SerialClass Serial;
#endif // ESP8266
/********
  Sketch
 ********/
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
    Serial.println();
    printVersion(false);
    Serial.println();
    Serial.println("Hello world!");
}

void loop(){}

#ifndef ESP8266
static long _now = millis();
int main() {
    setup();
    while(true) {
        loop();
        delay(10);
    }
    return 0;
}
#endif //ESP8266
