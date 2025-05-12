#include <LiquidCrystal_I2C.h>

////////////////////Prototypes de fonctions////////////////////////////

void setup();
void loop();
void message_choix();
void menu_boisson();
void choixMenu();
void valideSelection();
void detecte_presence();  
void resetEtat();
void distribution_boisson();
void detection_niveau();
void jouerBuzzer(int duree);
void afficherBarreProgression(unsigned long tempsEcoule);
void detectepiece();
///////////////////////////////////////////////////////////////////////

// Configuration des broches et variables
#define LED_PIN 10
#define PINPRESENCE 8
#define BTN_SELECT A0  // Bouton pour la sélection
#define BTN_NAVIGATE A1  // Bouton pour la navigation
#define BUZZER 6 
const int pinPompeEau = 9;  // Pin pour la pompe à eau
const int pinPompeJus = 11; // Pin pour la pompe à jus
LiquidCrystal_I2C lcd(0x20, 16, 2); // Configure l’adresse I2C et la taille de l’écran LCD (16x2)
const int dureeDistribution=20000;
const int nombreSegments = 16; // Nombre de segments pour la barre de progression (16 caractères par ligne)

// Variables de gestion des choix
int selectBtnVal = 0;
int navigateBtnVal = 0;
int currentSelection = 0;
bool boissonSelectionnee = false;
const int nombreBoissons = 2; // Nombre de choix 
const char* boissons[] = {"Jus", "Eau"}; // Liste des boissons
bool pompeEtat = false; 

void setup() {
  pinMode(PINPRESENCE, INPUT); 
  pinMode(BTN_SELECT, INPUT_PULLUP);
  pinMode(BTN_NAVIGATE, INPUT_PULLUP);
  pinMode(pinPompeEau, OUTPUT);        
  digitalWrite(pinPompeEau, LOW);      
  pinMode(pinPompeJus, OUTPUT);        
  digitalWrite(pinPompeJus, LOW);      
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);          
  pinMode(BUZZER, OUTPUT);

  lcd.init();
  lcd.backlight();

  // Afficher directement la sélection des boissons
  message_choix();
  menu_boisson();
  
  Serial.begin(19200);
}

void loop() {
  selectBtnVal = digitalRead(BTN_SELECT);
  navigateBtnVal = digitalRead(BTN_NAVIGATE);
  detecte_presence();

  if (!boissonSelectionnee && (selectBtnVal == LOW || navigateBtnVal == LOW)) {
    while (!boissonSelectionnee) {
      detecte_presence(); 
      if (digitalRead(BTN_NAVIGATE) == LOW) {
        choixMenu();
        delay(300); 
      }
      if (digitalRead(BTN_SELECT) == LOW) {
        valideSelection();
        delay(300); 
      }
    }
  }
}

void message_choix() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Choisissez votre");
  lcd.setCursor(0, 1);
  lcd.print("boisson");
  delay(2000); 
}

void menu_boisson() {
  lcd.clear();
  for (int i = 0; i < nombreBoissons; i++) {
    lcd.setCursor(0, i);
    if (i == currentSelection) { //affichage > 
      lcd.print("> ");
    } else {
      lcd.print("  ");
    }
    lcd.print(boissons[i]);
  }
}

void choixMenu() {
  currentSelection = (currentSelection + 1) % nombreBoissons; 
  menu_boisson();
}

void valideSelection() {
  boissonSelectionnee = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Boisson: ");
  lcd.setCursor(0, 1);
  lcd.print(boissons[currentSelection]);
  delay(2000); 
  distribution_boisson(); 
}

void detecte_presence() {
  if (digitalRead(PINPRESENCE) == LOW) { 
    digitalWrite(LED_PIN, LOW);  
  } else {
    digitalWrite(LED_PIN, HIGH);
  }
}

void distribution_boisson() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Distribution...");

  jouerBuzzer(500);
  
  unsigned long debutDistribution = millis();

  if (currentSelection == 0) { 
    digitalWrite(pinPompeJus, HIGH);
  } else { 
    digitalWrite(pinPompeEau, HIGH);
  }
  
  pompeEtat = true;
  while (pompeEtat) {
    unsigned long tempsEcoule = millis() - debutDistribution;
    if (tempsEcoule >= dureeDistribution) { 
      break;
    }

    afficherBarreProgression(tempsEcoule);

    if (digitalRead(PINPRESENCE) == LOW) { 
      digitalWrite(LED_PIN, LOW);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Gobelet retire!");
      delay(2000);
      break;
    }
    delay(100);
  }
  
  digitalWrite(pinPompeEau, LOW);
  digitalWrite(pinPompeJus, LOW);
  pompeEtat = false;

  jouerBuzzer(1000); // signaler la fin de la distribution
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Boisson servie!");
  delay(2000);
  resetEtat(); 
}

void resetEtat() {
  boissonSelectionnee = false;
  currentSelection = 0;        
  message_choix();
  menu_boisson();
}

void jouerBuzzer(int duree) {
  tone(BUZZER, 1000);
  delay(duree);
  noTone(BUZZER);
}

void afficherBarreProgression(unsigned long tempsEcoule) {
  int progression = (tempsEcoule * nombreSegments) / dureeDistribution;
  lcd.setCursor(0, 1);
  for (int i = 0; i < nombreSegments; i++) {
    if (i < progression) {
      lcd.print("#");
    } else {
      lcd.print(" ");
    }
  }
}

