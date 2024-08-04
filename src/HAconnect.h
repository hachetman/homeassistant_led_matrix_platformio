#ifndef HACONNECT_H
#define HACONNECT_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
enum class ui_state : uint32_t{
  NO_WIFI = 0x00,
  OVERVIEW = 0x01,
  WEATHER = 0x02
    };
struct HaExchange {
  ui_state uistate;
  ui_state last_uistate;
  int co2;
  int temperature;
  int mintemp;
  int maxtemp;
  int humidity;
  int ping;
  bool sun;
  int precipation;
  int solar_lux;
  int uv_index;
  int wind;
  int rain;
};
class HAconnect {
public:
  HAconnect(String Address, int Port, String Auth);
  auto getState(const String& entity) -> int;
  auto getWeather(struct HaExchange *haexchange) -> int;

  auto getPing()
      -> int;
  auto getSun() -> bool;

private:
  auto getEntity(const String& entity, String &message) -> int;
  HTTPClient http;
  String address;
  int port;
  String auth;
};
#endif
