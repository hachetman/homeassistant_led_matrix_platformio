#ifndef HACONNECT_H
#define HACONNECT_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
enum class ui_state : uint32_t{
  NO_WIFI = 0x00,
  OVERVIEW = 0x01
    };
struct HaExchange {
  ui_state uistate;
  int co2;
  int temperature;
  int mintemp;
  int maxtemp;
  int humidity;
  int ping;
  bool sun;
  int precipation;
};
class HAconnect {
public:
  HAconnect(String Address, int Port, String Auth);
  auto getState(String entity) -> int;
  auto getWeather(struct HaExchange *haexchange)
      -> int;
  auto getPing()
      -> int;
  auto getSun() -> bool;

private:
  auto getEntity(String entity, String &message) -> int;
  HTTPClient http;
  String address;
  int port;
  String auth;
};
#endif
