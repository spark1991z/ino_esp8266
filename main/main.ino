#include <vector>
#include <map>
#include <Wire.h>
#include <OneWire.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
extern "C" {
  #include "user_interface.h"
}
#endif
#define string String
namespace project {
  static const string PROJECT_STAGE_NAMES[]{"p","s","pa","a","pb","b","pr","r","f","eop"};
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
  static const string _name = "ALCOPYTHON";
  static const stage_t _stage = PRE_ALPHA;
  static const uint8_t _release = 6;
}
namespace core {
  static const uint8_t _version[] = {0,125};
  class Formatter {
    private:
      static std::vector<string> _values;
    public:
      static void reset() {
        _values.clear();
      }
      template <typename T> static void add(const T&_t) {
        _values.push_back(string(_t));
      }
      static const string format(const string&_pattern) {
        if(_values.size()<1) return _pattern;
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
  class Log {
    private:
      static const long _init_stamp;
      Log() {}
      template <typename T> static void echo(const char&_service, const T&_t) {
        Formatter::reset();
        Formatter::add((float)(millis() - _init_stamp) / 1000);
        Formatter::add(_service);
        Formatter::add(_t);
        Serial.println(Formatter::format("[[0]][[1]]\t[2]"));
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
  namespace sensors {
    static const long SENSORS_UPDATE_DELAY = 1000;
    static const float SENSOR_VALUE_DEFAULT = -1,
                       PRESSURE_PA2HPA = 100,
                       PRESSURE_PA2INHG = 3386.3752577878,
                       PRESSURE_PA2BAR = 100000,
                       PRESSURE_PA2TORR = 133.32236534674,
                       PRESSURE_PA2PSI = 6894.744825494;
    static const string SENSOR_VENDOR_MODEL_UNKNOWN = "Unknown";
    static const uint8_t SENSOR_2WIRE_REGISTER_CHIPID = 0xD0;
    static const string SENSOR_UNIT_NAMES[]{"'C","'F","Pa","hPa","inHg","bar","torr","psi"};                         
    enum sensor_unit_t {
      CELSIUS,
      FAHRENHEIT,
      PA,
      HPA,
      INHG,
      BAR,
      TORR,
      PSI
    };
    static const float c2f(const float&_c) {
      return _c*1.8+32;
    }
    static const float f2c(const float&_f) {
      return (_f-32)/1.8;
    }
    static const float pa2hpa(const float&_pa) {
      return _pa/PRESSURE_PA2HPA;
    }
    static const float pa2inhg(const float&_pa) {
      return _pa/PRESSURE_PA2INHG;
    }
    static const float pa2bar(const float&_pa) {
      return _pa/PRESSURE_PA2BAR;
    }
    static const float pa2torr(const float&_pa) {
      return _pa/PRESSURE_PA2TORR;
    }
    static const float pa2psi(const float&_pa) {
      return _pa/PRESSURE_PA2PSI;
    }
    enum wire_t {
      ONE_WIRE,
      TWO_WIRE
    };
    enum sensor_state_t {
      REQUEST_REQUIRED,
      PROCESSING
    };
    class SensorData {
      public:
        sensor_unit_t _unit;
        float _value;
        SensorData(const sensor_unit_t&_unit) : _unit(_unit), _value(SENSOR_VALUE_DEFAULT){}
    };
    class Sensor {
      private:
        uint8_t _chipId;
        std::vector<uint8_t> _addr;
        std::vector<SensorData> _data;
        long _next_request;
        sensor_state_t _state;
      public:
        Sensor(const uint8_t&_chipId) : _chipId(_chipId) {}
        Sensor(const uint8_t&_chipId, const std::vector<uint8_t>&_addr) :_chipId(_chipId), _next_request(millis()), _state(REQUEST_REQUIRED) { 
          for(uint8_t i=0;i<_addr.size();i++)
            this->_addr.push_back(_addr[i]);
        }
        const uint8_t chipId() {
          return _chipId;
        }
        const std::vector<uint8_t> addr() {
          return _addr;
        }
        const string addrStr() {
          string out = "";
          for(int i=0;i<_addr.size();i++) {
            if(i!=0) out += ':';
            if(_addr[i]<16) out += '0';
            out += string(_addr[i],HEX);
          }
          return out;
        }
        const std::vector<SensorData> data() {
          return _data;
        }
        void add(const SensorData&_d) {
          for(uint8_t i=0;i<_data.size();i++)
            if(_data[i]._unit == _d._unit) return;
          _data.push_back(_d); 
        }
        void update(const uint8_t&_id, const float&_value) {
          if(_id>=0 && _id<_data.size()) _data[_id]._value = _value;
        }
        const long nextRequest() {
          return _next_request;
        }
        void nextRequest(const uint16_t&_delay) {
          _next_request = millis() + _delay;
        }
        const sensor_state_t state() {
          return _state;
        }
        void state(const sensor_state_t&_state) {
          this->_state = _state;
        }        
    };
    class SensorObject {
      private:
        std::map<uint8_t,string> _devices;
        string _vendor;
      protected:
        void add(const uint8_t&_chipId, const string&_model) {
          if(_devices.find(_chipId)!=_devices.end()) return;
          _devices.insert(std::pair<uint8_t,string>(_chipId,_model));
        }
      public:
        SensorObject(const string&_vendor) : _vendor(_vendor){}
        const bool check(const uint8_t&_chipId) {
          return _devices.find(_chipId)!=_devices.end();
        }
        const string vendor() {
          return _vendor;
        }
        const string model(const uint8_t&_chipId) {
          return check(_chipId) ? _devices[_chipId] : SENSOR_VENDOR_MODEL_UNKNOWN;
        }
        virtual void recieve(OneWire&_1wire, Sensor&_sensor) {}
        virtual void recieve(TwoWire&_2wire, Sensor&_sensor) {}
    };
    class SensorWire {
      private:
        wire_t _type;
        OneWire* _1wire;
        TwoWire* _2wire;
        std::vector<Sensor> _sensors;
      public:
        SensorWire(const uint8_t&_gpio0) : _type(ONE_WIRE) {
          _1wire = new OneWire(_gpio0);
        }
        SensorWire(const uint8_t&_gpio0, const uint8_t&_gpio1) : _type(TWO_WIRE) {
          _2wire = new TwoWire();
          _2wire->begin(_gpio0,_gpio1);
        }
        void detect() {
          _sensors.clear();
          std::vector<uint8_t> _addr;
          switch(_type) {
            case ONE_WIRE:
              uint8_t addr[8];
              while(_1wire->search(addr)){
                for(uint8_t i=0;i<8;i++)
                  _addr.push_back(addr[i]);
                _sensors.push_back(Sensor(addr[0],_addr));
                _addr.clear();
              }
              _1wire->reset_search();
              break;
            case TWO_WIRE:
              for(uint8_t i=0x0; i<0xff; i++){
                _2wire->beginTransmission(i);
                _2wire->write(SENSOR_2WIRE_REGISTER_CHIPID);
                uint8_t err = _2wire->endTransmission();
                _2wire->requestFrom(i,1);
                uint8_t chipId = _2wire->read();                
                if(err == 0 && chipId!=0xff) {
                  _addr.push_back(i);
                  _sensors.push_back(Sensor(chipId,_addr));
                  _addr.clear();
                }
              }                          
              break;
          }
        }
        const bool update(const uint8_t&_id, SensorObject*_obj) {
          if(_id>=0 && _id<_sensors.size() && _sensors[_id].nextRequest() < millis()) {
            Sensor s = _sensors[_id];
            switch(_type) {
              case ONE_WIRE:
                _obj->recieve(*_1wire,s);
                break;
              case TWO_WIRE:
                _obj->recieve(*_2wire,s);
                break;
            }
            if(s.nextRequest()<millis()) {
              Formatter::reset();
              Formatter::add(s.addrStr());
              Formatter::add(SENSORS_UPDATE_DELAY);
              Log::error(Formatter::format("SensorWire: Loop detected on sensor [0]. Request time delay will be extended to [1]ms"));
              s.nextRequest(SENSORS_UPDATE_DELAY);
            }                         
            _sensors[_id] = s;
            return s.state()==REQUEST_REQUIRED;
          }
          return false;
        }
        const wire_t type() {
          return _type;
        }
        const uint8_t count() {
          return _sensors.size();
        }
        const std::vector<uint8_t> addr(const uint8_t&_id) {
          return _id>=0 && _id<_sensors.size() ? _sensors[_id].addr() : std::vector<uint8_t>();
        }
        const string addrStr(const uint8_t&_id) {
          return _id>=0 && _id<_sensors.size() ? _sensors[_id].addrStr() : string();
        }
        const std::vector<SensorData> data(const uint8_t&_id) {
          return _id>=0 && _id<_sensors.size() ? _sensors[_id].data() : std::vector<SensorData>();
        }
        const bool check(const uint8_t&_id, SensorObject*_obj) {
          return _id>=0 && _id<_sensors.size() && _obj->check(_sensors[_id].chipId());
        }
        const uint8_t chipId(const uint8_t&_id) {
          return _id>=0 && _id<_sensors.size() ? _sensors[_id].chipId() : 0xff;
        }
    };
    class Sensors {
      private:
        static std::map<uint8_t,SensorWire> _wires;
        static std::map<wire_t,std::vector<SensorObject*>> _objects;
        static bool _begin_reason;
        Sensors() {}
      public:
        static void add(const uint8_t&_gpio0) {
          if(_begin_reason ||
            _wires.find(_gpio0*10)!=_wires.end()) return;
          _wires.insert(std::pair<uint8_t,SensorWire>(_gpio0*10,SensorWire(_gpio0)));
        }
        static void add(const uint8_t&_gpio0, const uint8_t&_gpio1) {
          if(_begin_reason ||
            _wires.find(_gpio0*10)!=_wires.end() ||
            _wires.find(_gpio1*10)!=_wires.end() ||
            _wires.find(_gpio0*10+_gpio1)!=_wires.end() ||
            _wires.find(_gpio1*10+_gpio0)!=_wires.end()) return;          
          _wires.insert(std::pair<uint8_t,SensorWire>(_gpio0*10+_gpio1,SensorWire(_gpio0,_gpio1)));
        }
        static void add(const wire_t&_type, SensorObject*_obj) {
          if(_begin_reason) return;
          for(int i=0;i<_objects[_type].size();i++)
            if(_objects[_type][i]->vendor() == _obj->vendor()) return;
          _objects[_type].push_back(_obj);
        }
        static const std::vector<uint8_t> wireIDs() {
          std::vector<uint8_t> result;
          for(std::map<uint8_t,SensorWire>::iterator cur=_wires.begin();cur!=_wires.end();cur++)
            result.push_back(cur->first);
          return result;
        }       
        static void begin() {
          if(_begin_reason || _wires.size()<1) return;
          for(std::map<uint8_t,SensorWire>::iterator cur = _wires.begin(); cur!=_wires.end(); cur++) {
            SensorWire sw = cur->second;
            sw.detect();
            for(uint8_t i=0;i<sw.count();i++) {
              bool detect = false;
              Formatter::reset();
              Formatter::add(string(cur->first,HEX));            
              Formatter::add(i);              
              Formatter::add(sw.addrStr(i));
              Formatter::add(string(sw.chipId(i),HEX));
              string vendor = "", model = "";
              for(uint8_t j=0;j<_objects[sw.type()].size();j++) {
                if(_objects[sw.type()][j]->check(sw.chipId(i))) {
                  detect = true;                  
                  vendor = _objects[cur->second.type()][j]->vendor();
                  model = _objects[cur->second.type()][j]->model(sw.chipId(i));
                  break;
                }
              }
              Formatter::add(detect ? vendor : SENSOR_VENDOR_MODEL_UNKNOWN);
              Formatter::add(detect ? model : SENSOR_VENDOR_MODEL_UNKNOWN);
              Log::info(Formatter::format("Sensors: Found on wire(0x[0]) sensor (id=[1], addr=[2], chipId=0x[3], vendor=[4], model=[5])"));
            }
            cur->second = sw;
          }
          _begin_reason = true;
        }
        static void update() {
          if(!_begin_reason) return;
          for(std::map<uint8_t,SensorWire>::iterator cur=_wires.begin(); cur!=_wires.end(); cur++) {
            SensorWire sw = cur->second;
            for(uint8_t i=0;i<sw.count();i++) {
              for(uint8_t j=0;j<_objects[sw.type()].size();j++) {                             
                if(sw.check(i,_objects[sw.type()][j]) && sw.update(i,_objects[sw.type()][j])) {
                  const std::vector<SensorData> sd = sw.data(i);
                  Formatter::reset();
                  Formatter::add(string(cur->first,HEX));
                  Formatter::add(i);
                  Formatter::add(sd.size());
                  string val = "";
                  for(uint8_t k=0; k<sd.size();k++) {
                    if(k!=0) val += ", ";
                    val += sd[k]._value;
                    val += SENSOR_UNIT_NAMES[sd[k]._unit];
                  }
                  Formatter::add(val);
                  Log::debug(Formatter::format("Sensors: Recieved from wire(0x[0])sensor([1]) [2] values ([3])"));
                  break;
                }
              }
            }
            cur->second = sw;
          }
        }
    };
    std::map<uint8_t,SensorWire> Sensors::_wires;
    std::map<wire_t,std::vector<SensorObject*>> Sensors::_objects;
    bool Sensors::_begin_reason = false;
  }
  namespace net {
    namespace http {}
    namespace wlan {
      static const string WLAN_OPMODE_NAMES[]{"None","STA","AP","STA+AP"},
                          WLAN_AUTH_NAMES[]{"Open","WPA-PSK","WPA2-PSK","WPA/WPA2-PSK","MAX"};
      enum wlan_opmode_t {
        NONE,
        STA,
        AP,
        STA_AP
      };
      enum wlan_auth_t {
        OPEN,
        WEP,
        WPA_PSK,
        WPA2_PSK,
        WPA_WPA2_PSK,
        MAX
      };
      #ifdef ESP8266     
      class WiFiManager{
        private:
          static bool _begin_reason, _start_reason, _secure, _ready;
          static softap_config _config;
          WiFiManager(){}
        public:
          static void begin(){
            if(_begin_reason) return;
            Log::info("WiFiManager: Reading configuration from eeprom");
            wifi_softap_get_config(&_config);
            _begin_reason = true;
            WiFi.disconnect(); //Disable autoconnect
          }
          static void mode(const WiFiMode_t&_mode) {
           if(!_begin_reason || _start_reason) return;
           WiFi.mode(_mode);
          }
          static void ssid(const string&_ssid) {
            if(!_begin_reason || _start_reason) return;
            for(uint8 i=0;i<_ssid.length();i++)
              _config.ssid[i] = (uint8)_ssid.c_str()[i];
            _config.ssid[_ssid.length()] = '\0';
            _config.ssid_len = _ssid.length();
            _secure = false;
          }
          static void password(const string&_pass) {
            if(!_begin_reason || _start_reason) return;
            for(uint8 i=0;i<_pass.length();i++)
              _config.password[i] = (uint8)_pass.c_str()[i];
            _config.password[_pass.length()] = '\0';
            _secure = true;
          }
          static void start() {
            if(!_begin_reason || _start_reason) return;            
            Log::info("WiFiManager: Updating configuration");
            wifi_softap_set_config(&_config);
            Formatter::reset();
            Formatter::add(WLAN_OPMODE_NAMES[wifi_get_opmode()]);
            Formatter::add((char*)_config.ssid);
            Formatter::add((char*)_config.password);
            Log::info(Formatter::format("WiFiManager: Starting on [0] mode ([1]@[2])"));            
            switch(WiFi.getMode()){
              case WIFI_AP:
                if(_secure)
                  WiFi.softAP((char*)_config.ssid, (char*)_config.password);
                else
                  WiFi.softAP((char*)_config.ssid);
                break;
              case WIFI_STA:
              case WIFI_AP_STA:
                if(_secure)
                  WiFi.begin((char*)_config.ssid, (char*)_config.password);
                else
                  WiFi.begin((char*)_config.ssid);
                break;
            }            
            _start_reason = true;
          }
          static void stop() {
            if(!_begin_reason || !_start_reason) return;
            WiFi.disconnect();
            _start_reason = false;
            Log::info("WiFi: disconnected");
          }
          static void handleState(){
            if(!_begin_reason || !_start_reason || _ready) return;
            switch(WiFi.status()) {
              case WL_NO_SSID_AVAIL:
                Log::error("WiFiManager: No SSID available!");
                stop();
                break;
              case WL_CONNECT_FAILED:
                Log::error("WiFiManager: Connection failed");
                stop();
                break;
              case WL_CONNECTION_LOST:
                Log::error("WiFiManager: Connection lost");
                stop();
                break;
              case WL_NO_SHIELD:
                Log::error("WiFiManager: No shield found");
                stop();
                break;
              case WL_CONNECTED:
                _ready = true;
                break;
              case WL_DISCONNECTED:
              case WL_IDLE_STATUS:
                if(WiFi.getMode()!=WIFI_STA)
                  _ready = true;
                break;
            }
            if(_ready) {
              Formatter::reset();
              for(uint8_t i=0;i<4;i++)
                Formatter::add((int)(WiFi.getMode()==WIFI_STA ? WiFi.localIP()[i] : WiFi.softAPIP()[i]));
              Log::info(Formatter::format("Connected ([0].[1].[2].[3])"));
            }
          }
          static const bool ready() {
            return  _begin_reason &&
                    _start_reason &&
                    _ready;
          }
      };
      bool WiFiManager::_begin_reason = false,
           WiFiManager::_start_reason = false,
           WiFiManager::_ready = false,
           WiFiManager::_secure = false;
      softap_config WiFiManager::_config;
      #endif
    }    
  }
}
namespace extra {
  static const uint8_t _version[] = {2,52};
  namespace net {
    namespace http {}
  }
  namespace sensors {
    namespace adafruit {
      static void write8(TwoWire&_2wire, const uint8_t&_addr, const uint8_t&_reg, const uint8_t&_value) {
        _2wire.beginTransmission(_addr);
        _2wire.write(_reg);
        _2wire.write(_value);
        _2wire.endTransmission();
      }
      static const uint8_t read8(TwoWire&_2wire, const uint8_t&_addr, const uint8_t&_reg) {
        _2wire.beginTransmission(_addr);
        _2wire.write(_reg);
        _2wire.endTransmission();
        _2wire.requestFrom(_addr,1);
        return _2wire.read();        
      }
      static const uint16_t read16(TwoWire&_2wire, const uint8_t&_addr, const uint8_t&_reg) {            
        _2wire.beginTransmission(_addr);
        _2wire.write(_reg);
        _2wire.endTransmission();
        _2wire.requestFrom(_addr,2);            
        return (_2wire.read() << 8) | _2wire.read();
      }
      static const uint16_t read16_LE(TwoWire&_2wire, const uint8_t&_addr, const uint8_t&_reg) {
        uint16_t result = read16(_2wire, _addr, _reg);
        return (result >> 8) | (result << 8);
      }
      static const int16_t readS16(TwoWire&_2wire, const uint8_t&_addr, const uint8_t&_reg) {
        return (int16_t)read16(_2wire, _addr, _reg);
      }
      static const int16_t readS16_LE(TwoWire&_2wire, const uint8_t&_addr, const uint8_t&_reg) {
        return (int16_t)read16_LE(_2wire, _addr, _reg);
      }          
      static const uint32_t read24(TwoWire&_2wire, const uint8_t&_addr, const uint8_t&_reg) {
        uint32_t result;
        _2wire.beginTransmission(_addr);
        _2wire.write(_reg);
        _2wire.endTransmission();
        _2wire.requestFrom(_addr, 3);
        return (((_2wire.read() << 8) | _2wire.read()) << 8) | _2wire.read();
      }
      namespace bmp280 {         
        static const uint8_t BMP280_REGISTER_DIG_T[] = {0x88,0x8A, 0x8C},
                             BMP280_REGISTER_DIG_P[] = {0x8E, 0x90, 0x92, 0x94, 0x96, 0x98, 0x9A, 0x9C, 0x9E},
                             BMP280_REGISTER_CHIPID = 0xD0,
                             BMP280_REGISTER_VERSION = 0xD1,
                             BMP280_REGISTER_SOFTRESET = 0xE0,
                             BMP280_REGISTER_CAL26 = 0xE1,
                             BMP280_REGISTER_CONTROL = 0xF4,
                             BMP280_REGISTER_CONFIG = 0xF5, 
                             BMP280_REGISTER_PRESSUREDATA = 0xF7,
                             BMP280_REGISTER_TEMPDATA = 0xFA;
        struct bmp280_data {
          uint32_t _t_fine;
          uint16_t _dig_t[sizeof(BMP280_REGISTER_DIG_T)];
          uint16_t _dig_p[sizeof(BMP280_REGISTER_DIG_P)];
        };
        static std::map<uint8_t,bmp280_data> _diags;
        static void readCoefficients(TwoWire&_2wire, core::sensors::Sensor&_sensor) {
          if(_diags.find(_sensor.addr()[0])==_diags.end()){
            bmp280_data data;
            core::Formatter::reset();
            core::Formatter::add(string(_sensor.addr()[0],HEX));
            core::Formatter::add(_sensor.addr()[0]==0x76 ? sizeof(BMP280_REGISTER_DIG_T) : sizeof(BMP280_REGISTER_DIG_P));          
            core::Log::debug(core::Formatter::format("BMP280(0x[0]): readCoefficients ([1])"));
            if(_sensor.addr()[0]==0x76)
              for(int i=0;i<sizeof(BMP280_REGISTER_DIG_T);i++) {
                if(i==0) data._dig_t[i] = read16_LE(_2wire, _sensor.addr()[0], BMP280_REGISTER_DIG_T[i]);
                else data._dig_t[i] = readS16_LE(_2wire, _sensor.addr()[0], BMP280_REGISTER_DIG_T[i]);
                core::Formatter::reset();
                core::Formatter::add(string(_sensor.addr()[0],HEX));
                core::Formatter::add(i);
                core::Formatter::add(data._dig_t[i]);
                core::Log::debug(core::Formatter::format("BMP280(0x[0]): _dig_t[[1]] = [2]"));
              }
            else
              for(int i=0;i<sizeof(BMP280_REGISTER_DIG_P);i++) {
                if(i==0) data._dig_p[i] = read16_LE(_2wire, _sensor.addr()[0], BMP280_REGISTER_DIG_P[i]);
                else data._dig_p[i] = readS16_LE(_2wire, _sensor.addr()[0], BMP280_REGISTER_DIG_P[i]);
                core::Formatter::reset();
                core::Formatter::add(string(_sensor.addr()[0],HEX));
                core::Formatter::add(i);
                core::Formatter::add(data._dig_p[i]);
                core::Log::debug(core::Formatter::format("BMP280(0x[0]): _dig_p[[1]] = [2]"));
              }
            core::Formatter::reset();
            core::Formatter::add(string(_sensor.addr()[0],HEX));
            _diags.insert(std::pair<uint8_t,bmp280_data>(_sensor.addr()[0],data));
            core::Log::debug(core::Formatter::format("BMP280(0x[0]): Coefficients readed"));
          }          
        }
        static const bool readTemperature(TwoWire&_2wire, const uint8_t&_addr, float&_t) {
          if(_addr!=0x76) return false;
          int32_t adc_t = read24(_2wire, _addr, BMP280_REGISTER_TEMPDATA) >> 4,
                  var1 = ((((adc_t >> 3) - ((int32_t)_diags[_addr]._dig_t[0] << 1))) *
                    ((int32_t)_diags[_addr]._dig_t[1])) >> 11,
                  var2 = (((((adc_t >> 4) - ((int32_t)_diags[_addr]._dig_t[0])) * 
                    ((adc_t >> 4) - ((int32_t)_diags[_addr]._dig_t[0]))) >> 12) *
                    ((int32_t)_diags[_addr]._dig_t[2])) >> 14;              
          _diags[_addr]._t_fine = var1 + var2;
          core::Formatter::reset(); 
          core::Formatter::add(string(_addr,HEX));
          core::Formatter::add(adc_t);          
          core::Formatter::add(_diags[_addr]._t_fine);  
          core::Log::debug(core::Formatter::format("BMP280(0x[0]): adc_T = [1], t(fine) = [2]"));
          _t =((_diags[_addr]._t_fine * 5 + 128) >> 8) / 100.0;
          return true;
        }
        static const bool readPressure(TwoWire&_2wire, const uint8_t&_addr, float&_p) {
          if(_addr!=0xf6 || _diags.find(0x76)==_diags.end()) return false;            
          int64_t var1, var2, p;           
          int32_t adc_P = read24(_2wire, _addr, BMP280_REGISTER_PRESSUREDATA) >> 4;
          core::Formatter::reset();
          core::Formatter::add(string(_addr,HEX));
          core::Formatter::add(adc_P);  
          core::Log::debug(core::Formatter::format("BMP280(0x[0]): adc_P = [1]"));
          var1 = ((int64_t)_diags[0x76]._t_fine) - 128000;
          var2 = var1 * var1 * (int64_t)_diags[_addr]._dig_p[5];
          var2 = var2 + ((var1*(int64_t)_diags[_addr]._dig_p[4])<<17);
         var2 = var2 + (((int64_t)_diags[_addr]._dig_p[3])<<35);
          var1 = ((var1 * var1 * (int64_t)_diags[_addr]._dig_p[2])>>8) +
            ((var1 * (int64_t)_diags[_addr]._dig_p[1])<<12);
          var1 = (((((int64_t)1)<<47)+var1))*((int64_t)_diags[_addr]._dig_p[0])>>33; 
          if (var1 == 0) {
            return 0;  // avoid exception caused by division by zero
          }
          p = 1048576 - adc_P;
          p = (((p<<31) - var2)*3125) / var1;
          var1 = (((int64_t)_diags[_addr]._dig_p[8]) * (p>>13) * (p>>13)) >> 25;
          var2 = (((int64_t)_diags[_addr]._dig_p[7]) * p) >> 19;
          p = ((p + var1 + var2) >> 8) + (((int64_t)_diags[_addr]._dig_p[6])<<4);
          _p = (float)p/256.0;
          return true;
        }      
      }
      class AdafruitSensorObject : public core::sensors::SensorObject {
        public:
          AdafruitSensorObject() : core::sensors::SensorObject("Adafruit") {
            add(0x58,"BMP280");      
          }
          virtual void recieve(TwoWire&_2wire, core::sensors::Sensor&_sensor) {
            float t = -1;
            switch(_sensor.chipId()) {
              case 0x58: //BMP280
                bmp280::readCoefficients(_2wire,_sensor);
                switch(_sensor.addr()[0]){                                 
                  case 0x76: //Temperature
                    if(bmp280::readTemperature(_2wire,_sensor.addr()[0],t)) {
                      if(_sensor.data().size()<1){
                        _sensor.add(core::sensors::SensorData(core::sensors::CELSIUS));
                        _sensor.add(core::sensors::SensorData(core::sensors::FAHRENHEIT));
                      }
                      _sensor.update(0,t);
                      _sensor.update(1,core::sensors::c2f(t));
                    }                    
                    break;
                  case 0xf6: //Pressure
                    if(bmp280::readPressure(_2wire,_sensor.addr()[0],t)) {
                      if(_sensor.data().size()<1){
                        _sensor.add(core::sensors::SensorData(core::sensors::PA));
                        _sensor.add(core::sensors::SensorData(core::sensors::TORR));
                      }
                      _sensor.update(0,t);
                      _sensor.update(1,core::sensors::pa2torr(t));
                    }
                    break;
                }
                break;
            }
            _sensor.nextRequest(500);
          }
      };
    }
    namespace dallas {
      class DallasSensorObject : public core::sensors::SensorObject {
        public:
          DallasSensorObject() : core::sensors::SensorObject("Dallas") {
            add(0x10,"DS18S20");
            add(0x22,"DS1822");
            add(0x28,"DS18B20");          
          }
          virtual void recieve(OneWire&_1wire, core::sensors::Sensor&_sensor) {
            uint8_t addr[8];
            for(uint8_t i=0;i<8;i++)
              addr[i] = _sensor.addr()[i];
            switch(_sensor.state()) {
              case core::sensors::REQUEST_REQUIRED:           
                if(_sensor.data().size()<1) {
                  _sensor.add(core::sensors::SensorData(core::sensors::CELSIUS));
                  _sensor.add(core::sensors::SensorData(core::sensors::FAHRENHEIT));
                }
                _1wire.reset();
                _1wire.select(addr);
                _1wire.write(0x44,true);
                _1wire.reset();
                _sensor.state(core::sensors::PROCESSING);
                _sensor.nextRequest(500);
                break;
              case core::sensors::PROCESSING:
                _1wire.reset();
                _1wire.select(addr);
                _1wire.write(0xBE,false);
                uint8_t data[12];
                for(uint8_t i=0;i<9;i++)
                  data[i] = _1wire.read();
                uint16_t raw = (data[1] << 8) | data[0];
                if(addr[0] == 0x10) {
                  raw <<= 3;
                  if(data[7] == 0x10)
                    raw = (raw & 0xFFF0) + 12 - data[6];
                } else {
                  uint8_t cfg = (data[4] & 0x60);
                  if(cfg == 0x0) raw &= ~7;
                  else if(cfg == 0x20) raw &= ~3;
                  else if(cfg == 0x40) raw &= ~1; 
                }
                const float t = raw/16.0;
                _sensor.update(0,t);
                _sensor.update(1,core::sensors::c2f(t));
                _sensor.state(core::sensors::REQUEST_REQUIRED);
                _sensor.nextRequest(core::sensors::SENSORS_UPDATE_DELAY);
                break;
            }
          }
      };
    }    
  }
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(500);
  Serial.setDebugOutput(true);
  Serial.println();
  core::Log::info("Board starting");
  #ifdef ESP8266
    core::Formatter::reset();
    core::Formatter::add(project::_name);
    core::Formatter::add(core::_version[0]);
    core::Formatter::add(core::_version[1]);
    core::Formatter::add(extra::_version[0]);
    core::Formatter::add(extra::_version[1]);
    core::Formatter::add(project::PROJECT_STAGE_NAMES[project::_stage]);
    core::Formatter::add(project::_release);
    core::Log::info(core::Formatter::format("[0] version [1].[2]c[3].[4]e_[5][6]"));
    core::sensors::Sensors::add(core::sensors::ONE_WIRE,new extra::sensors::dallas::DallasSensorObject());
    core::sensors::Sensors::add(core::sensors::TWO_WIRE,new extra::sensors::adafruit::AdafruitSensorObject());
    core::sensors::Sensors::add(D1);
    core::sensors::Sensors::add(D2,D3);
    core::sensors::Sensors::begin();
    core::net::wlan::WiFiManager::begin();
    core::net::wlan::WiFiManager::mode(WIFI_AP);
    core::net::wlan::WiFiManager::ssid("alcopython");
    core::net::wlan::WiFiManager::password("changeme");
    core::net::wlan::WiFiManager::start();
  #else
    core::Log::error("Sorry, but this sketch only for ESP8266 boards!");
  #endif
}

void loop() {
  // put your main code here, to run repeatedly:
  #ifdef ESP8266
    core::sensors::Sensors::update();
    core::net::wlan::WiFiManager::handleState();
    /* TODO
    if(core::net::wlan::WiFiManager::ready()) {
        if(!core::net::http::WebServer::started())  
          core::net::http::WebServer::begin();
        else core::net::http::WebServer::handleClient();
    
    }
    */
  #endif
}
