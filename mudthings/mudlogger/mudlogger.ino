#include <WiFi.h>
#include <HTTPClient.h>

// Defines SSID, PWD, LOG_URL
#include <secrets.h>

void setup() {
    Serial.begin(9600);

    WiFi.begin(SSID, PWD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.print("connected!");
}

void loop() {
    delay(1000);

    String url = LOG_URL;
    String data = "hello from mudthing :)";

    HTTPClient http;
    http.begin(url.c_str());
    http.POST(data);
    http.end();
}

