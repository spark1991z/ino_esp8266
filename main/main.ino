#ifdef ESP8266
  extern "C" {
    #include "user_interface.h"
  }
  #include <vector>
  #include <map>
  #include <Wire.h>
  #include <OneWire.h>
  #define string String
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
    static const string name    = "ASAIOP";
    static const stage_t stage  = STAGE_SKELETON;

  }
  namespace core {
    static const float version  = 0.45;
    namespace utils {
      class Utils {
        private:
          Utils() {}
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
          static const std::vector<string> split(const string&_source, const char&_delim) {
            std::vector<string> out;
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
          static std::vector<string> _values;
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
      std::vector<string> Formatter::_values;
      bool Formatter::_lock;
      class Log {
        private:
          static const long _init_stamp;
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
      const long Log::_init_stamp = millis();
    }
    namespace board {
      static const string rst_reasons[] = {"default", "wdt", "exception", "soft wdt", "soft restart", "deep sleep awake", "ext sys"},
                          boot_modes[]  = {"enhance", "normal"},
                          flash_size_map[] = {"4/256/256","2","8/512/512","16/512/512","32/512/512","16/1024/1024","32/1024/1024"};      
      static void printInfo() {
        utils::Formatter::add(system_get_time());
        utils::Log::debug(utils::Formatter::format("system time: [0]"));
        utils::Formatter::add(rst_reasons[system_get_rst_info()->reason]);
        utils::Log::debug(utils::Formatter::format("system reset reason: [0]"));     
        utils::Formatter::add(system_get_free_heap_size());
        utils::Log::debug(utils::Formatter::format("system free heap size: [0]"));
        utils::Formatter::add(system_get_os_print());
        system_set_os_print(1);
        utils::Formatter::add(system_get_os_print());
        utils::Log::debug(utils::Formatter::format("system os print: [0] / [1]"));
        utils::Log::debug("meminfo:");
        system_print_meminfo();
        utils::Formatter::add(string(system_get_chip_id(),HEX));
        utils::Log::debug(utils::Formatter::format("system chip id: 0x[0]"));
        utils::Formatter::add(system_get_sdk_version());
        utils::Log::debug(utils::Formatter::format("sdk version: [0]"));
        utils::Formatter::add(system_get_boot_version());
        utils::Log::debug(utils::Formatter::format("boot version: [0]"));
        utils::Formatter::add(string(system_get_userbin_addr(),HEX));
        utils::Log::debug(utils::Formatter::format("userbin addr: 0x[0]"));
        utils::Formatter::add(boot_modes[system_get_boot_mode()]);
        utils::Log::debug(utils::Formatter::format("boot mode: [0]"));
        utils::Formatter::add(system_get_cpu_freq());
        utils::Log::debug(utils::Formatter::format("cpu freq: [0]"));
        utils::Formatter::add(flash_size_map[system_get_flash_size_map()]);
        utils::Log::debug(utils::Formatter::format("system flash size map: [0]"));        
      }      
    }
    namespace led {
      enum mode_t {
        MODE_DISABLED,
        MODE_HIGH,
        MODE_LOW,
        MODE_BLINK  
      };
      static const long LED_PULSE_DELAY = 1000;
      class Led {
        private:
          static mode_t _mode;
          static uint8_t _gpio;
          static int _pulse_count;
          static bool _begin_reason, _led_on;
          static long _pulse_delay, _next_pulse;
          Led() {}
        public:
          static void begin(const uint8_t&_gpio) {
            if(_begin_reason) return;
            Led::_gpio = _gpio;
            pinMode(_gpio,OUTPUT);
            _mode = MODE_DISABLED;
            _next_pulse = millis();
            _pulse_delay = LED_PULSE_DELAY;
            _pulse_count = 0;
            _begin_reason = true;
            utils::Formatter::add(_gpio);
            utils::Log::info(utils::Formatter::format("Led started on pin '[0]'"));
          }
          static void blink(const long&_pulse_delay) {
            if(!_begin_reason) return;
            _mode = MODE_BLINK;
            _led_on = false;
            Led::_pulse_delay = _pulse_delay;
            _pulse_count = 0;
            utils::Formatter::add(_pulse_delay);
            utils::Log::debug(utils::Formatter::format("Led switched to mode 'BLINK' with delay '[0]'"));
          }
          static const int count() {
            return _pulse_count;
          }
          static void high() {
            if(!_begin_reason) return;
            _mode = MODE_HIGH;
            _led_on = true;
            _pulse_count = 0;
            _pulse_delay = LED_PULSE_DELAY;
            utils::Formatter::add(_pulse_delay);
            utils::Log::debug(utils::Formatter::format("Led switched to mode 'HIGH' with delay '[0]'"));
          }
          static void low() {
            if(!_begin_reason) return;
            _mode = MODE_LOW;
            _led_on = false;
            _pulse_count = 0;
            _pulse_delay = LED_PULSE_DELAY;
            utils::Formatter::add(_pulse_delay);
            utils::Log::debug(utils::Formatter::format("Led switched to mode 'LOW' with delay '[0]'"));
          }
          static void pulse() {
            if(!_begin_reason || millis() < _next_pulse) return;
            _next_pulse += _pulse_delay / (_mode==MODE_BLINK ? 2 : 1);
            _led_on = _mode==MODE_BLINK ? !_led_on : _led_on;
            digitalWrite(_gpio,_led_on);
            if(_mode != MODE_BLINK || (_mode==MODE_BLINK && _led_on))
              _pulse_count ++;
          }
      };
      mode_t Led::_mode;
      uint8_t Led::_gpio;
      int Led::_pulse_count;
      bool Led::_begin_reason = false, Led::_led_on;
      long Led::_pulse_delay, Led::_next_pulse;
    }
    namespace sensor {
      enum type_t {
        SENSOR_WIRE,
        SENSOR_ONEWIRE
      };
      enum state_t {
        STATE_DISABLED,
        STATE_WAITING,
        STATE_PROCESSING
      };
      class Sensor {
        private:
          type_t _type;
          std::vector<uint8_t> _addr;
        public:
          Sensor(const type_t&_type, const std::vector<uint8_t>&_addr) {
            this->_type = _type;
            for(int i=0;i<_addr.size();i++)
              this->_addr.push_back(_addr[i]);
          }
          const string addrStr() {
            string out;
            for(int i=0;i<_addr.size();i++) {
              if(i!=0) out += ':';
              if(_addr[i] < 10) out += '0';
              out += string(_addr[i],HEX);
            }
            return out;
          }
          const size_t addrSize() {
            return _addr.size();
          }
          const uint8_t addr(const int&_idx) {
            return _idx>=0 && _idx < _addr.size() ? _addr[_idx] : 0xFF;
          }
          const uint8_t chipId() {
            return _addr[0];
          }
      };
      static const long SENSOR_UPDATE_DELAY = 5000;
      class SensorManager {
        private:
          static OneWire* _onewire;
          static std::vector<Sensor> _sensors;
          static int _i2c_idx, _spi_idx, _gpio_wire_sda, _gpio_wire_scl, _gpio_onewire;
          static long _next_update;
          static state_t _state;
          static bool _begin_reason, _init_i2c, _init_spi;
          SensorManager() {}
        public:
          static void wireI2C(const int&_gpio_wire_sda, const int&_gpio_wire_scl) {
            if(_begin_reason) return;
            SensorManager::_gpio_wire_sda = _gpio_wire_sda;
            SensorManager::_gpio_wire_scl = _gpio_wire_scl;
          }
          static void wireSPI(const int&_gpio_onewire) {
            if(_begin_reason) return;
            SensorManager::_gpio_onewire = _gpio_onewire;
          }
          static void detect() {
            if(!_begin_reason || _state!= STATE_WAITING || millis() < _next_update) return;
            utils::Log::debug("Detecting sensors started");
            _state = STATE_PROCESSING;
            _sensors.clear();
            std::vector<uint8_t> _addr;
            //Wire detecting
            int _cnt = 0;
            if(_init_i2c) for(uint8_t i = 0x01; i<0xff; i++ ) {
              Wire.beginTransmission(i);
              if(Wire.available()) {
                _addr.push_back(i);
                utils::Formatter::add(_cnt);
                _sensors.push_back(Sensor(SENSOR_WIRE,_addr));
                utils::Formatter::add(string(i,HEX));
                utils::Log::debug(utils::Formatter::format("Wire sensor [0]: 0x[1]"));
                _addr.clear();
                _cnt ++;
              }
              Wire.endTransmission();
            }
            _i2c_idx = _sensors.size()>0 ? 0 : -1;
            _spi_idx = _sensors.size();
            //OneWire detecting
            _cnt = 0;
            if(_init_spi) {
              uint8_t _addr2[8];
              _onewire->reset_search();
              while(_onewire->search(_addr2)) {
                for(int i=0;i<8;i++)
                  _addr.push_back(_addr2[i]);
                utils::Formatter::add(_cnt);
                Sensor s(SENSOR_ONEWIRE,_addr);
                _sensors.push_back(s);
                utils::Formatter::add(s.addrStr());
                utils::Log::debug(utils::Formatter::format("OneWire sensor [0]: [1]"));
                _addr.clear();
                _cnt ++;
              }
            }
            utils::Formatter::add(_sensors.size());
            utils::Log::debug(utils::Formatter::format("Total detected sensors: [0]"));
            utils::Log::debug("Detecting sensors finished");
            _next_update = millis() + SENSOR_UPDATE_DELAY;
            _state = STATE_WAITING;
          }
          static void begin() {
            if(_begin_reason) return;
            if(_gpio_wire_sda!=-1 && _gpio_wire_scl!=_gpio_wire_sda && _gpio_wire_scl!=-1) {
              Wire.begin(_gpio_wire_sda, _gpio_wire_scl);
              utils::Formatter::add(_gpio_wire_sda);
              utils::Formatter::add(_gpio_wire_scl);
              utils::Log::info(utils::Formatter::format("Wire started on pins: (SDA: '[0]', SCL '[1]')"));
              _init_i2c = true;
            }          
            if(_gpio_onewire!=-1 && _gpio_onewire!=_gpio_wire_sda && _gpio_onewire!=_gpio_wire_scl) {
              _onewire = new OneWire(_gpio_onewire);
              utils::Formatter::add(_gpio_onewire);
              utils::Log::info(utils::Formatter::format("OneWire started on pin: '[0]'"));
              _init_spi = true;
            }
            if(!_init_i2c && !_init_spi) {
              utils::Log::error("Unable to begin SensorManager - no pins I2C/SPI");
              return;
            }
            _begin_reason = true;
            _state = STATE_WAITING;
            _next_update = millis();
            detect();     
          }
      };
      OneWire* SensorManager::_onewire;
      std::vector<Sensor> SensorManager::_sensors;
      int SensorManager::_i2c_idx, SensorManager::_spi_idx, SensorManager::_gpio_wire_sda=-1, SensorManager::_gpio_wire_scl=-1, SensorManager::_gpio_onewire=-1;
      long SensorManager::_next_update;
      state_t SensorManager::_state = STATE_DISABLED;
      bool SensorManager::_begin_reason = false, SensorManager::_init_i2c = false, SensorManager::_init_spi = false;
    }
  }
  namespace net {
    static const float version  = 0.00;
  }
  namespace extra {
    static const float version  = 0.00;
  }
#endif //ESP8266
void setup() {
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  Serial.println();
  Serial.println("Board starting");
  #ifdef ESP8266
    core::board::printInfo();
    core::utils::Formatter::add("----------------------------------------");
    core::utils::Formatter::add(project::name);
    core::utils::Formatter::add(core::version);
    core::utils::Formatter::add(net::version);
    core::utils::Formatter::add(extra::version);
    core::utils::Formatter::add(project::str(project::stage)); 
    Serial.println(core::utils::Formatter::format("\n[0]\n[1] version: [2]c[3]n[4]e-[5]\n[0]\n"));
    core::led::Led::begin(D4);
    core::led::Led::blink(1000);
    core::sensor::SensorManager::wireSPI(D1);
    core::sensor::SensorManager::wireI2C(D2,D3);
    core::sensor::SensorManager::begin();
  #else
    Serial.println("Sorry, but this sketch only for ESP8266 boards!");
  #endif //ESP8266
}
void loop() {
  #ifdef ESP8266
    core::led::Led::pulse();
  #endif //ESP8266
}
