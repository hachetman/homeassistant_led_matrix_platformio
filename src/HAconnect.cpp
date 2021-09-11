#include "HAconnect.h"
#include "HardwareSerial.h"

HAconnect::HAconnect(String Address, int Port, String Auth)
    : address(Address), port(Port), auth(Auth) {}

auto HAconnect::getEntity(String entity, String &message) -> int {
  String access_path = "/api/states/" + entity;
  int httpCode = 0;
  http.begin(address, port,
             access_path); // Specify destination for HTTP request
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

  if (result == 200) {
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, message);
    return doc["state"].as<int>();
  } else {
    Serial.println("Error in Connect");
    return 0;
  }
}
