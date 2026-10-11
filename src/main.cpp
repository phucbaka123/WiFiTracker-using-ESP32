#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "secrets.h"
#include <WiFi.h>

struct Button {
  int pin;
  bool lastState;
  unsigned long lastChange;
};

Button btnMode = {27, HIGH, 0};
Button btnUP = {26, HIGH, 0};
Button btnDown = {25, HIGH, 0};
Button btnReset = {33, HIGH, 0};

LiquidCrystal_I2C lcd(0x27, 16,2);

const unsigned long DEBOUNCE_MS = 40;
int networkCount;
int bestChannel = 1;
int channelCount[14];
int channelWeight[14];

bool wasPressed(Button &btn){
  bool now = digitalRead(btn.pin);
  if(now != btn.lastState && millis() - btn.lastChange > DEBOUNCE_MS ){
    btn.lastChange = millis();
    btn.lastState = now;
    if (now == LOW) return true;
  }
  return false;
}
void rescan(){
  WiFi.scanDelete();
  networkCount = WiFi.scanNetworks();
  for(int i = 0; i < 14; i++) { channelWeight[i] = 0; }
  for(int i = 0; i < 14; i++) { channelCount[i] = 0; }
  bestChannel = 1;

  int back;
  int front;
  
  for(int n = 0; n < networkCount; n++){
    int ch = WiFi.channel(n);
    if (ch < 1 || ch > 13) continue;
    channelCount[ch]++;

    back = ch - 3;
    if(back < 1) back = 1;
    front = ch + 3;
    if(front > 13) front = 13;

    while(back <= front){
      int index = (ch - back);
      if(index < 0) index = index * (-1);
      channelWeight[back] = channelWeight[back] + 4 - index;
      back++;
    }
  }
  int i = 1;
  while(i < 12){
    if(channelWeight[i] < channelWeight[bestChannel]){
      bestChannel = i;
    }
    i = i + 5;
  }

}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(btnMode.pin, INPUT_PULLUP);
  pinMode(btnUP.pin, INPUT_PULLUP);
  pinMode(btnDown.pin, INPUT_PULLUP);
  pinMode(btnReset.pin, INPUT_PULLUP);
  Wire.begin(21,22);
  lcd.init();
  lcd.backlight();
  lcd.print("Connecting...");
  
  rescan();

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  lcd.clear();
}

int screen = 0;
int selected = 0;
int selectedChannel = 1;
unsigned long lastDraw = 0;
void loop() {
  // put your main code here, to run repeatedly:

  //RESET BUTTON
  if(wasPressed(btnReset)){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Scanning...");
    rescan();
    selected = 0;
    selectedChannel = 1;
    lastDraw = 0;
    lcd.clear();
  } 

  if (wasPressed(btnMode)){
    screen++;
    if(screen > 2) screen = 0;
    lastDraw = 0;
    lcd.clear();
  }

  bool UpPressed = wasPressed(btnUP);
  bool DownPressed = wasPressed(btnDown);
  if (UpPressed && screen == 1 && networkCount > 0){
    selected--;
    if(selected < 0){
      selected = networkCount - 1;
    }
    lcd.clear();
  }
  if (DownPressed && screen == 1 && networkCount > 0){
    selected++;
    if(selected >= networkCount){
      selected = 0;
    }
    lcd.clear();
  }  
  
  if (UpPressed && screen == 2){
    selectedChannel--;
    if(selectedChannel < 1){
    selectedChannel = 13;
    }
    lcd.clear();
    }
  if (DownPressed && screen == 2){
    selectedChannel++;
    if(selectedChannel > 13){
      selectedChannel = 1;
    }
    lcd.clear();
  }
  

  

  if(millis() - lastDraw >= 200){
    lastDraw = millis();
    if(screen == 0){ //DEFAULT MODE SHOWING THE SET UP CONNECTION
      int rssi = WiFi.RSSI();
      lcd.setCursor(0, 0);
      lcd.print(WiFi.SSID());
      lcd.setCursor(0,1);
      lcd.print("RSSI: ");
      lcd.print(rssi);
      lcd.print(" dBm ");
    } else if (screen == 1){ //SECOND MODE BROWSING EACH NETWORKS
      if(networkCount <= 0){
        lcd.setCursor(0,0);
        lcd.print("No Networks...");
      } else {
        lcd.setCursor(0, 0);
        lcd.print(WiFi.SSID(selected));
        lcd.setCursor(0,1);
        lcd.print("RSSI: ");
        lcd.print(WiFi.RSSI(selected));
        lcd.print(" dBm ");
      }

    } else if (screen == 2) { //THIRD MODE FOR BROWSING EACH CHANNELS
        
      lcd.setCursor(0, 0);
      lcd.print("Ch");
      lcd.print(selectedChannel);
      lcd.print(": ");
      lcd.print(channelCount[selectedChannel]);
      lcd.print(" W: ");
      lcd.print(channelWeight[selectedChannel]);

      lcd.setCursor(0, 1);
      lcd.print("Best Channel: ");
      lcd.print(bestChannel);
    }
  }
}
