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
  static const uint8_t _release = 2;
}
namespace core {
  static const float _version = 0.98;
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
  namespace sensors {
    static const long SENSORS_UPDATE_DELAY = 1000;
    enum sensor_t {
      TEMPERATURE,
      PRESSURE,
      UNKNOWN
    };
    static const string str(const sensor_t&_type) {
      switch(_type) {
        case TEMPERATURE: return "Temperature";
        case PRESSURE: return "Pressure";
      }
      return "Unknown";
    }
    enum temperature_unit_t {
      CELSIUS,
      FAHRENHEIT
    };
    static const string str(const temperature_unit_t&_unit) {
      switch(_unit) {
        case CELSIUS: return "'C";
        case FAHRENHEIT: return "'F";
      }
      return "N/A";
    }
    enum pressure_unit_t {
      PA,
      HPA,
      INHG,
      BAR,
      TORR,
      PSI
    };
    static const string str(const pressure_unit_t&_unit) {
      switch(_unit) {
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
    enum sensor_status_t {
      REQUEST_REQUIRE,
      PROCESSING,
      OK
    };
    class Sensor {
      private:
        std::vector<uint8_t> _addr;        
        sensor_status_t _status;
        long _next_request;
        float _value;
      public:
        Sensor() {}
        Sensor(const std::vector<uint8_t>&_addr) : _status(REQUEST_REQUIRE), _value(0), _next_request(millis()) {
          for(int i=0;i<_addr.size();i++)
            this->_addr.push_back(_addr[i]);          
        }
        const float value() {
          return _value;
        }
        void value(const float&_value) {
          if(_status != OK) return;
          this->_value = _value;
        }
        const size_t addrSize(){
          return _addr.size();
        }
        const uint8_t addr(const int&_id) {
          return _id >= 0 && _id < _addr.size() ? _addr[_id] : 0xff;
        }
        const string addrStr(){
          string _out = "";
          for(int i=0;i<_addr.size();i++) {
            if(i!=0) _out += ':';
            if(_addr[i]<16) _out += '0';
            _out += string(_addr[i],HEX);
          }
          return _out;
        }
        const sensor_status_t status() {
          return _status;
        }
        void processing(const long&_delay) {
          if(_status!=REQUEST_REQUIRE) return;
          _status = PROCESSING;
          _next_request = millis() + _delay;
        }
        void ok() {
          _status = OK;
          _next_request = millis();
        }
        const long nextRequest() {
          return _next_request;
        }
    };
    class SensorMeta {
      private:
        const sensor_t _type;
        const string _model;
      public:
        SensorMeta() : _type(UNKNOWN), _model("Unknown") {}
        SensorMeta(const sensor_t&_type, const string&_model) : _type(_type), _model(_model) {}
        const sensor_t type() {
          return _type;
        }
        const string model(){
          return _model;
        };        
    };
    class SensorObject {
      private:
        std::map<uint8_t,SensorMeta> _devs;
        const string _vendor;
      protected:
        void add(const uint8_t&_chipId, const sensor_t&_type, const string&_desc) {
          if(check(_chipId)) return;
          _devs.insert(std::pair<uint8_t,SensorMeta>(_chipId,SensorMeta(_type,_desc)));
        }
      public:
        const bool check(const uint8_t&_chipId) {
          return _devs.find(_chipId)!=_devs.end();
        }
        const sensor_t sensorType(const uint8_t&_chipId) {
          return check(_chipId) ? _devs[_chipId].type() : UNKNOWN;
        }
        const string sensorModel(const uint8_t&_chipId) {
          return check(_chipId) ? _devs[_chipId].model() : "Unknown";
        }
        virtual void recieve(OneWire&_1wire, Sensor&_s) {}
        virtual void recieve(TwoWire&_2wire, Sensor&_s) {}
    };
    class SensorWire {
      private:
        const uint8_t _id;
        const wire_t _type;
        OneWire* _1wire;
        TwoWire* _2wire;
        std::vector<Sensor> _sensors;
        bool _begin_reason;
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
          void begin() {}
          void end() {}
          void update() {}
    };
    class Sensors {
    };
  }
  namespace net {
    namespace http {}
    namespace wlan {}
  }
}
namespace extra {
  static const float _version = 0.0;
  namespace net {
    namespace http {}
  }
  namespace sensors {}
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();
  Serial.println("Board starting");
  #ifdef ESP8266
    core::Formatter::begin();
    core::Formatter::add("----------------------------------------------");
    core::Formatter::add(project::_name);
    core::Formatter::add(core::_version);
    core::Formatter::add(extra::_version);
    core::Formatter::add(project::str(project::_stage));
    core::Formatter::add(project::_release);
    Serial.println(core::Formatter::format("[0]\n[1] version c[2]e[3] ([4] [5])\n[0]"));
    core::Formatter::end();
  #else
    core::Log::error("Sorry, but this sketch only for ESP8266 boards!");
  #endif
}

void loop() {
  // put your main code here, to run repeatedly:
  #ifdef ESP8266
  #endif
}
