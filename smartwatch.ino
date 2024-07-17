#include <Wire.h>
#include <SoftwareSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <UniversalTelegramBot.h>
#include <Time.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define MY_WIFI_SSID "S-nam Lhendup"
#define PASSWORD "12345678"
#define serverIP "172.20.10.2"
#define serverPort 80
#define BLUE_PIN D8   //
#define RED_PIN D10   //
#define GREEN_PIN D9  //
#define COMMON_ANODE false

#define BUTTON D7  //
#define BUTTON2 D0
#define MOTOR D2
#define BUZZER D3
#define SDA_PIN D5
#define SCL_PIN D4

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

String chatId = "1518322415";
String BOTtoken = "7056551918:AAHTLJdIUikX3InX0bZ9bbn8rrlkBBjc238";

WiFiClientSecure clientTCP;
UniversalTelegramBot bot(BOTtoken, clientTCP);
unsigned long bot_lasttime;
bool buttonPressed = false;
unsigned long buttonPressStartTime = 0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

const unsigned long BOT_MTBS = 1000;

WiFiServer server(serverPort);

bool redPillActive = false;
bool greenPillActive = false;

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;) {}
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);

  pinMode(BUZZER, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(MOTOR, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);

  WiFi.begin(MY_WIFI_SSID, PASSWORD);
  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    setColor(255, 255, 255);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  server.begin();
  Serial.println("Server started");
  bot.sendMessage(chatId, "WiFi connected 2");
  clientesp();

  display.setCursor(0, 0);
  display.setTextSize(2);
  display.println("WiFi Connected!");
  display.display();
  delay(2000);
  display.clearDisplay();

  setColor(0, 255, 0);
  digitalWrite(MOTOR, 1);
  delay(300);
  setColor(0, 0, 0);
  digitalWrite(MOTOR, 0);
}



void loop() {
  BUT_read();
  TIME();
  telegram();
  if (digitalRead(BUTTON2) == LOW) {
    if (redPillActive) {
      redPillActive = true;
      setColor(0, 0, 0);
      digitalWrite(MOTOR, 0);
      Serial.println("Pressed1");
    }
    if (greenPillActive) {
      greenPillActive = false;
      setColor(0, 0, 0);
      digitalWrite(MOTOR, 0);
      Serial.println("Pressed2");
    }
  }
}
void telegram() {
  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}

void setColor(int red, int green, int blue) {
  if (COMMON_ANODE) {
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
  }
  analogWrite(RED_PIN, red);
  analogWrite(GREEN_PIN, green);
  analogWrite(BLUE_PIN, blue);
}


void BUT_read() {
  if (digitalRead(BUTTON) == LOW) {
    if (!buttonPressed) {
      buttonPressed = true;
      buttonPressStartTime = millis();
    }
  } else {
    if (buttonPressed) {
      if (millis() - buttonPressStartTime >= 2000) {
        bot.sendMessage(chatId, "I need help!!!");
        display.clearDisplay();
        display.display();
        display.setCursor(0, 0);
        display.setTextSize(2);
        display.println("");
        display.println("Need HELP");
        display.println("");
        display.display();
        delay(2000);
        display.clearDisplay();
      }
      buttonPressed = false;
    }
  }
}

void TIME() {
  Wire.begin(SDA_PIN, SCL_PIN);
  timeClient.begin();
  timeClient.setTimeOffset(6 * 3600);

  display.clearDisplay();
  display.setCursor(0, 0);

  timeClient.update();

  int hours = timeClient.getHours();
  int minutes = timeClient.getMinutes();

  String formattedTime = (hours < 10 ? "0" : "") + String(hours) + ":" + (minutes < 10 ? "0" : "") + String(minutes);

  display.println("");
  display.println("BHT Time:   " + formattedTime);
  display.println("");
  display.display();

  delay(1000);
}



void handleNewMessages(int numNewMessages) {
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    Serial.println(text);
    String from_name = bot.messages[i].from_name;
    if (from_name == "")
      from_name = "Guest";

    if (text == "Hi") {
      bot.sendMessage(chat_id, "Hi, Remind to take /REDPILL or /GREENPILL ?", "", 0);
    }
    if (text == "/REDPILL") {
      redPillActive = true;
      display.clearDisplay();
      display.display();
      display.setCursor(0, 0);
      display.setTextSize(2);
      display.println("");
      display.println("Take RED pill");
      display.println("");
      display.display();
      delay(2000);
      display.clearDisplay();

      while (digitalRead(BUTTON2) == HIGH) {
        digitalWrite(MOTOR, 1);
        delay(100);
      }
      digitalWrite(MOTOR, 0);

      bot.sendMessage(chat_id, "RED PILL is Taken", "", 0);
    }

    if (text == "/GREENPILL") {
      greenPillActive = true;
      display.clearDisplay();
      display.display();
      display.setCursor(0, 0);
      display.setTextSize(2);
      display.println("");
      display.println("Take GREEN pill");
      display.println("");
      display.display();
      delay(2000);
      display.clearDisplay();

      while (digitalRead(BUTTON2) == HIGH) {
        digitalWrite(MOTOR, 1);
        delay(100);
      }

      digitalWrite(MOTOR, 0);
      bot.sendMessage(chat_id, "GREEN PILL is Taken", "", 0);
    }
  }
}

void clientesp() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("New client connected");
    while (client.connected()) {
      if (client.available()) {
        String request = client.readStringUntil('\r');
        Serial.println("Received: " + request);

        if (request.equals("Notification")) {
          setColor(0, 0, 255);

          while (digitalRead(BUTTON2) == HIGH) {
            digitalWrite(MOTOR, 1);
            delay(100);
          }
          digitalWrite(MOTOR, 0);
          setColor(0, 0, 0);
        }
        if (request.equals("door")) {
          setColor(0, 255, 0);

          while (digitalRead(BUTTON2) == HIGH) {
            digitalWrite(MOTOR, 1);
            delay(100);
          }
          digitalWrite(MOTOR, 0);
          setColor(0, 0, 0);
        }
      }
    }
  }
}