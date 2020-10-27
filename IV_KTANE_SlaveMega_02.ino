#include <Wire.h>
#include <pitches.h>
#include <LiquidCrystal.h>

//**********************************************************************************************************************************
//SLAVE #1 : AFFICHEUR DIGIT + POTENTIOMETRE
//Branchements :

 /** de 2 à 8 : pin A,B,C,D,E,G,F
 * de 9 à 12 : pin COM1,COM2,COM3,COM4
 * sur 13 : pin dp
 * sur A1 : potentiometre
 * sur A4 : lien MASTER-SLAVE
 * sur A5 : lien SLAVE-MASTER
 * A0 : pin libre pour randSeed
 * une entrée 5V pour le poentiometre
 * une sortie GND pour le potentiometre
 */

//**********************************************************************************************************************************
//Données :

/** --Branchements-- */
//General Setting
//Simon's Says
//Keypad
int kpUpLeft;
int kpUpRight;
int kpDownLeft;
int kpDownRight;
//Button
//Wires
//Venting Gas
LiquidCrystal vgLcd(7, 8, 9, 10, 11, 12); // initialize the library with the numbers of the interface pins
int vgOnLed = 4;
int vgBuzz =5;
int vgNoButton = 2;
int vgYesButton = 3;


/** --Données de la configuration de la bombe-- */
byte modNum;
byte numReceived;
int ledSimonSays = 2;
int ledMaster = 3;
int ledComplicatedWires = 4;
int ledPassword = 5;
int ledWires = 6;
int ledVentingGas = 7;
byte nbError = 0; //nombre d'erreurs 
byte cnfModule = 0; //nombre de modules terminés

/** --Emplacements des liens Maître-Esclaves--  */
int slaveUno = A3;
int slaveMega = A4;
int masterUno = A5;

/** --Setting-- */
boolean setting; //true = réglage en cours //false = démarrage du jeu
byte setSet[6][4];
byte modSet = 0;
byte numSet = 0;


/** --Données Simon's Says-- */
int simonColorButton[4] = {2,3,4,5};
int simonColorLed[4] = {8,9,10,11};
int simonBuzz = 12;
int simonLed = A2;

byte simonKey[5]; // la séquence des 5 touches, déterminées par random

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
 
const unsigned int simonNote[4] = {262,330,392,440}; //C4,E4,G4,A5 //Do, Mi, Sol, La

//MODULE terminé ou non
boolean cnfSimon = false; //true = MODULE s'arrête et trueLEd allumée + envoi signal terminé //false = MODULE continue de fonctionner + envoi signal ERROR

//PHASE de setup, lecture ou jeu
byte simonPhase =0; //simonPhase = 0 SETUP // simonPhase = 1 Lecture Led // simonPhase = 2 jouer les Led

//Chrono de bascule de la phase 2 à la phase 1
int simonChrono =0; //

//Nombre de touches actives
byte nbKey =0; //nombre de touche active // 5 -> cnfSimon = true

//Nombre de touches appuyée pour la sequence
byte nbTry =0; 


/** --Données Venting Gas-- */
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

if (modSet == 7){

/** --Setting : Simon's Says-- */   
  if (setSet[1][4] == 1)
    vowel = true;
  else if (setSet[1][4] == 2)
    vowel = false;
  else if (setSet[1][4] == 0){
    if (setSet[1][3] == 1)
      vowel = true;
    else if (setSet[1][3] == 2)
      vowel = false;
  }else if (setSet[1][3] == 0){
    if (setSet[1][2] == 1)
      vowel = true;
    else if (setSet[1][2] == 2)
      vowel = false;    
  }else if (setSet[1][2] == 0){
    if (setSet[1][1] == 1)
      vowel = true;
    else if (setSet[1][1] == 2)
      vowel = false;}   

if (vowel){
simonColor[3][4] = colorVowel[3][4];}
else {
simonColor[3][4] = colorConsonant[3][4];}
          
setting = false;
  }
}
  
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

/** --KEYPAD-- */
if (!cnfKeypad){
switch (kpPhase) {
  case 0:
  break;
  case 1:
  break;
  case 2:
  break;
  case 3:
  break;
  case 4:
  break;
  case 5:
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
void receiveEvent(byte bytesReceived)
{
  numReceived = Wire.read();    // receive byte as an integer
}

//**********************************************************************************************************************************
void requestEvent()
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
