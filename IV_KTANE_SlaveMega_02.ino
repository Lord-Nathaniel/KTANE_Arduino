#include <Wire.h>
#include <pitches.h>
#include <LiquidCrystal.h>

//**********************************************************************************************************************************
//SLAVE #1 : AFFICHEUR DIGIT + POTENTIOMETRE
//Branchements :

 /** 
 * de 2 à 5 : Led Rouge/Jaune/Verte/Bleue du Simon's Says
 * de 6 à 9 : Boutons Rouge/Jaune/Verte/Bleue du Simon's Says
 * sur 10 : Buzzer du Simon's Says 
 * de 14 à 16 : Led RGB du Button
 * sur 17 : Bouton du Button
 * de 18 à 23 : Fils du Wire
 * de 24 à 27 : Boutons du Keypad
 * de 28 à 33 : Afficheur Venting Gas
 * de 34 à 35 : Boutons du Venting Gas
 * sur 36 : Buzzer du Venting Gas
 * sur 37 : Led du Morse Code
 * de 39 à 44 : Fils du Complicated Wires
 * de 45 à 50 : Led du Complicated Wires
 * sur A1 : lien MASTER-SLAVE
 * sur A2 : lien SLAVE-MASTER
 * sur A3 : Potentiomètre du Morse Code 
 * A0 : pin libre pour randSeed
 * une entrée 5V pour le poentiometre
 * une sortie GND pour le potentiometre
 */

//**********************************************************************************************************************************
//Données :

/** --Modules dans l'ordre du réglage-- */
/* Simon's Says 
 * Button
 * Wires
 * Keypad
 * Venting Gas (confirmation)
 * Morse Code
 * Complicated Wires
 * Password
 * Discharge Capacitor (confirmation)
 * Master Module (confirmation)
 */

/** --Données de la configuration de la bombe-- */
byte modNum;
byte numReceived;
byte nbError = 0; //nombre d'erreurs 
byte cnfModule = 0; //nombre de modules terminés

/** --Emplacements des liens Maître-Esclaves--  */
int slaveUno = A3;
int slaveMega = A4;
int masterUno = A5;

/** --Setting-- */
boolean setting; //true = réglage en cours //false = démarrage du jeu
byte setSet[6][4]; //affichage du réglage : module en cours de réglage/position
byte modSet = 0; //module en cours de réglage
byte numSet = 0; //position du chiffre


/** --Données Simon's Says-- */
byte simonColorButton[4] = {2,3,4,5};
byte simonColorLed[4] = {8,9,10,11};
byte simonBuzz = 12;
byte simonLed = A2;
//
byte simonKey[5]; // la séquence des 5 touches, déterminées par random
//
boolean vowel;
byte simonColor[3][4];
byte colorVowel[3][4] = {
    3,2,1,0,
    1,0,3,2,
    2,3,1,0};
byte colorConsonant[3][4] = {
    3,0,2,1,
    0,2,1,3,
    1,0,3,2};    
//
const unsigned int simonNote[4] = {262,330,392,440}; //C4,E4,G4,A5 //Do, Mi, Sol, La
//
//MODULE terminé ou non
boolean cnfSimon = false; //true = MODULE s'arrête et trueLEd allumée + envoi signal terminé //false = MODULE continue de fonctionner + envoi signal ERROR
//
//PHASE de lecture, de jeu ou terminé
byte simonPhase =0; // simonPhase = 0 Lecture Led // simonPhase = 1 jouer les Led //simonPhase = 2 Module Terminé
//
//Chrono de bascule de la phase 2 à la phase 1
int simonChrono =0; //
//
//Nombre de touches actives
byte nbKey =0; //nombre de touche active // 5 -> cnfSimon = true
//
//Nombre de touches appuyée pour la sequence
byte nbTry =0; 

/** --Données Button-- */
boolean buttonToggle;
boolean etatButton;

//MODULE terminé ou non
boolean cnfButton = false; //true = MODULE s'arrête et trueLEd allumée + envoi signal terminé //false = MODULE continue de fonctionner + envoi signal ERROR

/** --Données Wire-- */
//MODULE terminé ou non
boolean cnfWire = false; //true = MODULE s'arrête et trueLEd allumée + envoi signal terminé //false = MODULE continue de fonctionner + envoi signal ERROR

/** --Données Keypad-- */
byte keypadLed;
byte keypadOrder[4];
//MODULE terminé ou non
boolean cnfKeypad = false; //true = MODULE s'arrête et trueLEd allumée + envoi signal terminé //false = MODULE continue de fonctionner + envoi signal ERROR
//
//PHASE d'aquisition ou terminé
byte kpPhase =0; // keypadPhase = 0 attente 1e touche // keypadPhase = 1 attente 2e touche //keypadPhase = 2 attente 3e touche // keypadPhase = 3 attente 4e touche //keypadPhase = 4 module Terminé
//
//Etat des boutons du keypad
boolean kpUpLeft;
boolean kpUpRight;
boolean kpDownLeft;
boolean kpDownRight;

/** --Données Wire-- */
byte wireCut;

/** --Données Venting Gas-- */
LiquidCrystal vgLcd(7, 8, 9, 10, 11, 12); // initialize the library with the numbers of the interface pins
int vgOnLed = 4;
int vgBuzz =5;
int vgNoButton = 2;
int vgYesButton = 3;

byte vgPhase = 0;
byte vgTimerVtg = 47; //TIMER de 45seconds + 2secondes de ping opérations
byte vgTimerDsp; //TIMER affiché avec ping pris en compte
byte vgRandOption; //random déterminant yes ou no
byte vgRandStartChrono; //random déterminant le temps entre le premier module résolu et l'activation du venting gas
byte vgRandStandByChrono; //random déterminant le temps d'attente entre deux activations

boolean vgStart = false;
boolean etatVgNoButton; //Pression du bouton NO du module 
boolean etatVgYesButton; //Pression du bouton YES du module
boolean vgWarning = true; //true = 1 avertissement avant explosion de la bombe //false = pas d'avertissement

byte vgTimeWarning[6]= {20, 5, 4, 3, 2, 1}; //Double buzz pour rappeller le chrono


/** --Données Morse Code-- */

/** --Données Complicated Wires-- */

/** --Données Password-- */

/** --Données Discharge Capacitor-- */


//********************************************************************************************************************************************************************************************************************************************************************
void setup(){

/** --Main Setting-- */  
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent);

/** --Setting : Simon's Says-- */  
randomSeed(analogRead(A0));

for (int i=0; i<5; i++)
    simonKey[i]=random(0,4);  

for (int i=0; i<4; i++) {
  pinMode(simonColorLed[i],OUTPUT);
  pinMode(simonColorButton[i],INPUT);
  digitalWrite(simonColorLed[i],LOW);}

pinMode(simonLed,OUTPUT);
digitalWrite(simonLed,LOW);

/** --Setting : Venting Gas-- */
vgRandStartChrono=random(10,26);
vgRandStandByChrono=random(30,46);
vgRandOption = random(0,2);
  
vgLcd.begin(16, 2); // set up the LCD's number of columns and rows:

}

//********************************************************************************************************************************************************************************************************************************************************************
void loop(){

/** ---ETAT 1 : ORDRE MASTER --> MODULES---  */
if (setting){
  switch(numReceived){
    case 0:
    setSet[modSet][numSet]=0;
    numSet++;
    break;  
    case 1:
    setSet[modSet][numSet]=1;
    numSet++;
    break;  
    case 2:
    setSet[modSet][numSet]=2;
    numSet++;
    break;  
    case 3:
    setSet[modSet][numSet]=3;
    numSet++;
    break;  
    case 4:
    setSet[modSet][numSet]=4;
    numSet++;
    break;  
    case 5:
    setSet[modSet][numSet]=5;
    numSet++;
    break;  
    case 6:
    setSet[modSet][numSet]=6;
    numSet++;
    break; 
    case 100:
    break;
    case 110:
    modSet++;
    break;
    case 120:
    for (int i=0; i<5; i++)
      setSet[modSet][i]=0;
    break;
    case 130:
    for (int i=0; i<5; i++)
      setSet[modSet][i]=0;
    modSet--;  
    break;
    case 140:
    for (int j=0; j<5; j++)
      {for (int i=0; i<5; i++)
      setSet[j][i]=0;}
    break;  
    }

if (modSet == 11){

/** --Setting : Simon's Says-- */   
  if (setSet[0][4] == 1)
    vowel = true;
  else if (setSet[0][4] == 2)
    vowel = false;
  else if (setSet[0][4] == 0){
    if (setSet[0][3] == 1)
      vowel = true;
    else if (setSet[0][3] == 2)
      vowel = false;
  }else if (setSet[0][3] == 0){
    if (setSet[0][2] == 1)
      vowel = true;
    else if (setSet[0][2] == 2)
      vowel = false;    
  }else if (setSet[0][2] == 0){
    if (setSet[0][1] == 1)
      vowel = true;
    else if (setSet[0][1] == 2)
      vowel = false;}   

if (vowel){
simonColor[3][4] = colorVowel[3][4];}
else {
simonColor[3][4] = colorConsonant[3][4];}
          
/** --Setting : Button-- */  
  if (setSet[0][4] == 1)
    buttonToggle = true;
  else if (setSet[0][4] == 2)
    buttonToggle = false;
  else if (setSet[0][4] == 0){
    if (setSet[0][3] == 1)
      buttonToggle = true;
    else if (setSet[0][3] == 2)
      buttonToggle = false;
  }else if (setSet[0][3] == 0){
    if (setSet[0][2] == 1)
      buttonToggle = true;
    else if (setSet[0][2] == 2)
      buttonToggle = false;    
  }else if (setSet[0][2] == 0){
    if (setSet[0][1] == 1)
      buttonToggle = true;
    else if (setSet[0][1] == 2)
      buttonToggle = false;}  

/** --Setting : Keypad-- */ 
keypadOrder[0] = setSet[2][0];
keypadOrder[1] = setSet[2][1];
keypadOrder[2] = setSet[2][2];
keypadOrder[3] = setSet[2][3];

/** --Setting : Wires-- */  
wireCut = setSet[3][0];

setting = false;
  } 
}

//********************************************************************************************************************************************************************************************************************************************************************
  
if (!setting){

/** --SIMON'S SAYS-- */
  if (!cnfSimon) {
switch (simonPhase) {
  case 0: //Simon's Says affiche l'ordre des boutons à presser
     simonRead(nbKey);
  break;
  case 1: //Simon's Says enregistre l'ordre des boutons pressé par l'utilisateur
     simonWrite(nbKey);
  break;
  case 2: //Module complété
     cnfModule ++;
     digitalWrite(simonLed,HIGH);
     cnfSimon = true;
  break;
  }
}

/** --BUTTON-- */
if (!cnfButton){
//switch (buttonPhase) {
  
//  }
}

/** --KEYPAD-- */
if (!cnfKeypad){
switch (kpPhase) {
  case 0: //début du module - attente de la 1e touche
    if (keypadButton()==keypadOrder[0]){
      kpPhase ++;
    } else if (keypadButton()!=keypadOrder[0]) {
      nbError ++;
    }
  break;
  case 1: //1e touche correcte - attente de la 2e touche
    if (keypadButton()==keypadOrder[1]){
      kpPhase ++;
    } else if(keypadButton()!=keypadOrder[1]) {
      nbError ++;
      kpPhase =0;
    }
    break;
  case 2: //2e touche correcte - attente de la 3e touche
    if (keypadButton()==keypadOrder[2]){
      kpPhase ++;
    } else if (keypadButton()!=keypadOrder[2]) {
      nbError ++;
      kpPhase =0;
    }
    break;
  case 3: //3e touche correcte - attente de la 4e touche
    if (keypadButton()==keypadOrder[3]){
      kpPhase ++;
    } else if (keypadButton()!=keypadOrder[3]) {
      nbError ++;
      kpPhase =0;
    }
    break;
  case 4: //4e touche correcte = module complété
    cnfModule ++;
    digitalWrite(keypadLed,HIGH);
    cnfKeypad = true;
  break;
  }
}

/** --VENTING GAS-- */
etatVgNoButton = digitalRead(vgNoButton);
etatVgYesButton = digitalRead(vgYesButton);
vgTimerDsp = vgTimerVtg - 2; //ping de 2 secondes, non affiché

switch (vgPhase) {
  case 0: //Aucun module résolu et en attente, ou démarre après 2min d'attente
    vgLcd.setCursor(1, 0);
    vgLcd.print("                ");
    vgLcd.setCursor(0, 1);
    vgLcd.print("                ");

    if (vgStart){
      if (vgRandStartChrono == 0)
      vgPhase = 1;
    } else if (vgRandStartChrono != 0){
      delay (1000);
      vgRandStartChrono --;}
  break;
  
  case 1: //Allumage : un module a été résolu, ou le temps de StandBy est écoulé
    vgOption();

    for (int i = 0; i<6; i++) { //Double buzz pour rappeller le chrono
      if (vgTimerVtg == vgTimeWarning[i]) {
        tone (vgBuzz, NOTE_C4, 100);
        delay(150);
        tone (vgBuzz, NOTE_C4, 100);}
    }
    
    if (vgTimerVtg == 0){
      vgLcd.setCursor(1, 0);
      vgLcd.print("                ");
      vgLcd.setCursor(0, 1);
      vgLcd.print("                ");
  
      vgLcd.setCursor(1, 0);
      vgLcd.print(" ECHEC ECHEC ");
      vgLcd.setCursor(0, 1);
      vgLcd.print("  ECHEC ECHEC ");
       //fin de la bombe !
      }  

  delay (1000);
  vgTimerVtg --;  
  break;
  
  case 2: //StandBy : bouton correct pressé
    if (vgRandStandByChrono == 0) {
      vgRandStandByChrono = random (35,46); 
      tone (vgBuzz, NOTE_C4, 100);
      delay(150);
      tone (vgBuzz, NOTE_C4, 100); 
      vgRandOption = random(0,2);
      vgPhase = 1;
  } else if (vgRandStandByChrono != 0) {
      vgRandStandByChrono --;
      delay (1000);}
  break;}

}
}
//********************************************************************************************************************************************************************************************************************************************************************
void receiveEvent(byte bytesReceived) //Evenement : bytes reçus par le maître
{
  numReceived = Wire.read();    // receive byte as an integer
}

//**********************************************************************************************************************************
void requestEvent() //Evenement : bytes requis par le maître
{
  Wire.write(nbError);
}

//********************************************************************************************************************************************************************************************************************************************************************
/** --SIMON'S SAYS Function-- */
void simonRead (byte x) { //séquence de leture des Led
  for (int i = 0; i <= x; i++) {
    delay (200);
    padUpDown(simonKey[i]);}
  simonPhase = 1;   
}

//********************************************************************************************************************************************************************************************************************************************************************
/** --SIMON'S SAYS Function-- */
void simonWrite (int x){ //séquence d'appui 

if (simonButton() == 4){
  delay (20);
  simonChrono ++;}
  
if(simonChrono == 500){
  simonChrono = 0;
  simonPhase = 0;} 

if (simonButton() < 4){ 
  validation(simonButton());}

}

//********************************************************************************************************************************************************************************************************************************************************************
/** --SIMON'S SAYS Function-- */
void validation (byte x) {    //vérification si chaque touche est egal a son nbKey et donne goodLed ou wrongLed
  if (x == simonColor[nbError][simonKey[nbTry]]){
     padUpDown(simonColor[nbError][simonKey[nbTry]]);
     nbTry++;
     simonChrono =0;

    if (nbTry > nbKey){
       nbTry = 0;
       nbKey++;
       simonPhase = 0;
       delay (2000);}
  

    if (nbKey == 5){
       simonPhase = 2;}

  } else {
     nbError ++;}
}

//*******************************************************
/** --SIMON'S SAYS Function-- */
void padUpDown (byte x) {     //sequence d'allumage et extinction des led
    digitalWrite(simonColorLed[x],HIGH);
    tone(simonBuzz,simonNote[x]);
    delay(300);
    digitalWrite(simonColorLed[x],LOW);
    noTone(simonBuzz);
}

//*******************************************************
/** --SIMON'S SAYS Function-- */
byte simonButton (){       //vérification de l'appui d'un bouton
  if (digitalRead(simonButton[0])){
  return 0;
  delay (200);}
  else if (digitalRead(simonButton[1])){
  return 1;
  delay (200);}  
  else if (digitalRead(simonButton[2])){
  return 2;
  delay (200);}  
  else if (digitalRead(simonButton[3])){
  return 3;
  delay (200);}  
  else
  return 4;
}

//********************************************************************************************************************************************************************************************************************************************************************
/** --KEYPAD Function-- */
byte keypadButton(){
  if (digitalRead(kpUpLeft)){
  return 0;
  delay (200);}
  else if (digitalRead(kpUpRight)){
  return 1;
  delay (200);}  
  else if (digitalRead(kpDownLeft)){
  return 2;
  delay (200);}  
  else if (digitalRead(kpDownRight)){
  return 3;
  delay (200);}  
  else
  return 4;
}

//********************************************************************************************************************************************************************************************************************************************************************
/** --VENTING GAS Function-- */
void vgOption (){
switch (vgRandOption) {
  case 0:
  vgLcd.setCursor(1, 0);
  vgLcd.print("Ventiler Gaz?");
  vgLcd.setCursor(0, 1);
  vgLcd.print("Oui/Non ");
  vgLcd.print(vgTimerDsp);

  if (etatVgNoButton) {
    if (vgWarning){
      vgLcd.setCursor(1, 0);
      vgLcd.print("                ");  
      vgLcd.setCursor(1, 0);
      vgLcd.print("Gaz va exploser!");
      vgWarning = false;
    } else if (!vgWarning){
      vgLcd.setCursor(1, 0);
      vgLcd.print("                ");
      vgLcd.setCursor(0, 1);
      vgLcd.print("                ");
  
      vgLcd.setCursor(1, 0);
      vgLcd.print(" ECHEC ECHEC ");
      vgLcd.setCursor(0, 1);
      vgLcd.print("  ECHEC ECHEC ");
        //fin de la bombe !
    }     
  }
  
  if (etatVgYesButton) {
    vgLcd.setCursor(1, 0);
    vgLcd.print("                ");  
    vgLcd.setCursor(1, 0);
    vgLcd.print("Gaz ventile!");
    vgLcd.setCursor(0, 1);
    vgLcd.print("                ");
    vgWarning = true;
    vgPhase = 2;}      
  break;

  case 1:
  vgLcd.setCursor(1, 0);
  vgLcd.print("Exploser bombe?");
  vgLcd.setCursor(0, 1);
  vgLcd.print("Oui/Non ");
  vgLcd.print(vgTimerDsp);

  if (etatVgYesButton) {
    if (vgWarning){
      vgLcd.setCursor(1, 0);
      vgLcd.print("                ");  
      vgLcd.setCursor(1, 0);
      vgLcd.print("Etes-vous sur ?");
      vgWarning = false;
    } else if (!vgWarning){
      vgLcd.setCursor(1, 0);
      vgLcd.print("                ");
      vgLcd.setCursor(0, 1);
      vgLcd.print("                ");
  
      vgLcd.setCursor(1, 0);
      vgLcd.print(" ECHEC ECHEC ");
      vgLcd.setCursor(0, 1);
      vgLcd.print("  ECHEC ECHEC ");
       //fin de la bombe !
    } 
  }  

  if (etatVgNoButton) {
    vgLcd.setCursor(1, 0);
    vgLcd.print("                ");  
    vgLcd.setCursor(1, 0);
    vgLcd.print("Desamorcee!");
    vgLcd.setCursor(0, 1);
    vgLcd.print("                ");
    vgWarning = true;      
    vgPhase = 2;}
  break;
}
}  
