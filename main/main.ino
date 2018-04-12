#ifdef ESP8266
  extern "C" {
    #include "user_interface.h"
  }
  #include<vector>
  #define string String
  using namespace std;
  namespace project {
    enum stage_t {
      STAGE_PROTOTYPE,
      STAGE_SKELETON,
      STAGE_PRE_ALPHA,
      STAGE_ALPHA,
      STAGE_PRE_BETA,
      STAGE_BETA,
      STAGE_PRE_RELEASE,
      STAGE_RELEASE,
      STAGE_FINAL,
      STAGE_END_OF_PROJECT
    };
    static const string str(const stage_t&_stage) {
      switch(_stage) {
        case STAGE_PROTOTYPE: return "prototype";
        case STAGE_SKELETON: return "skeleton";
        case STAGE_PRE_ALPHA: return "pre_alpha";
        case STAGE_ALPHA: return "alpha";
        case STAGE_PRE_BETA: return "pre_beta";
        case STAGE_BETA: return "beta";
        case STAGE_PRE_RELEASE: return "pre_release";
        case STAGE_RELEASE: return "release";
        case STAGE_FINAL: return "final";
        case STAGE_END_OF_PROJECT: return "end_of_project";
      }
      return "unknown";
    }
    static const string project_name    = "ASAIOP";
    static const stage_t project_stage  = STAGE_SKELETON;

  }
  namespace core {
    static const float core_version  = 0.17;
    namespace utils {
      class Utils {
        private:
          Utils(){}
        public:
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
      };
      class Formatter {
        private:
          static vector<string> _values;
          static bool _lock;
          Formatter() {}
        public:
          static void reset() {
            _values.clear();
            _lock = false;
          }
          template<typename T> static void add(const T&_t) {
            if(!_lock) _values.push_back(Utils::any2str(_t));
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
                    float idx = Utils::str2num(_num);
                    if(idx < 0 || idx >= _values.size()) _out += "[#]";
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
            if(!_lock) reset();
            return _out;
          }
          static const bool lock() {
            return _lock;
          }
          static void lock(const bool&_lock) {
            Formatter::_lock = _lock;
          }
          
      };
      vector<string> Formatter::_values;
      bool Formatter::_lock;
      class Log {
        private:
          static const long& _init_stamp;
          template<typename T> static void echo(const char&_service, const T&_t) {
            Formatter::add((float)(millis() - _init_stamp) / 1000);
            Formatter::add(_service);
            Formatter::add(_t);
            Serial.println(Formatter::format("[[0]][[1]] [2]"));
          }
          Log() {}
        public:
          template<typename T> static void info(const T&_t) {
            echo('I',_t);
          }
          template<typename T> static void error(const T&_t) {
            echo('E',_t);
          }
          template<typename T> static void debug(const T&_t) {
            echo('D',_t);
          }
      };
      const long& Log::_init_stamp = millis();
    }
    namespace board {
      static const string rst_reasons[] = {"default", "wdt", "exception", "soft wdt", "soft restart", "deep sleep awake", "ext sys"},
                          boot_modes[]  = {"enhance", "normal"},
                          flash_size_map[] = {"4/256/256","2","8/512/512","16/512/512","32/512/512","16/1024/1024","32/1024/1024"};      
      static void printInfo() {
        using namespace utils;
        Formatter::add(system_get_time());
        Log::debug(Formatter::format("system time: [0]"));
        Formatter::add(rst_reasons[system_get_rst_info()->reason]);
        Log::debug(Formatter::format("system reset reason: [0]"));     
        Formatter::add(system_get_free_heap_size());
        Log::debug(Formatter::format("system free heap size: [0]"));
        Formatter::add(system_get_os_print());
        system_set_os_print(1);
        Formatter::add(system_get_os_print());
        Log::debug(Formatter::format("system os print: [0] / [1]"));
        Log::debug("meminfo:");
        system_print_meminfo();
        Formatter::add(string(system_get_chip_id(),HEX));
        Log::debug(Formatter::format("system chip id: 0x[0]"));
        Formatter::add(system_get_sdk_version());
        Log::debug(Formatter::format("sdk version: [0]"));
        Formatter::add(system_get_boot_version());
        Log::debug(Formatter::format("boot version: [0]"));
        Formatter::add(string(system_get_userbin_addr(),HEX));
        Log::debug(Formatter::format("userbin addr: 0x[0]"));
        Formatter::add(boot_modes[system_get_boot_mode()]);
        Log::debug(Formatter::format("boot mode: [0]"));
        Formatter::add(system_get_cpu_freq());
        Log::debug(Formatter::format("cpu freq: [0]"));
        Formatter::add(flash_size_map[system_get_flash_size_map()]);
        Log::debug(Formatter::format("system flash size map: [0]"));        
      }      
      enum pin_type_t {
        ANALOG,
        DIGITAL,
        PROGRAM
      };
      enum pin_interface_t {
        HSPI_CS,
        HSPI_CLK,
        HSPI_HD,
        HSPI_MISO,
        HSPI_MOSI,
        HSPI_WP,
        SPI_CS,
        SPI_CLK,
        SPI_HD,
        SPI_MISO,
        SPI_MOSI,
        SPI_WP,
        URX,
        UTX,
        UCTS,
        URTS        
      };
      enum pin_function_t {
        SDIO_CLK,
        SDIO_DATA,
        SDIO_CMD,
        MTCK,
        MTDI,
        MTDO,
        MTMS,
        TOUT,
        XPD_DCDC        
      };
      class Pin {
        private:
          pin_type_t _type;
          uint8_t _mode, _def_mode;
          int _value;
          vector<pin_function_t> _funcs;
          vector<pin_interface_t> _ints;
          bool _pwm, _lock;
        public:
          Pin(const pin_type_t&_type,
              const vector<pin_function_t>&_funcs,
              const vector<pin_interface_t>&_ints,
              const bool&_pwm,
              const uint8_t&_def_mode) {
                this->_type = _type;
                this->_pwm = _pwm;
                this->_def_mode = _def_mode;
                for(int i=0;i<_funcs.size();i++)
                  this->_funcs.push_back(_funcs[i]);
                for(int i=0;i<_ints.size();i++)
                  this->_ints.push_back(_ints[i]);
                reset();
              }
          void reset() {
            _mode = _def_mode;
            _value = 0;
          }
          const pin_type_t type() {
            return _type;
          }
          const uint8_t mode() {
            return _mode;
          }
          void mode(const uint8_t&_mode) {
            if(!_lock) this->_mode = _mode;
          }
          const int value() {
            return _value;
          }
          void value(const int&_value) {
            this->_value = _value;
          }
          const bool pwm() {
            return _pwm;
          }
          const bool lock() {
            return _lock;
          }
          void lock(const bool&_lock) {
            this->_lock = _lock;
          }
          const bool hasInterface(const pin_interface_t&_int) {
            for(int i=0;i<_ints.size();i++)
              if(_ints[i] == _int) return true;
            return false;
          }
          const bool hasFunction(const pin_function_t&_func) {
            for(int i=0;i<_funcs.size();i++)
              if(_funcs[i] == _func) return true;
            return false;
          }
      };
      enum board_variant_t {
        NODEMCU_ESP12E,
        WEMOS_D1_R2_MINI
      };
      static const string str(const board_variant_t&_var) {
        switch(_var) {
          case NODEMCU_ESP12E: return "NodeMCU ESP-12E";
          case WEMOS_D1_R2_MINI: return "WeMos D1 R2 & mini";
        }
        return "unknown";
      }
      class Board {
      };
    }
    namespace led {}
  }
  namespace net {
    static const float net_version  = 0.00;
  }
  namespace extra {
    static const float extra_version  = 0.00;
  }
  using namespace project;
  using namespace core;
  using namespace net;
  using namespace extra;
#endif //ESP8266
void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println("Board starting");
  #ifdef ESP8266
    board::printInfo();
    using namespace utils;
    Formatter::add(project_name);
    Formatter::add(core_version);
    Formatter::add(net_version);
    Formatter::add(extra_version);
    Formatter::add(str(project_stage));
    Serial.println(Formatter::format("\n[0] version [1]c[2]n[3]e-[4]\n"));
  #else
    Serial.println("Sorry, but this sketch only for ESP8266 boards!");
  #endif //ESP8266
}
void loop() {
  #ifdef ESP8266
  #endif //ESP8266
}
