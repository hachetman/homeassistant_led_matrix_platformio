#ifndef HACONNECT_H
#define HACONNECT_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
class HAconnect {
public:
  HAconnect(String Address, int Port, String Auth);
  auto getState(String entity) -> int;

private:
  auto getEntity(String entity, String & message) -> int;
  HTTPClient http;
  String address;
  int port;
  String auth;
};
#endif
