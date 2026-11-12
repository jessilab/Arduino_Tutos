
#include <Adafruit_DotStar.h>
#include <SPI.h>

// Jessi Lab Tuto Kamehameha

#define TRIG_PIN 4
#define ECHO_PIN 5
#define DATA_PIN 6
#define CLOCK_PIN 7
#define NB_LED 30

Adafruit_DotStar strip(NB_LED, DATA_PIN, CLOCK_PIN, DOTSTAR_BGR);

const float DISTANCE_DETECTION = 60.0;
const float DISTANCE_EXPLOSION = 10.0;
const float DISTANCE_REARMEMENT = 12.0;

const unsigned long TEMPS_CHARGE = 1000;
const unsigned long TEMPS_EXPLOSION = 650;
const unsigned long PERTE_SIGNAL = 350;
const unsigned long INTERVALLE_MESURE = 35;
const unsigned long TEMPS_REARMEMENT = 150;

enum Etat {
  REPOS,
  CHARGE,
  EXPLOSION,
  ATTENTE_REARMEMENT
};

Etat etat = REPOS;

unsigned long debutCharge = 0;
unsigned long debutExplosion = 0;
unsigned long dernierePresence = 0;
unsigned long derniereMesure = 0;
unsigned long debutRearmement = 0;

float distanceFiltree = -1;

float mesurerDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duree = pulseIn(
    ECHO_PIN,
    HIGH,
    15000UL
  );

  if (duree == 0) return -1;

  return duree * 0.0343 / 2.0;
}

void eteindreLED() {
  strip.clear();
  strip.show();
}

void afficherCharge(unsigned long duree) {
  float progression = min(
    1.0f,
    (float)duree / TEMPS_CHARGE
  );

  int nombre = max(
    1,
    (int)(progression * NB_LED)
  );

  strip.clear();

  for (int i = 0; i < nombre; i++) {
    strip.setPixelColor(
      i,
      strip.Color(0, 65, 220)
    );
  }

  strip.setPixelColor(
    nombre - 1,
    strip.Color(130, 220, 255)
  );

  strip.show();
}

void afficherExplosion(unsigned long duree) {
  strip.clear();

  if (duree < 120) {
    for (int i = 0; i < NB_LED; i++) {
      strip.setPixelColor(
        i,
        strip.Color(180, 230, 255)
      );
    }
  }
  else if (duree < 420) {
    int position = map(
      duree,
      120,
      420,
      0,
      NB_LED + 10
    );

    for (int j = 0; j < 10; j++) {
      int index = position - j;

      if (index >= 0 && index < NB_LED) {
        int intensite = 255 - j * 20;

        strip.setPixelColor(
          index,
          strip.Color(
            0,
            intensite / 2,
            intensite
          )
        );
      }
    }
  }
  else {
    int intensite = map(
      duree,
      420,
      TEMPS_EXPLOSION,
      120,
      0
    );

    intensite = constrain(
      intensite,
      0,
      120
    );

    for (int i = 0; i < NB_LED; i++) {
      strip.setPixelColor(
        i,
        strip.Color(
          0,
          intensite / 2,
          intensite
        )
      );
    }
  }

  strip.show();
}

void commencerCharge(unsigned long maintenant) {
  etat = CHARGE;
  debutCharge = maintenant;
  dernierePresence = maintenant;

  Serial.println("CHARGE");
}

void commencerExplosion(unsigned long maintenant) {
  etat = EXPLOSION;
  debutExplosion = maintenant;

  Serial.println("FIRE");
}

void annulerCharge() {
  etat = REPOS;
  eteindreLED();

  Serial.println("RESET");
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(TRIG_PIN, LOW);

  strip.begin();
  strip.setBrightness(35);
  eteindreLED();

  delay(500);

  Serial.println("READY");
}

void loop() {
  unsigned long maintenant = millis();

  if (etat == EXPLOSION) {
    unsigned long duree =
      maintenant - debutExplosion;

    if (duree < TEMPS_EXPLOSION) {
      afficherExplosion(duree);
    } else {
      eteindreLED();
      etat = ATTENTE_REARMEMENT;
      debutRearmement = 0;
    }
  }

  if (
    maintenant - derniereMesure <
    INTERVALLE_MESURE
  ) {
    return;
  }

  derniereMesure = maintenant;

  float distance = mesurerDistance();

  bool mesureValide =
    distance >= 2.0 &&
    distance <= 300.0;

  if (mesureValide) {
    if (distanceFiltree < 0) {
      distanceFiltree = distance;
    } else {
      distanceFiltree =
        0.55f * distance +
        0.45f * distanceFiltree;
    }
  } else {
    distanceFiltree = -1;
  }

  bool presence =
    mesureValide &&
    distanceFiltree <= DISTANCE_DETECTION;

  bool tresProche =
    mesureValide &&
    distance <= DISTANCE_EXPLOSION;

  switch (etat) {
    case REPOS:
      if (presence) {
        commencerCharge(maintenant);
      }
      break;

    case CHARGE:
      if (presence) {
        dernierePresence = maintenant;
      }

      if (tresProche) {
        commencerExplosion(maintenant);
      }
      else if (
        maintenant - dernierePresence >
        PERTE_SIGNAL
      ) {
        annulerCharge();
      }
      else {
        afficherCharge(
          maintenant - debutCharge
        );
      }
      break;

    case EXPLOSION:
      break;

    case ATTENTE_REARMEMENT:
      if (
        mesureValide &&
        distance > DISTANCE_REARMEMENT
      ) {
        if (debutRearmement == 0) {
          debutRearmement = maintenant;
        }

        if (
          maintenant - debutRearmement >=
          TEMPS_REARMEMENT
        ) {
          etat = REPOS;
          distanceFiltree = -1;
          debutRearmement = 0;
        }
      } else {
        debutRearmement = 0;
      }
      break;
  }
}
