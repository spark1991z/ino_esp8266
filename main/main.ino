#include <vector>
#include <map>
#include <Wire.h>
#include <OneWire.h>
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
  static const uint8_t _release = 4;
}
namespace core {
  static const uint8_t _version[2] = {0,115};
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
    static const float SENSOR_VALUE_DEFAULT = -1;
    static const string SENSOR_MODEL_UNKNOWN = "Unknown";    
    enum sensor_unit_t {
      // Temperature
      CELSIUS,
      FAHRENHEIT,
      // Pressure
      PA,
      HPA,
      INHG,
      BAR,
      TORR,
      PSI,
      // Unknown
      UNKNOWN
    };
    static const string str(const sensor_unit_t&_unit) {
      switch(_unit) {
        case CELSIUS: return "'C";
        case FAHRENHEIT: return "'F";
        case PA: return "Pa";
        case HPA: return "hPa";
        case INHG: return "inHg";
        case BAR: return "bar";
        case TORR: return "torr";
        case PSI: return "psi";
      }
      return "N/A";
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
        std::vector<uint8_t> _addr;
        std::vector<SensorData> _data;
        long _next_request;
        sensor_state_t _state;
      public:
        Sensor() {}
        Sensor(const std::vector<uint8_t>&_addr) :_next_request(millis()), _state(REQUEST_REQUIRED) { 
          for(uint8_t i=0;i<_addr.size();i++)
            this->_addr.push_back(_addr[i]);
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
          return check(_chipId) ? _devices[_chipId] : SENSOR_MODEL_UNKNOWN;
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
                _sensors.push_back(Sensor(_addr));
                _addr.clear();
              }
              _1wire->reset_search();
              break;
            case TWO_WIRE:
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
          return _id>=0 && _id<_sensors.size() && _obj->check(_sensors[_id].addr()[0]);
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
              string val = "";
              for(uint8_t j=0;j<_objects[sw.type()].size();j++) {
                if(_objects[sw.type()][j]->check(sw.addr(i)[0])) {
                  detect = true;
                  val = _objects[cur->second.type()][j]->vendor()+" "+_objects[cur->second.type()][j]->model(sw.addr(i)[0]);
                  break;
                }
              }
              Formatter::add(detect ? val : SENSOR_MODEL_UNKNOWN);
              Log::info(Formatter::format("Found on wire 0x[0] sensor [1]: [2] ([3])"));
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
                    val += str(sd[k]._unit);
                  }
                  Formatter::add(val);
                  Log::debug(Formatter::format("Recieved from wire 0x[0] sensor [1] [2] values: {[3]}"));
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
    namespace wlan {}
  }
}
namespace extra {
  static const uint8_t _version[2] = {1,0};
  namespace net {
    namespace http {}
  }
  namespace sensors {
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
              if(_sensor.data().size()<2) {
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
              _sensor.update(0,raw/16.0);
              _sensor.update(1,raw/16.0*1.8+32);
              _sensor.state(core::sensors::REQUEST_REQUIRED);
              _sensor.nextRequest(core::sensors::SENSORS_UPDATE_DELAY);
              break;
          }
        }
    };    
  }
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();
  Serial.println("Board starting");
  #ifdef ESP8266
    core::Formatter::reset();
    core::Formatter::add("----------------------------------------------");
    core::Formatter::add(project::_name);
    core::Formatter::add(core::_version[0]);
    core::Formatter::add(core::_version[1]);
    core::Formatter::add(extra::_version[0]);
    core::Formatter::add(extra::_version[1]);
    core::Formatter::add(project::str(project::_stage));
    core::Formatter::add(project::_release);
    Serial.println(core::Formatter::format("[0]\n[1] version [2].[3]c[4].[5]e ([6] [7])\n[0]"));
    core::sensors::Sensors::add(core::sensors::ONE_WIRE,new extra::sensors::DallasSensorObject());
    core::sensors::Sensors::add(D1);
    core::sensors::Sensors::add(D2,D3);
    core::sensors::Sensors::begin();
  #else
    core::Log::error("Sorry, but this sketch only for ESP8266 boards!");
  #endif
}

void loop() {
  // put your main code here, to run repeatedly:
  #ifdef ESP8266
  core::sensors::Sensors::update();
  #endif
}
