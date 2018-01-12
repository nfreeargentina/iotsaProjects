#include "iotsaUser.h"
#include "iotsaConfigFile.h"
#include "IotsaJWTToken.h"
#include "ArduinoJWT.h"

// This module requires the ArduinoJWT library from https://github.com/yutter/ArduinoJWT

IotsaJWTTokenMod::IotsaJWTTokenMod(IotsaApplication &_app, IotsaAuthMod &_chain)
:	chain(_chain),
	IotsaAuthMod(_app)
{
	configLoad();
}
	
void
IotsaJWTTokenMod::handler() {
  if (needsAuthentication("tokens")) return;
  bool anyChanged = false;
  if (server.hasArg("issuer")) {
    trustedIssuer = server.arg("issuer");
    anyChanged = true;
  }
  if (server.hasArg("issuerKey")) {
   issuerKey = server.arg("issuerKey");
    anyChanged = true;
  }
  if (anyChanged) configSave();

  String message = "<html><head><title>JWT Keys</title></head><body><h1>JWT Keys</h1>";
  message += "<form method='get'>Trusted JWT Issuer: <input name='issuer' value='";
  message += trustedIssuer;
  message += "'>";
  message += "<br>Secret Key: <input name='issuerKey' value='";
  message += issuerKey;
  message += "'>";

  message += "<br><input type='submit'></form>";
  server.send(200, "text/html", message);
}

void IotsaJWTTokenMod::setup() {
  configLoad();
}

void IotsaJWTTokenMod::serverSetup() {
  server.on("/jwt", std::bind(&IotsaJWTTokenMod::handler, this));
}

void IotsaJWTTokenMod::configLoad() {
  IotsaConfigFileLoad cf("/config/jwt.cfg");
  cf.get("trustedIssuer", trustedIssuer, "");
  cf.get("issuerKey", issuerKey, "");
}

void IotsaJWTTokenMod::configSave() {
  IotsaConfigFileSave cf("/config/jwt.cfg");
  cf.put("trustedIssuer", trustedIssuer);
  cf.put("issuerKey", issuerKey);
}

void IotsaJWTTokenMod::loop() {
}

String IotsaJWTTokenMod::info() {
  String message = "<p>JWT tokens enabled.";
  message += " See <a href=\"/jwt\">/tokens</a> to change settings.";
  message += "</p>";
  return message;
}

bool IotsaJWTTokenMod::needsAuthentication(const char *right) {
  if (server.hasHeader("Authorization")) {
    String authHeader = server.header("Authorization");
    if (authHeader.startsWith("Bearer ")) {
      String token = authHeader.substring(7);
      ArduinoJWT(issuerKey);
      String payload;
      bool ok = decodeJWT(token, payload);
      if (ok) {
        // Token decoded correctly.
        // xxxjack decode JSON from payload
        // xxxjack check that issuer matches
        // xxxjack check that right matches
        return true;
      }
  }
  IotsaSerial.println("No token match, try user/password");
  // If no rights fall back to username/password authentication
  return chain.needsAuthentication(right);
}
