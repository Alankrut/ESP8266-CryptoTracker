#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "config.h"

ESP8266WiFiMulti WiFiMulti;
LiquidCrystal_I2C lcd(0x27, 20, 4);

const int MILLI_IN_SEC = 1000;
const char* host = "min-api.cryptocompare.com";
const int port = 443;

void setup()
{
  Serial.begin(9600);
  Wire.begin(2, 0);

  // Connect to wifi
  WiFiMulti.addAP(ssid, password);

  // initialize the LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();
}

/**
 * Invoked when the system wakes. If wifi is connected. gets the prices for the requested coins
 */
void loop()
{
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    Serial.println("Checking prices...");

    for(int i=0; i < 2; i++) {
      String coinTicker = String(arrCoins[i][0]);
      String coinFullName = String(arrCoins[i][1]);
      int coinPosition = atoi(String(arrCoins[i][2]).c_str());

      lookupCoin(coinTicker,coinFullName,coinPosition);
    }
  } else {
    Serial.println("Waiting on wifi connection...");
  }
  delay(delaySecs * MILLI_IN_SEC);
}

/**
 * Calls the API to get the value of the coin as a json, which will 
 * then be parsed and printed to the screen
 */
void lookupCoin(String strCoin, String strCoinFullName, int numLine) {
  StaticJsonBuffer<200> jBuffer;
  JsonObject& data = jBuffer.parseObject(getCoinValue(strCoin));

  if (!data.success()) {
    Serial.println("An error occured parsing the response");
    return;
  }

  float value = data[currency];
  Serial.println(strCoinFullName + ": $" + value);
  printCoinDetails(numLine,strCoinFullName,value);
}

/**
 * Print the coin details to the LCD screen
 */
void printCoinDetails(int numLine, String strCoinFullName, float value){
  lcd.setCursor(0, numLine);
  lcd.print(strCoinFullName); // Start Print text to Line 1
  lcd.setCursor(strCoinFullName.length() + 1, numLine);
  lcd.print("$" + String(value));  
}

/**
 * Communicates with the API to get the value
 */
String getCoinValue(String strCoin) {
  WiFiClientSecure client;
  Serial.println("Connecting...");
  Serial.println("Asking for " + strCoin);

  if (!client.connect(host, port)) {
    Serial.println("Connection Error");
    return "Connection Error";
  }

  String url = "/data/price?fsym=" + strCoin + "&tsyms=" + currency;

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }

  String line = client.readStringUntil('\n');
  line = client.readStringUntil('\n');
  Serial.println("Disconnecting...");
  return line;
}
