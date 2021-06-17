#include <avr/wdt.h>
//pins
#define ignition A1
#define crank A2
#define backlight A3
#define button A4
#define sens A5

//constants
#define timerDuration 20
#define treshold_hi 890
#define treshold_lo 860
//#define debug

//variables
bool shortClick, longClick;
bool iState, bState;
unsigned long longTime;

void setup() {
#ifdef debug
  Serial.begin(9600);
#endif
  pinMode(ignition, OUTPUT);
  pinMode(crank, OUTPUT);
  pinMode(backlight, OUTPUT);
  pinMode(button, INPUT);
  pinMode(sens, INPUT);

  digitalWrite(ignition, 0);
  digitalWrite(crank, 0);
  digitalWrite(backlight, 0);

  shortClick = longClick = iState = bState = false;
}

void loop() {
  if (digitalRead(button)) {
    delay(25);
    longTime = millis() + 300;
    if (digitalRead(button)) {
      shortClick = true;
      while (digitalRead(button)) {
        if (millis() > longTime) {
          shortClick = false;
          longClick = true;
          break;
        }
      }
    }
  }

  if (shortClick) {
    shortClick = false;
    iState = !iState;
    digitalWrite(ignition, iState);
    if (!iState) { //reset uC
      reboot();
    }
#ifdef debug
    Serial.println("short");
#endif
  }

  if (longClick) {
    longClick = false;
#ifdef debug
    Serial.println("longClick start");
#endif
    if (iState & bState) {
      digitalWrite(crank, 1);
    }
    else if (iState & !bState) {
#ifdef debug
      Serial.println("timer activated");
#endif
      turboTimer();
    }
    while (digitalRead(button));
    digitalWrite(crank, 0);
#ifdef debug
    Serial.println("longClick end");
#endif
    delay(400);
  }
  
  if (bState && analogRead(sens) > treshold_hi) {
    bState = false;
  }
  else if (!bState && analogRead(sens) < treshold_lo) {
    bState = true;
  }
  digitalWrite(backlight, bState & iState);
}

void reboot() {
  wdt_disable();
  wdt_enable(WDTO_15MS);
  while (true);
}

void turboTimer() {
  bState = true;
  bool irq = true;
  for (int i = 0; i < timerDuration; i++) {
    if (!digitalRead(button)) {
      irq = false;
    }
    if (i == 7 && irq) {
#ifdef debug
      Serial.println("SuperLongClick start");
#endif
      digitalWrite(crank, 1);
      while (digitalRead(button));
      digitalWrite(crank, 0);
#ifdef debug
      Serial.println("SuperLongClick end");
#endif
      delay(400);
      return;
    }
    digitalWrite(backlight, bState);
    delay(500);
    digitalWrite(backlight, !bState);
    delay(500);
  }
  digitalWrite(ignition, 0);
  reboot();
}
