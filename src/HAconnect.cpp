#include "HAconnect.h"

#include "ArduinoJson.hpp"
#include "HTTPClient.h"
#include "HardwareSerial.h"
#include <utility>

HAconnect::HAconnect(String Address, int Port, String Auth)
    : http(), address(std::move(Address)), port(Port), auth(std::move(Auth)) {}

auto HAconnect::getEntity(const String &entity, String &message) -> int {
  String access_path = "/api/states/" + entity;
  int httpCode;
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
      message = HTTPClient::errorToString(httpCode).c_str();
      http.end();
    }
  }
  return httpCode;
}

auto HAconnect::getWeather(struct HaExchange *haexchange) -> int {
  String message;
  int result = getEntity("sensor.weather_forecast_daily", message);

  if (result == HTTP_CODE_OK) {
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, message);
    String forecastTomorrow = doc["attributes"]["forecast"][0];
    haexchange->maxtemp =
        doc["attributes"]["forecast"][0]["temperature"].as<int>();
    haexchange->mintemp = doc["attributes"]["forecast"][0]["templow"].as<int>();
    haexchange->precipation =
        doc["attributes"]["forecast"][0]["precipation"].as<int>();

    return 0;
  } else {
    Serial.print("Error in Connect: ");
    Serial.println(message.c_str());
    return 1;
  }
}
auto HAconnect::getState(const String &entity) -> int {

  String message;
  int result = getEntity(entity, message);

  if (result == HTTP_CODE_OK) {
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, message);
    return doc["state"].as<int>();

  } else {
    Serial.print("Error in Connect:");
    Serial.println(entity);
    return 0;
  }
}

auto HAconnect::getPing() -> int {

  String message;
  String buffer;
  int result = getEntity("binary_sensor.heise_de", message);
  if (result == HTTP_CODE_OK) {
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, message);
    return doc["attributes"]["round_trip_time_avg"];
  } else {
    Serial.println("Error in Connect: Ping");
    return 0;
  }
}

auto HAconnect::getSun() -> bool {

  String message;
  String buffer;
  int result = getEntity("sun.sun", message);
  bool retval;
  if (result == HTTP_CODE_OK) {
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, message);
    if (doc["state"] == "above_horizon") {
      retval = true;
    } else {
      retval = false;
    }
  } else {
    Serial.println("Error in Connect: Sun");
    retval = false;
  }
  return retval;
}
