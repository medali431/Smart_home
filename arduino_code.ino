#include <SPI.h>              // Bibliothèque pour la communication SPI
#include <MFRC522.h>          // Bibliothèque pour le module RFID RC522
#include <Servo.h>            // Bibliothèque pour contrôler les servomoteurs
#include <DHT.h>              // Bibliothèque pour le capteur de température DHT

// Configuration du module RFID RC522
#define RST_PIN 9           // Broche RST pour le module RFID
#define SS_PIN 53           // Broche SS (SDA) pour le module RFID
#define RFID_SERVO_PIN 8    // Broche du servomoteur contrôlé par RFID
#define MANUAL_SERVO_PIN 10 // Broche du servomoteur manuel

// Configuration des capteurs et périphériques
#define PIR_PIN 7           // Broche du capteur PIR (détection de mouvement)
#define GAS_SENSOR_PIN A0   // Broche analogique pour le capteur de gaz
#define BUZZER_PIN 12       // Broche du buzzer
#define DHT_PIN 6           // Broche du capteur DHT pour la température
#define DHT_TYPE DHT11      // Type de capteur DHT utilisé (DHT11)
#define WATER_SENSOR_PIN A1 // Broche analogique pour le capteur d'eau
#define WINDOW_SERVO1_PIN 4 // Broche du servomoteur pour la fenêtre 1
#define WINDOW_SERVO2_PIN 5 // Broche du servomoteur pour la fenêtre 2
#define FAN_SERVO_PIN 3     // Broche du servomoteur pour le ventilateur

// Initialisation des composants
MFRC522 rfid(SS_PIN, RST_PIN);    // Création d'une instance pour le module RFID
Servo rfidServo, manualServo;    // Création des instances pour les servos
Servo fanServo, windowServo1, windowServo2;
DHT dht(DHT_PIN, DHT_TYPE);       // Initialisation du capteur DHT
int leds[] = {A2, A3, A4};        // Broches pour les LEDs contrôlées par le PIR

// UID de la carte maître (remplacez ces valeurs par celles de votre carte)
byte masterCard[4] = {0xF3, 0x7E, 0x59, 0x1A};

// Variables pour le contrôle simultané
bool rfidServoActive = false;        // État du servomoteur RFID
bool manualServoActive = false;      // État du servomoteur manuel
unsigned long rfidServoStartTime = 0;// Temps d'activation du servo RFID
unsigned long manualServoStartTime = 0; // Temps d'activation du servo manuel
const unsigned long servoOpenDuration = 5000; // Durée d'ouverture des servos (en ms)

// Variables pour les capteurs
bool pirDetected = false;  // Détection par le capteur PIR
bool gasDetected = false;  // Détection par le capteur de gaz
bool waterDetected = false;// Détection par le capteur d'eau

void setup() {
  // Initialisation des servomoteurs
  rfidServo.attach(RFID_SERVO_PIN);        // Attache le servomoteur RFID à sa broche
  manualServo.attach(MANUAL_SERVO_PIN);    // Attache le servomoteur manuel
  fanServo.attach(FAN_SERVO_PIN);          // Attache le servomoteur ventilateur
  windowServo1.attach(WINDOW_SERVO1_PIN);  // Attache le servomoteur pour fenêtre 1
  windowServo2.attach(WINDOW_SERVO2_PIN);  // Attache le servomoteur pour fenêtre 2
  
  // Définir les positions initiales des servomoteurs
  rfidServo.write(0);
  manualServo.write(0);
  fanServo.write(0);
  windowServo1.write(0);
  windowServo2.write(0);

  // Initialisation des capteurs
  pinMode(PIR_PIN, INPUT);           // Configure la broche PIR comme entrée
  pinMode(GAS_SENSOR_PIN, INPUT);    // Configure la broche du capteur de gaz comme entrée
  pinMode(BUZZER_PIN, OUTPUT);       // Configure la broche du buzzer comme sortie
  pinMode(WATER_SENSOR_PIN, INPUT);  // Configure la broche du capteur d'eau comme entrée
  for (int i = 0; i < 3; i++) pinMode(leds[i], OUTPUT); // Configure les LEDs comme sorties

  // Initialisation du module RFID
  SPI.begin();                       // Démarre la communication SPI
  rfid.PCD_Init();                   // Initialise le module RFID

  Serial.begin(9600);                // Démarre la communication série
}

void loop() {
  handleRFID();        // Gère la lecture RFID
  handleManualServo(); // Gère le contrôle manuel des servos
  handlePIR();         // Gère la détection PIR
  handleGasSensor();   // Gère la détection de gaz
  handleDHTSensor();   // Gère la détection de température
  handleWaterSensor(); // Gère la détection d'eau
  updateServos();      // Met à jour les états des servomoteurs
}

// Gère la détection RFID
void handleRFID() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    if (checkUID(rfid.uid.uidByte)) {
      Serial.println("Carte reconnue. Activation du servomoteur RFID.");
      rfidServo.write(90);            // Ouvre le servomoteur RFID
      rfidServoActive = true;
      rfidServoStartTime = millis();
    } else {
      Serial.println("Carte non reconnue.");
    }
    rfid.PICC_HaltA();                // Arrête la lecture RFID
  }
}

// Gère les commandes série pour le servomoteur manuel
void handleManualServo() {
  if (Serial.available()) {
    String command = Serial.readString(); // Lire la commande depuis le port série
    command.trim();

    if (command == "OPEN") {
      Serial.println("Commande OPEN reçue. Ouverture du servomoteur manuel.");
      manualServo.write(90);         // Ouvre le servomoteur manuel
      manualServoActive = true;
      manualServoStartTime = millis();
    } else {
      Serial.println("Commande non reconnue.");
    }
  }
}

// Gère la détection PIR
void handlePIR() {
  pirDetected = digitalRead(PIR_PIN); // Lecture de l'état du capteur PIR
  if (pirDetected) {
    Serial.println("Mouvement détecté !");
    for (int i = 0; i < 3; i++) digitalWrite(leds[i], HIGH); // Allume les LEDs
  } else {
    for (int i = 0; i < 3; i++) digitalWrite(leds[i], LOW);  // Éteint les LEDs
  }
}

// Gère le capteur de gaz
void handleGasSensor() {
  gasDetected = analogRead(GAS_SENSOR_PIN) > 300; // Lecture avec seuil ajustable
  if (gasDetected) {
    Serial.println("Gaz détecté !");
    digitalWrite(BUZZER_PIN, HIGH);// Active le buzzer
  } else {
    digitalWrite(BUZZER_PIN, LOW); // Désactive le buzzer
  }
}

// Gère le capteur de température
void handleDHTSensor() {
  float temp = dht.readTemperature(); // Lecture de la température
  if (!isnan(temp)) {
    if (temp > 30) {                 // Si la température dépasse 30°C
      Serial.println("Température élevée !");
      fanServo.write(90);           // Active le ventilateur
    } else {
      fanServo.write(0);            // Désactive le ventilateur
    }
  }
}

// Gère le capteur d’eau
void handleWaterSensor() {
  waterDetected = analogRead(WATER_SENSOR_PIN) > 500; // Lecture avec seuil ajustable
  if (waterDetected) {
    Serial.println("Eau détectée !");
    windowServo1.write(90);         // Ferme la fenêtre 1
    windowServo2.write(90);         // Ferme la fenêtre 2
  } else {
    windowServo1.write(0);          // Ouvre la fenêtre 1
    windowServo2.write(0);          // Ouvre la fenêtre 2
  }
}

// Met à jour les servomoteurs
void updateServos() {
  unsigned long currentTime = millis();

  if (rfidServoActive && (currentTime - rfidServoStartTime >= servoOpenDuration)) {
    rfidServo.write(0);            // Ferme automatiquement le servomoteur RFID
    rfidServoActive = false;
    Serial.println("Servomoteur RFID fermé automatiquement.");
  }

  if (manualServoActive && (currentTime - manualServoStartTime >= servoOpenDuration)) {
    manualServo.write(0);          // Ferme automatiquement le servomoteur manuel
    manualServoActive = false;
    Serial.println("Servomoteur manuel fermé automatiquement.");
  }
}

// Vérifie l'UID de la carte
bool checkUID(byte* uid) {
  for (byte i = 0; i < 4; i++) {
    if (uid[i] != masterCard[i]) {
      return false;                // Retourne faux si l'UID ne correspond pas
    }
  }
  return true;                     // Retourne vrai si l'UID correspond
}
