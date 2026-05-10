// Library to install : Adafruit DotStar

#include <Adafruit_DotStar.h>
#include <SPI.h>

#define NB_LED 30

#define DATA_PIN 6
#define CLOCK_PIN 7

#define BTN_RED   2
#define BTN_GREEN 3

Adafruit_DotStar ledStrip(NB_LED, DATA_PIN, CLOCK_PIN, DOTSTAR_BGR);

int enemyPos = NB_LED - 1;
bool redEnemy = true;

bool shotOn = false;
int shotPos = 0;
bool redShot = true;

unsigned long oldTime = 0;

int speedGame = 120;
int minSpeed = 25;
int speedBoost = 8;

int score = 0;

bool oldRedState = HIGH;
bool oldGreenState = HIGH;

void setup() {

  pinMode(BTN_RED, INPUT_PULLUP);
  pinMode(BTN_GREEN, INPUT_PULLUP);

  ledStrip.begin();
  ledStrip.setBrightness(50);
  ledStrip.show();

  randomSeed(analogRead(A0));

  newGame();
}

void loop() {

  checkButtons();

  unsigned long currentTime = millis();

  if (currentTime - oldTime >= speedGame) {

    oldTime = currentTime;

    ledStrip.clear();

    if (redEnemy) {
      ledStrip.setPixelColor(enemyPos, 255, 0, 0);
    }
    else {
      ledStrip.setPixelColor(enemyPos, 0, 255, 0);
    }

    if (shotOn) {

      if (redShot) {
        ledStrip.setPixelColor(shotPos, 255, 0, 0);
      }
      else {
        ledStrip.setPixelColor(shotPos, 0, 255, 0);
      }
    }

    ledStrip.show();

    if (shotOn && shotPos >= enemyPos) {

      if (redShot == redEnemy) {
        win();
      }
      else {
        lose();
      }

      return;
    }

    enemyPos--;

    if (shotOn) {
      shotPos++;
    }

    if (enemyPos < 0) {
      lose();
      return;
    }

    if (shotPos >= NB_LED) {
      shotOn = false;
    }
  }
}

void checkButtons() {

  bool redState = digitalRead(BTN_RED);
  bool greenState = digitalRead(BTN_GREEN);

  if (oldRedState == HIGH && redState == LOW) {
    shoot(true);
  }

  if (oldGreenState == HIGH && greenState == LOW) {
    shoot(false);
  }

  oldRedState = redState;
  oldGreenState = greenState;
}

void shoot(bool red) {

  if (!shotOn) {

    shotOn = true;
    shotPos = 0;
    redShot = red;
  }
}

void win() {

  score++;

  if (speedGame > minSpeed) {
    speedGame -= speedBoost;
  }

  for (int i = 0; i < NB_LED; i++) {
    ledStrip.setPixelColor(i, 255, 255, 255);
  }

  ledStrip.show();
  delay(100);

  ledStrip.clear();
  ledStrip.show();

  delay(80);

  newGame();
}

void lose() {

  speedGame = 120;
  score = 0;

  for (int i = 0; i < NB_LED; i++) {
    ledStrip.setPixelColor(i, 255, 0, 0);
  }

  ledStrip.show();
  delay(250);

  ledStrip.clear();
  ledStrip.show();

  delay(150);

  newGame();
}

void newGame() {

  enemyPos = NB_LED - 1;

  redEnemy = random(0, 2);

  shotOn = false;
  shotPos = 0;
}