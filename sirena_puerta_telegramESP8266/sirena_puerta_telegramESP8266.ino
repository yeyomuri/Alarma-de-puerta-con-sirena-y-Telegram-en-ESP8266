/* 
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-door-status-telegram/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Set GPIOs for LED and reedswitch
const int reedSwitch = 3;
const int led = 0; //optional

// Detects whenever the door changed state
bool changeState = false;

// Holds reedswitch state (1=opened, 0=close)
bool state;
String doorState;

// Auxiliary variables (it will only detect changes that are 1500 milliseconds apart)
unsigned long previousMillis = 0; 
const long interval = 1500;


const char* ssid = "TP-LINK_877C";
const char* password = "20432447";

// Initialize Telegram BOT
#define BOTtoken "5328238426:AAEE42q-0a2YenEfyM14O5GTmd5RDDXi52k"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "5146763659"


X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Runs whenever the reedswitch changes state
ICACHE_RAM_ATTR void changeDoorStatus() {
  Serial.println("State changed");
  changeState = true;
}

//-------------------------------------------------------- Function messages from telegram ----------------------------------------------------------------------------
int ledStatus = 0;

String handleNewMessages(int numNewMessages)
{
  String text;
  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "")
      from_name = "Sandro";

    if (text == "/on")
    {
      ledStatus = 1;
      bot.sendMessage(chat_id, "La sirena esta activada", "");
    }

    if (text == "/off")
    {
      ledStatus = 0;
      bot.sendMessage(chat_id, "La sirena esta desactivada", "");
    }

    if (text == "/estado")
    {
      if (ledStatus)
      {
        bot.sendMessage(chat_id, "La sirena esta activada", "");
      }
      else
      {
        bot.sendMessage(chat_id, "La sirena esta desactivada", "");
      }
    }
  }
  return text;
}
//-------------------------------------------------------- Finish function messages from telegram ----------------------------------------------------------------------------

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
    // Read the current door state
  pinMode(reedSwitch, INPUT);

  // Set LED state to match door state
  pinMode(led, OUTPUT);
  digitalWrite(led, true);
  
  configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
  client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected"); 
     
  String welcome = "Hola. Bienvenido a Smart Garage, " ".\n";
      welcome += "Sigue las instrucciones para controlar tu alarma.\n";
      welcome += "/on : Para activa la sirena.\n";
      welcome += "/off : Para desactivar la sirena\n";
      welcome += "/estado : Para mirar el estado de tu porton";

  bot.sendMessage(CHAT_ID, welcome, "");
}

const unsigned long BOT_MTBS = 1000; // mean time between scan messages
unsigned long bot_lasttime; // last time messages' scan has been done
String message = ""; 

bool stateClose = true;
bool stateOpen =true;
void loop() {

  //---------------------------------------------------------------- Read from Telegram -----------------------------------------------------------
    if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("got response");
      message = handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
  //---------------------------------------------------------------- Finish read from Telegram -----------------------------------------------------------
  if(message == "/on" && message != "/estado"){
    state = digitalRead(reedSwitch);
    if(state){
      if(stateClose){
        digitalWrite(led, true);
        doorState = "cerrada";
        stateClose = false;
        //Send notification
        bot.sendMessage(CHAT_ID, "La puerta esta " + doorState, "");
      }
      stateOpen = true;
    }
    else{  
      if(stateOpen){
        digitalWrite(led, false);
        doorState = "abierta";
        stateOpen = false;
        //Send notification
        bot.sendMessage(CHAT_ID, "¡CUIDADO! La puerta esta " + doorState, "");
        delay(30000);
        digitalWrite(led, true);
      }
      stateClose = true;
    }
  }
}
