// ***************************************************************************
// Request handlers
// ***************************************************************************
void getArgs() {
  if (server.arg("rgb") != "") {
    uint32_t rgb = (uint32_t) strtol(server.arg("rgb").c_str(), NULL, 16);
    main_color.red = ((rgb >> 16) & 0xFF);
    main_color.green = ((rgb >> 8) & 0xFF);
    main_color.blue = ((rgb >> 0) & 0xFF);
  } else {
    main_color.red = server.arg("r").toInt();
    main_color.green = server.arg("g").toInt();
    main_color.blue = server.arg("b").toInt();
  }
  ws2812fx_speed = constrain(server.arg("s").toInt(), 0, 255);
  if (server.arg("s") == "") {
    ws2812fx_speed = 128;
  }

  ws2812fx_mode = constrain(server.arg("m").toInt(), 0, strip.getModeCount()-1);
  
  main_color.red = constrain(main_color.red, 0, 255);
  main_color.green = constrain(main_color.green, 0, 255);
  main_color.blue = constrain(main_color.blue, 0, 255);

  DBG_OUTPUT_PORT.print("Mode: ");
  DBG_OUTPUT_PORT.print(mode);
  DBG_OUTPUT_PORT.print(", Color: ");
  DBG_OUTPUT_PORT.print(main_color.red);
  DBG_OUTPUT_PORT.print(", ");
  DBG_OUTPUT_PORT.print(main_color.green);
  DBG_OUTPUT_PORT.print(", ");
  DBG_OUTPUT_PORT.print(main_color.blue);
  DBG_OUTPUT_PORT.print(", Speed:");
  DBG_OUTPUT_PORT.print(ws2812fx_speed);
  DBG_OUTPUT_PORT.print(", Brightness:");
  DBG_OUTPUT_PORT.println(brightness);
}




void handleSetMainColor(uint8_t * payload) {
  // decode rgb data
  uint32_t rgb = (uint32_t) strtol((const char *) &payload[1], NULL, 16);
  main_color.red = ((rgb >> 16) & 0xFF);
  main_color.green = ((rgb >> 8) & 0xFF);
  main_color.blue = ((rgb >> 0) & 0xFF);
  strip.setColor(main_color.red, main_color.green, main_color.blue);
}

void handleSetAllMode(uint8_t * payload) {
  // decode rgb data
  uint32_t rgb = (uint32_t) strtol((const char *) &payload[1], NULL, 16);

  main_color.red = ((rgb >> 16) & 0xFF);
  main_color.green = ((rgb >> 8) & 0xFF);
  main_color.blue = ((rgb >> 0) & 0xFF);

  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, main_color.red, main_color.green, main_color.blue);
  }
  strip.show();
  DBG_OUTPUT_PORT.printf("WS: Set all leds to main color: [%u] [%u] [%u]\n", main_color.red, main_color.green, main_color.blue);
  exit_func = true;
  mode = ALL;
}

void handleSetSingleLED(uint8_t * payload) {
  // decode led index
  uint64_t rgb = (uint64_t) strtol((const char *) &payload[1], NULL, 16);

  uint8_t led =          ((rgb >> 24) & 0xFF);
  if (led < strip.numPixels()) {
    ledstates[led].red =   ((rgb >> 16) & 0xFF);
    ledstates[led].green = ((rgb >> 8)  & 0xFF);
    ledstates[led].blue =  ((rgb >> 0)  & 0xFF);
    DBG_OUTPUT_PORT.printf("WS: Set single led [%u] to [%u] [%u] [%u]!\n", led, ledstates[led].red, ledstates[led].green, ledstates[led].blue);

    for (uint8_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, ledstates[i].red, ledstates[i].green, ledstates[i].blue);
      //DBG_OUTPUT_PORT.printf("[%u]--[%u] [%u] [%u] [%u] LED index!\n", rgb, i, ledstates[i].red, ledstates[i].green, ledstates[i].blue);
    }
    strip.show();
  }
  exit_func = true;
  mode = ALL;
}

void handleSetNamedMode(String str_mode) {
  exit_func = true;
  
  if (str_mode.startsWith("=off")) {
    mode = OFF;
  }
  if (str_mode.startsWith("=all")) {
    mode = ALL;
  }
  if (str_mode.startsWith("=wipe")) {
    mode = WIPE;
  }
  if (str_mode.startsWith("=rainbow")) {
    mode = RAINBOW;
  }
  if (str_mode.startsWith("=rainbowCycle")) {
    mode = RAINBOWCYCLE;
  }
  if (str_mode.startsWith("=theaterchase")) {
    mode = THEATERCHASE;
  }
  if (str_mode.startsWith("=theaterchaseRainbow")) {
    mode = THEATERCHASERAINBOW;
  }
  if (str_mode.startsWith("=tv")) {
    mode = TV;
  }
}

void handleSetWS2812FXMode(uint8_t * payload) {
  mode = HOLD;
  uint8_t ws2812fx_mode = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
  ws2812fx_mode = constrain(ws2812fx_mode, 0, 255);
  strip.setColor(main_color.red, main_color.green, main_color.blue);
  strip.setMode(ws2812fx_mode);
}









void handleMinimalUpload() {
  char temp[1500];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf ( temp, 1500,
       "<!DOCTYPE html>\
				<html>\
					<head>\
						<title>ESP8266 Upload</title>\
						<meta charset=\"utf-8\">\
						<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\
						<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
					</head>\
					<body>\
						<form action=\"/edit\" method=\"post\" enctype=\"multipart/form-data\">\
							<input type=\"file\" name=\"data\">\
							<input type=\"text\" name=\"path\" value=\"/\">\
							<button>Upload</button>\
						</form>\
					</body>\
				</html>",
             hr, min % 60, sec % 60
           );
  server.send ( 200, "text/html", temp );
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }
  server.send ( 404, "text/plain", message );
}

char* listStatusJSON() {
  char json[255];
  snprintf(json, sizeof(json), "{\"mode\":%d, \"ws2812fx_mode\":%d, \"ws2812fx_mode_name\":\"%s\", \"speed\":%d, \"brightness\":%d, \"color\":[%d, %d, %d]}", mode, strip.getMode(), strip.getModeName(strip.getMode()), ws2812fx_speed, brightness, main_color.red, main_color.green, main_color.blue);
  return json;
}

void getStatusJSON() {
  server.send ( 200, "application/json", listStatusJSON() );
}

String listModesJSON() {
  String modes = "[";
  for(uint8_t i=0; i < strip.getModeCount(); i++) {
    modes += "{\"mode\":";
    modes += i;
    modes += ", \"name\":\"";
    modes += strip.getModeName(i);
    modes += "\"},";
  }
  modes += "{}]";
  return modes;
}

void getModesJSON() {
  server.send ( 200, "application/json", listModesJSON() );
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  switch (type) {
    case WStype_DISCONNECTED:
      DBG_OUTPUT_PORT.printf("WS: [%u] Disconnected!\n", num);
      break;

    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        DBG_OUTPUT_PORT.printf("WS: [%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, "Connected");
      }
      break;

    case WStype_TEXT:
      DBG_OUTPUT_PORT.printf("WS: [%u] get Text: %s\n", num, payload);

      // # ==> Set main color
      if (payload[0] == '#') {
        handleSetMainColor(payload);
        DBG_OUTPUT_PORT.printf("Set main color to: [%u] [%u] [%u]\n", main_color.red, main_color.green, main_color.blue);
        webSocket.sendTXT(num, "OK");
      }

      // ? ==> Set speed
      if (payload[0] == '?') {
        uint8_t d = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
        ws2812fx_speed = constrain(d, 0, 255);
        strip.setSpeed(ws2812fx_speed);
        DBG_OUTPUT_PORT.printf("WS: Set speed to: [%u]\n", ws2812fx_speed);
        webSocket.sendTXT(num, "OK");
      }

      // % ==> Set brightness
      if (payload[0] == '%') {
        uint8_t b = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
        brightness = ((b >> 0) & 0xFF);
        DBG_OUTPUT_PORT.printf("WS: Set brightness to: [%u]\n", brightness);
        strip.setBrightness(brightness);
        webSocket.sendTXT(num, "OK");
      }

      // * ==> Set main color and light all LEDs (Shortcut)
      if (payload[0] == '*') {
        handleSetAllMode(payload);
        webSocket.sendTXT(num, "OK");
      }

      // ! ==> Set single LED in given color
      if (payload[0] == '!') {
        handleSetSingleLED(payload);
        webSocket.sendTXT(num, "OK");
      }

      // = ==> Activate named mode
      if (payload[0] == '=') {
        // we get mode data
        String str_mode = String((char *) &payload[0]);

        handleSetNamedMode(str_mode);

        DBG_OUTPUT_PORT.printf("Activated mode [%u]!\n", mode);
        webSocket.sendTXT(num, "OK");
      }

      // $ ==> Get status Info.
      if (payload[0] == '$') {
        DBG_OUTPUT_PORT.printf("Get status info.");
        
        String json = listStatusJSON();
        DBG_OUTPUT_PORT.println(json);
        webSocket.sendTXT(num, json);
      }
      
      // ~ ==> Get WS2812 modes.
      if (payload[0] == '~') {
        DBG_OUTPUT_PORT.printf("Get WS2812 modes.");
        
        String json = listModesJSON();
        DBG_OUTPUT_PORT.println(json);
        webSocket.sendTXT(num, json);
      }
      
      // / ==> Set WS2812 mode.
      if (payload[0] == '/') {
        handleSetWS2812FXMode(payload);
        webSocket.sendTXT(num, "OK");
      }
      break;
  }
}

void checkForRequests() {
  webSocket.loop();
  server.handleClient();
}


// ***************************************************************************
// MQTT callback / connection handler
// ***************************************************************************
#ifdef ENABLE_MQTT
  void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    DBG_OUTPUT_PORT.print("Message arrived [");
    DBG_OUTPUT_PORT.print(topic);
    DBG_OUTPUT_PORT.print("] ");
    for (int i = 0; i < length; i++) {
      DBG_OUTPUT_PORT.print((char)payload[i]);
    }
    DBG_OUTPUT_PORT.println();

    // # ==> Set main color
    if (payload[0] == '#') {
      handleSetMainColor(payload);
      DBG_OUTPUT_PORT.printf("MQTT: Set main color to [%u] [%u] [%u]\n", main_color.red, main_color.green, main_color.blue);
      mqtt_client.publish(mqtt_outtopic, "OK");
    }

    // ? ==> Set speed
    if (payload[0] == '?') {
      uint8_t d = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
      ws2812fx_speed = constrain(d, 0, 255);
      strip.setSpeed(ws2812fx_speed);
      DBG_OUTPUT_PORT.printf("MQTT: Set speed to [%u]\n", ws2812fx_speed);
      mqtt_client.publish(mqtt_outtopic, "OK");
    }

    // % ==> Set brightness
    if (payload[0] == '%') {
      uint8_t b = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
      brightness = constrain(b, 0, 255);
      strip.setBrightness(brightness);
      DBG_OUTPUT_PORT.printf("MQTT: Set brightness to [%u]\n", brightness);
      mqtt_client.publish(mqtt_outtopic, "OK");
    }

    // * ==> Set main color and light all LEDs (Shortcut)
    if (payload[0] == '*') {
      handleSetAllMode(payload);
      DBG_OUTPUT_PORT.printf("MQTT: Set main color and light all LEDs [%u]\n", payload);
      mqtt_client.publish(mqtt_outtopic, "OK");
    }

    // ! ==> Set single LED in given color
    if (payload[0] == '!') {
      handleSetSingleLED(payload);
      DBG_OUTPUT_PORT.printf("MQTT: Set single LED in given color [%u]\n", payload);
      mqtt_client.publish(mqtt_outtopic, "OK");
    }
    
    // = ==> Activate named mode
    if (payload[0] == '=') {
      String str_mode = String((char *) &payload[0]);
      handleSetNamedMode(str_mode);
      DBG_OUTPUT_PORT.printf("MQTT: Activate named mode [%u]\n", payload);
      mqtt_client.publish(mqtt_outtopic, "OK");
    }

    // $ ==> Get status Info.
    if (payload[0] == '$') {
      DBG_OUTPUT_PORT.printf("MQTT: Get status info.");
      mqtt_client.publish(mqtt_outtopic, listStatusJSON());
    }

    // ~ ==> Get WS2812 modes.
    // TODO: Fix this, doesn't return anything. Too long?
    if (payload[0] == '~') {
      DBG_OUTPUT_PORT.printf("MQTT: Get WS2812 modes.");
      String json_modes = listModesJSON();
      DBG_OUTPUT_PORT.printf(json_modes.c_str());
      
      mqtt_client.publish(mqtt_outtopic, json_modes.c_str());
    }
    
    // / ==> Set WS2812 mode.
    if ((char)payload[0] == '/') {
      handleSetWS2812FXMode(payload);
      DBG_OUTPUT_PORT.printf("MQTT: Set WS2812 mode [%u]\n", payload);
      mqtt_client.publish(mqtt_outtopic, "OK");
    }
  }
  
  void mqtt_reconnect() {
    // Loop until we're reconnected
    while (!mqtt_client.connected()) {
      DBG_OUTPUT_PORT.print("Attempting MQTT connection...");
      // Attempt to connect
      if (mqtt_client.connect("ESP8266Client")) {
        DBG_OUTPUT_PORT.println("connected");
        // Once connected, publish an announcement...
        char * message = new char[18 + strlen(HOSTNAME) + 1];
        strcpy(message, "McLighting ready: ");
        strcat(message, HOSTNAME);
        mqtt_client.publish(mqtt_outtopic, message);
        // ... and resubscribe
        mqtt_client.subscribe(mqtt_intopic);
      } else {
        DBG_OUTPUT_PORT.print("failed, rc=");
        DBG_OUTPUT_PORT.print(mqtt_client.state());
        DBG_OUTPUT_PORT.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
  }  
#endif
