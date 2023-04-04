#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>

#include "./esppl_functions.h"


#define LIST_SIZE 2

uint8_t friendmac[LIST_SIZE][ESPPL_MAC_LEN] = {
   {0xBC, 0xE1, 0x43, 0x81, 0x8E, 0x74},
   {0xB0, 0xE5, 0xF9, 0x51, 0xC9, 0x38}
  };

String friendname[LIST_SIZE] = {
   "Mert",
   "Hamza"
  };

unsigned long timer[LIST_SIZE]={0,0};
int sayac=0;


// Replace with your network credentials
const char* ssid = "FIKO ONLINE";
const char* password = "LMKP2023";

// Initialize Telegram BOT
#define BOTtoken "YOUT_BOT_TOKEN_HERE"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "-1001836673208"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

const int ledPin = 2;
bool ledState = LOW;

int gelensayisi=0;
int cooldown = 0;
String gidenadi="0";

bool gelenvar;
String gelenler[LIST_SIZE]; 

bool maccmp(uint8_t *mac1, uint8_t *mac2) {
  for (int i=0; i < ESPPL_MAC_LEN; i++) {
    if (mac1[i] != mac2[i]) {
      return false;
    }
  }
  return true;
}




void cb(esppl_frame_info *info) {
  for (int i=0; i<LIST_SIZE; i++) {
    if (maccmp(info->sourceaddr, friendmac[i]) || maccmp(info->receiveraddr, friendmac[i])) {
      Serial.printf("\n%s is here! :)", friendname[i].c_str());
      sayac++;
      gelensayisi=sayac;
      gelenler[i] = friendname[i].c_str();
      timer[i]=millis();
      cooldown = 1000; // here we set it to 1000 if we detect a packet that matches our list
    }
    if(millis()-timer[i]>20000 && timer[i]!=0){
      sayac--;
      gelensayisi--;
      gidenadi=gelenler[i];
     // gelenler[i]="";
    }

      else { // this is for if the packet does not match any we are tracking
        if (cooldown > 0) {
          cooldown--; } //subtract one from the cooldown timer if the value of "cooldown" is more than one
          } } 
    
}






// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    Serial.println(chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/basla") {
      String welcome = "Hoşgeldin, " + from_name + " Agam\n";
      welcome += "Emret bana\n\n";
      welcome += "/isikac ıçığı açarım.\n";
      welcome += "/isikkapat ışığı kapatırım.\n";
      welcome += "/durum ışığın durumunu öğrenmek için.\n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/isikkac") {
      bot.sendMessage(chat_id, "Işık açık agam.", "");
      ledState = LOW;
      digitalWrite(ledPin, ledState);
    }
    
    if (text == "/isikkapat") {
      bot.sendMessage(chat_id, "Işık kapatıldı agam.", "");
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
    }
    
    if (text == "/durum") {
      if (digitalRead(ledPin)){
        bot.sendMessage(chat_id, "Işık kapalı", "");
      }
      else{
        bot.sendMessage(chat_id, "Işık açık", "");
      }
    }
  }
}

void netbaglan(){
  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif
    // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

}

void netchecktg(){
  netbaglan();
  for(int i = 0; i < 100; i++){
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  Serial.println(numNewMessages);
    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
  }
}
void sniffmode(){
  for(int i = 0; i < 20; i++){
esppl_init(cb);
esppl_sniffing_start();
Serial.println("sniffinge girdim");
    for (int i = ESPPL_CHANNEL_MIN; i <= ESPPL_CHANNEL_MAX; i++ ) {
      esppl_set_channel(i);
      esppl_process_frames();
}
  }
esppl_sniffing_enabled = false;
}

void esppl_deactive() {
  wifi_promiscuous_enable(true);
  wifi_promiscuous_enable(false);
  esppl_sniffing_enabled = true;
}



void loop() {
netchecktg();
if(gelensayisi>0){
  for(int i=0;i<LIST_SIZE;i++){
    if((millis()-timer[i])<5000){
        bot.sendMessage(String(-1001836673208), gelenler[i] + " Online.", "");
        timer[i]=0;
    }
}
}
if(gidenadi!="0"){
bot.sendMessage(String(-1001836673208), gidenadi + " Offline.", "");
gidenadi="0";
}
sniffmode();
esppl_deactive();
  }