#pragma once
#include <WebServer.h>
#include "ApiHandler.h"

// Thin HTTP adapter: forwards every request to ApiHandler.
class ApiServer {
public:
  ApiServer(ApiHandler& handler, uint16_t port);
  void update(bool wifiConnected);   // call every loop()

private:
  void onRequest();
  WebServer   _server;
  ApiHandler& _handler;
  bool        _started = false;
};