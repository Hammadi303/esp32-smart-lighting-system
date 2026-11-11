
#include "ApiServer.h"

ApiServer::ApiServer(ApiHandler& handler, uint16_t port)
  : _server(port), _handler(handler) {}

void ApiServer::update(bool wifiConnected) {
  if (!_started && wifiConnected) {
    _server.onNotFound([this]() { onRequest(); });
    _server.begin();
    _started = true;
    Serial.println("HTTP server started");
  }

  if (_started) _server.handleClient();
}

void ApiServer::onRequest() {
  const HTTPMethod m = _server.method();
  const char* method = (m == HTTP_GET) ? "GET" : (m == HTTP_POST) ? "POST" : "OTHER";

  String body = _server.arg("plain");

  Serial.println("----- HTTP REQUEST -----");
  Serial.print("Method: ");
  Serial.println(method);
  Serial.print("URI: ");
  Serial.println(_server.uri());
  Serial.print("Body: ");
  Serial.println(body);

  ApiResponse r = _handler.handle(method, _server.uri(), body, millis());

  Serial.print("HTTP response: ");
  Serial.println(r.status);
  Serial.println(r.body);

  _server.send(r.status, "application/json", r.body);
}
