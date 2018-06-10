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
    static const float version  = 0.71;
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
            utils::Log::info(utils::Formatter::format("Led started on GPIO[0]"));
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
    namespace sensors {
      static const long SENSORS_UPDATE_DELAY = 1000;
      enum sensor_t {
        NONE,
        TEMPERATURE,
        PRESSURE
      };
      static const string sensor_t2str(const sensor_t&_type) {
        switch(_type) {
          case TEMPERATURE: return "Temperature";
          case PRESSURE: return "Pressure";
        }
        return "None";
      }
      enum wire_t {
        ONE_WIRE,
        TWO_WIRE
      };
      class Sensor {
        private:
          std::vector<uint8_t> _addr;
          float _value = -1;
        public:
          Sensor() {}
          Sensor(const std::vector<uint8_t>&_addr) {
            for(int i=0;i<_addr.size();i++)
              this->_addr.push_back(_addr[i]);
          }
          uint8_t chipId() {
            return _addr.size()>=1 ? _addr[0] : 0xff;
          }
          float value() {
            return _value;
          }
          void value(const float&_value) {
            this->_value = _value;
          }
          size_t addrSize() {
            return _addr.size();
          }
          uint8_t addr(const int&_id) {
            return _id>=0 && _id<_addr.size() ? _addr[_id] : 0xff;
          }
          string addr2str() {
            string out = "";
            for(int i=0;i<_addr.size();i++) {
              if(i!=0) out += ':';
              if(_addr[i]<10) out += '0';
              out += string(_addr[i],HEX);
            }
            return out;
          }          
      };
      class SensorMeta {
        private:
          sensor_t _type;
          string _display;
        public:
          SensorMeta() : _type(NONE), _display("Unknown"){}
          SensorMeta(const sensor_t&_type, const string&_display) : _type(_type),_display(_display) {
          }
          sensor_t type() {
            return _type;
          }
          string desc() {
            return _display;
          }
      };
      class SensorObject {
        private:
          std::map<uint8_t, SensorMeta> _devs;
          string _vendor;
        protected:
          void add(const uint8_t&_cid, const sensor_t&_type, const string&_display) {
            if(check(_cid)) return;
            _devs.insert(std::pair<uint8_t,SensorMeta>(_cid,SensorMeta(_type,_display)));            
          }
        public:
          SensorObject(const string&_vendor): _vendor(_vendor){}
          const string vendor() {
            return _vendor;  
          }
          bool check(const uint8_t&_cid) {
            return _devs.find(_cid)!=_devs.end();
          }
          const SensorMeta meta(const uint8_t&_cid) {
            return check(_cid) ? _devs[_cid] : SensorMeta();
          }
          virtual void recieve(OneWire&_1wire, Sensor&_s){}
          virtual void recieve(TwoWire&_2wire, Sensor&_s){}
      };
      class SensorWire {
        private:
          const uint8_t _id;
          const wire_t _type;
          OneWire* _1wire;
          TwoWire* _2wire;
          std::vector<Sensor> _sensors;
          bool _begin_reason = false;
        public:
          SensorWire(const uint8_t&_id, const uint8_t&_gpio0) : _id(_id), _type(ONE_WIRE) {
            _1wire = new OneWire(_gpio0);
          }
          SensorWire(const uint8_t&_id, const uint8_t&_gpio0, const uint8_t&_gpio1) : _id(_id), _type(TWO_WIRE){
            _2wire = new TwoWire();
            _2wire->begin(_gpio0,_gpio1);
          }
          const uint8_t id() {
            return _id;
          }
          const wire_t type() {
            return _type;
          }
          const size_t size() {
            return _sensors.size();
          }
          Sensor sensor(const int&_id) {
            return _id>=0 && _id<_sensors.size() ? _sensors[_id] : Sensor();
          }
          void begin() {
            if(_begin_reason) return;
            std::vector<uint8_t> _addr_;
            switch(_type) {
              case ONE_WIRE:
                  _1wire->reset_search();
                  uint8_t _addr[8];                  
                  while(_1wire->search(_addr)) {
                    for(int i=0;i<8;i++)
                      _addr_.push_back(_addr[i]);
                    _sensors.push_back(Sensor(_addr_));
                    _addr_.clear();
                  }
                  _1wire->reset_search();
                break;
              case TWO_WIRE:
                for(uint8_t i=0x0;i<0xff;i++) {
                  _2wire->beginTransmission(i);
                  _2wire->write(0);
                  _2wire->endTransmission();
                  _2wire->requestFrom(i,1);
                  if(_2wire->available()>0){
                    _addr_.push_back(i);
                    _sensors.push_back(Sensor(_addr_));
                    _addr_.clear();
                  }
                }
            }
            _begin_reason = true;
          }
          void end () {
            if(!_begin_reason) return;
            _sensors.clear();
            _begin_reason = false;
          } 
          void update(const int&_id, SensorObject&_obj) {
            if(!_begin_reason || _id<0 || _id>=_sensors.size() || !_obj.check(_sensors[_id].chipId())) return;
            switch(_type) {
              case ONE_WIRE: _obj.recieve(*_1wire, _sensors[_id]); break;
              case TWO_WIRE: _obj.recieve(*_2wire, _sensors[_id]); break;
            }                        
          }      
      };
      class Sensors {
        private:
          static std::map<uint8_t, SensorWire> _wires;          
          static std::map<wire_t, std::vector<SensorObject*>> _objs;
          static bool _begin_reason;
          static long _next_update;
          Sensors(){}
        public:
          static void add(const uint8_t&_gpio0) {
            if(_begin_reason || _wires.find(_gpio0)!=_wires.end()) return;
            _wires.insert(std::pair<uint8_t,SensorWire>(_gpio0, SensorWire(_wires.size(),_gpio0)));
            utils::Formatter::add(_gpio0);
            utils::Log::info(utils::Formatter::format("SensorWire (OneWire: GPIO[0]) added"));
          }
          static void add(const uint8_t&_gpio0, const uint8_t&_gpio1) {
            if(_begin_reason || _wires.find(_gpio0)!=_wires.end() || _wires.find(_gpio1)!=_wires.end() || _wires.find(_gpio0*10+_gpio1)!=_wires.end()) return;
            _wires.insert(std::pair<uint8_t,SensorWire>(_gpio0*10+_gpio1,SensorWire(_wires.size(),_gpio0, _gpio1)));
            utils::Formatter::add(_gpio0);
            utils::Formatter::add(_gpio1);
            utils::Log::info(utils::Formatter::format("SensorWire (TwoWire: SDA=GPIO[0], SCL=GPIO[1]) added"));
          }
          static void add(const wire_t&_type, SensorObject&_obj) {
            if(_begin_reason || _objs.find(_type)!=_objs.end()) return;
            std::vector<SensorObject*> _db = _objs[_type];
            for(int i=0;i<_db.size();i++)
              if(_db[i]->vendor() == _obj.vendor()) return;
            _db.push_back(&_obj);
            _objs[_type] = _db;
          }
          static void begin() {
            if(_begin_reason || _wires.size()<1) return;
            utils::Log::info("Searching sensors on wires started");
            for(std::map<uint8_t,SensorWire>::iterator cur=_wires.begin(); cur!=_wires.end(); cur++) {
              cur->second.begin();
              for(int i=0;i<cur->second.size();i++) {
                utils::Formatter::add(cur->second.id());
                string k = "";
                if(cur->first<10) k += '0';
                k += string(cur->first,HEX);
                utils::Formatter::add(k);
                utils::Formatter::add(i);
                utils::Formatter::add(cur->second.sensor(i).addr2str());
                utils::Formatter::add(string(cur->second.sensor(i).chipId(),HEX));
                string meta = "Unknown";
                if(_objs.find(cur->second.type())!=_objs.end()) {
                  for(int j=0;j<_objs[cur->second.type()].size();j++) 
                    if(_objs[cur->second.type()][j]->check(cur->second.sensor(i).chipId())){
                      meta = _objs[cur->second.type()][j]->vendor();
                      meta += ' ';
                      SensorMeta sm = _objs[cur->second.type()][j]->meta(cur->second.sensor(i).chipId());
                      meta += sm.desc();
                      meta += ' ';
                      meta += sensor_t2str(sm.type());
                      break;
                    }                  
                }
                utils::Formatter::add(meta);
                utils::Log::debug(utils::Formatter::format("Found on wire [0] (0x[1]): sensor [2] ([3]) as (0x[4], [5])"));
              }
            }
            utils::Log::info("Searching sensors on wires finished");
            _begin_reason = true;
          }
          static void update() {
            if(!_begin_reason || millis()<_next_update) return;
            _next_update = millis() + SENSORS_UPDATE_DELAY;
            utils::Log::info("Updating sensors values started");
            for(std::map<uint8_t,SensorWire>::iterator cur=_wires.begin(); cur!=_wires.end(); cur++) {
              for(int i=0;i<cur->second.size();i++) {
                if(_objs.find(cur->second.type())!=_objs.end()) {
                  for(int j=0;j<_objs[cur->second.type()].size();j++)
                    if(_objs[cur->second.type()][j]->check(cur->second.sensor(i).chipId())) {
                      cur->second.update(i,*_objs[cur->second.type()][j]);
                      utils::Formatter::add(cur->second.id());
                      utils::Formatter::add(i);
                      utils::Formatter::add(cur->second.sensor(i).addr2str());
                      utils::Formatter::add(cur->second.sensor(i).value());
                      utils::Log::debug(utils::Formatter::format("Recieved value from wire [0] sensor [1] ([2]): [3]"));
                    }                   
                }
              }
            }
            utils::Log::info("Updating sensors values finished");
          } 
      };
      std::map<uint8_t,SensorWire> Sensors::_wires;
      std::map<wire_t, std::vector<SensorObject*>> Sensors::_objs;
      bool Sensors::_begin_reason = false;
      long Sensors::_next_update;
    }
  }
  namespace net {
    static const float version  = 0.00;
  }
  namespace extra {
    static const float version  = 0.02;
    namespace sensors {    
      class DallasSensorObject : public core::sensors::SensorObject {
        public:
          DallasSensorObject() : core::sensors::SensorObject("Dallas") {
            add(0x10, core::sensors::TEMPERATURE,"DS18S20");
            add(0x22, core::sensors::TEMPERATURE,"DS1822");
            add(0x28, core::sensors::TEMPERATURE,"DS18B20");
          }
          virtual void recieve(OneWire&_1wire, core::sensors::Sensor&_s) {            
            uint8_t addr[8];
            for(int i=0;i<8;i++)
              addr[i] = _s.addr(i);                         
            _1wire.reset();
            _1wire.select(addr);
            _1wire.write(0x44,true);  
            _1wire.reset();
            uint8_t data[12];
            _1wire.select(addr);
            _1wire.write(0xBE, false);
            for(int i=0;i<9;i++)
              data[i] = _1wire.read();
            uint16_t raw = (data[1] << 8) | data[0];
            if(_s.chipId()==0x10) {
              raw = raw << 3;
              if(data[7] == 0x10)
                raw = (raw & 0xFFF0) + 12 - data[6];
            } else {
              uint8_t cfg = (data[4] & 0x60);
              if(cfg == 0x0) raw = raw & ~7;
              else if(cfg == 0x20) raw = raw & ~3;
              else if(cfg == 0x40) raw = raw & ~1;
            }
           _s.value(raw/16.0);
          }
      };
      class AdafruitSensorObject : public core::sensors::SensorObject {
        public:
          AdafruitSensorObject() : core::sensors::SensorObject("Adafruit") {
            add(0x76, core::sensors::PRESSURE,"BMP280");
            add(0xf6, core::sensors::TEMPERATURE, "BMP280");
          }
          virtual void recieve(TwoWire&_2wire, core::sensors::Sensor&_s) {}
      };
    }    
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
    core::sensors::Sensors::add(core::sensors::ONE_WIRE,*new extra::sensors::DallasSensorObject());
    core::sensors::Sensors::add(core::sensors::TWO_WIRE,*new extra::sensors::AdafruitSensorObject());
    core::sensors::Sensors::add(D1);
    core::sensors::Sensors::add(D2,D3);
    core::sensors::Sensors::begin();
  #else
    Serial.println("Sorry, but this sketch only for ESP8266 boards!");
  #endif //ESP8266
}
void loop() {
  #ifdef ESP8266
    core::led::Led::pulse();
    core::sensors::Sensors::update();
  #endif //ESP8266
}
