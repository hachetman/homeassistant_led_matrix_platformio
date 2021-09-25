#include "HAconnect.h"
#include "ArduinoJson.hpp"
#include "HTTPClient.h"
#include "HardwareSerial.h"

HAconnect::HAconnect(String Address, int Port, String Auth)
    : http(), address(Address), port(Port), auth(Auth) {}
auto HAconnect::getEntity(String entity, String &message) -> int {
  String access_path = "/api/states/" + entity;
  int httpCode = 0;
  http.begin(address, port, access_path);
  http.addHeader("Content-Type", "application/json");
  http.setAuthorization("");
  http.addHeader("Authorization", auth.c_str());

  httpCode = http.GET();

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      message = http.getString();
      http.end();
    } else {
      message = http.errorToString(httpCode).c_str();
      http.end();
    }
  }
  return httpCode;
}

auto HAconnect::getState(String entity) -> int {

  String message;
  int result = getEntity(entity, message);

  if (result == HTTP_CODE_OK) {
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, message);
    return doc["state"].as<int>();
  } else {
    Serial.println("Error in Connect");
    return 0;
  }
}

auto HAconnect::getWeather(struct HaExchange *haexchange) -> int {

  String message;
  String buffer;
  int result = getEntity("weather.home", message);

  if (result == HTTP_CODE_OK) {
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, message);
    haexchange->humidity = doc["attributes"]["humidity"];
    haexchange->temperature = doc["attributes"]["temperature"];
    serializeJson(doc["attributes"]["forecast"][0], buffer);
    Serial.println(buffer);
    return doc["attributes"].as<int>();
  } else {
    Serial.println("Error in Connect");
    return 0;
  }
}

auto HAconnect::getPing() -> int {

  String message;
  String buffer;
  int result = getEntity("binary_sensor.ping_heise_de", message);
  if (result == HTTP_CODE_OK) {
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, message);
    return doc["attributes"]["round_trip_time_avg"];
  } else {
    Serial.println("Error in Connect");
    return 0;
  }
}

auto HAconnect::getSun() -> bool {

  String message;
  String buffer;
  int result = getEntity("sun.sun", message);
  if (result == HTTP_CODE_OK) {
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, message);
    if (doc["state"] == "above_horizon") {
      return true;
    } else {
      return false;
    }
  } else {
    Serial.println("Error in Connect");
    return false;
  }
}
