#include <LiquidCrystal_I2C.h>

////////////////////Prototypes de fonctions////////////////////////////
void message_choix();
void menu_boisson();
void choixMenu();
void valideSelection();
void detecte_presence();  
void resetEtat();
void distribution_boisson();
void jouerBuzzer(int duree);
void afficherBarreProgression(unsigned long tempsEcoule);
bool detectepiece();

///////////////////////////////////////////////////////////////////////

// Configuration des broches et variables
#define LED_PIN 10
#define PINPRESENCE 8
#define BTN_SELECT A0
#define BTN_NAVIGATE A1
#define BUZZER 6 
#define PINPIECE 7

LiquidCrystal_I2C lcd(0x20, 16, 2);
const int pinPompeEau = 9;
const int pinPompeJus = 11;
const int dureeDistribution = 20000;
const int nombreSegments = 16;

int selectBtnVal = 0;
int navigateBtnVal = 0;
int currentSelection = 0;
const int nombreBoissons = 2;
const char* boissons[] = {"Jus", "Eau"};
bool pompeEtat = false;

// Machine à états
enum EtatSysteme {
  ETAT_ATTENTE_PIECE,
  ETAT_CHOIX_BOISSON,
  ETAT_DISTRIBUTION,
  ETAT_FIN
};

EtatSysteme etatActuel = ETAT_ATTENTE_PIECE; //Attendre insertion pièce

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
  pinMode(PINPIECE, INPUT);

  lcd.init();
  lcd.backlight();
  Serial.begin(19200);
}

void loop() {
  detecte_presence();  // Toujours détecter présence gobelet 

  switch (etatActuel) {
    case ETAT_ATTENTE_PIECE:
      lcd.setCursor(0, 0);
      lcd.print("Inserez piece...  ");
      lcd.setCursor(0, 1);
      lcd.print("                  ");
      if (detectepiece()) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Piece detectee !");
        delay(1000);
        message_choix();
        menu_boisson();
        etatActuel = ETAT_CHOIX_BOISSON;
      }
      delay(300);
      break;

    case ETAT_CHOIX_BOISSON:
      selectBtnVal = digitalRead(BTN_SELECT);
      navigateBtnVal = digitalRead(BTN_NAVIGATE);

      if (navigateBtnVal == LOW) {
        choixMenu();
        delay(300);
      } else if (selectBtnVal == LOW) {
        valideSelection();  // Affiche le choix
        etatActuel = ETAT_DISTRIBUTION;
        delay(300);
      }
      break;

    case ETAT_DISTRIBUTION:
      distribution_boisson();
      etatActuel = ETAT_FIN;
      break;

    case ETAT_FIN:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Merci !"); //
      delay(2000);
      resetEtat();
      etatActuel = ETAT_ATTENTE_PIECE;
      break;
  }
}

bool detectepiece() {
  return digitalRead(PINPIECE) == HIGH;
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
    if (i == currentSelection) {
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
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Boisson: ");
  lcd.setCursor(0, 1);
  lcd.print(boissons[currentSelection]);
  delay(2000); 
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

  jouerBuzzer(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Boisson servie!");
  delay(2000);
}

void resetEtat() {
  currentSelection = 0;
  lcd.clear();
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



