#include <Wire.h>
#include "SevSeg.h"
SevSeg sevseg;
#include "IRremote.h"
   
//**********************************************************************************************************************************
//MASTER
//Branchements :
 
 /** 
  *  sur A0 : pin MasterLed
  *  sur A1 : pin ErrorLedOne
  *  sur A2 : pin ErrorLedTwo
  *  sur A3 : pin Slave Uno
  *  sur A4 : pin Slave Mega
  *  sur A5 : pin Master Uno
  */
   
//**********************************************************************************************************************************
//Données :

/** ---Données générales--- */
int state = 0; //0 = Démarrage //1 = Réglages des paramètres au moyen de la télécommande //2 = Jeu en cours //3 = Etat de Défaite //4 = Etat de Victoire
int masterLed = A0;
int errorLedOne = A1;
int errorLedTwo = A2;
byte modNum; //Numéro du module en cours de réglage
byte numReceived = 100;

/** !!--Ne pas modifier : données sevseg--!!  */
float displayTimeSecs = 1; //how long do you want each number on display to show (in secs)
float displayTime = (displayTimeSecs * 5000);
long startNumber = 240; //countdown starts with this number
long endNumber = 0; //countdown ends with this number

/** --Pour la fonction irReceiver-- */
int receiver = 11; //Signal Pin of IR receiver to Arduino Digital 
IRrecv irrecv(receiver);     // create instance of 'irrecv'
decode_results results;      // create instance of 'decode_results'
boolean firstreceiver = true;

/** --Liste des mots pour l'afficheur principal-- */
char ends = "End ";
char opens = "OPEn";

/** --Emplacements des liens Maître-Esclaves--  */
int slaveUno = A3;
int slaveMega = A4;
int masterUno = A5;
   
//**********************************************************************************************************************************
void setup() {
  
Wire.begin(); //Rejoint le bus I2C (addresse slave)

  
/** --emplacement afficheur-- */
byte numDigits = 4;
byte digitPins[] = {9, 10, 11, 12};
byte segmentPins[] = {2, 3, 4, 5, 6, 8, 7, 13}; //A,B,C,D,E,G,F,dp


/** --Ne pas modifier : données sevseg--  */
bool resistorsOnSegments = true;
byte hardwareConfig = COMMON_CATHODE;
bool updateWithDelays = false;
bool leadingZeros = true;
bool disableDecPoint = false;

/** --Ne pas modifier : données sevseg--  */
sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments,
updateWithDelays, leadingZeros, disableDecPoint);
sevseg.setBrightness(90);

/**  --Ne pas modifier : donnée IRReceiver--  */
irrecv.enableIRIn(); // Start the receiver

state = 1;

}
//**********************************************************************************************************************************
void loop() {
  
long a = (startNumber-1)/600;
long b = startNumber/60;
long c = (startNumber%60)/10;
long d = startNumber%10;
long n = a*1000+b*100+c*10+d;

/** ---ETAT 1 : TRANSFERT RECEVEUR IR --> MODULES---  */
if (state == 1){
  modNum = 10;
  int j = 1;
  
/** --Démarrage module 1-- */
if (firstreceiver == true){ //envoi démarrage module 1
  Wire.beginTransmission(slaveMega); // transmit to device 
  Wire.write(modNum);
  Wire.endTransmission();   // stop transmitting
  firstreceiver = false;}

/** --NE PAS MODIFIER-- */
/** --Fonction pour récupérer le bouton appuyé-- */  
if (irrecv.decode(&results)) // have we received an IR signal?
  {
    numReceived = irReceiver(); 
    irrecv.resume(); // receive the next value
  } 

/** --Bouton 0/1/2/3/4/5/6 pressé-- */
if (numReceived == 0 || 1 || 2 || 3 || 4 || 5 || 6){
  Wire.beginTransmission(slaveMega); // transmit to device 
  Wire.write(numReceived);
  Wire.endTransmission();   // stop transmitting
  numReceived = 100;}

/** --Bouton de validation pressé-- */
if (numReceived == 110){ 
  Wire.beginTransmission(slaveMega); // transmit to device 
  modNum += 10;  
  Wire.write(modNum);
  Wire.endTransmission();
  numReceived = 100;}   // stop transmitting

/** --Bouton de remise à zéro pressé-- */  
if (numReceived == 120){ 
  Wire.beginTransmission(slaveMega); // transmit to device 
  Wire.write(numReceived);
  Wire.endTransmission();
  numReceived = 100;}

/** --Bouton de retour arrière pressé-- */
if (numReceived == 130){ //cas de retour arrière
  Wire.beginTransmission(slaveMega); // transmit to device 
  Wire.write(numReceived);
  Wire.endTransmission();
  numReceived = 100;
    modNum -= 10;}

/** --Bouton d'annulation complète pressé-- */
if (numReceived == 140){ //cas d'annulation complète
  Wire.beginTransmission(slaveMega); // transmit to device 
  Wire.write(numReceived);
  Wire.endTransmission();
  numReceived = 100;
    modNum = 10;}

if (modNum == 70)
  state = 2;    
}

/** ---ETAT 2 : JEU---  */
if (state == 2) {

//ETAT 2,1 : TIMER
boolean etatTimer = true;
if (etatTimer) {
    if (n >= endNumber) {
        for (long i = 0; i <= displayTime; i++){
        sevseg.setNumber(n,2);
        sevseg.refreshDisplay();
        } 
    startNumber--;
    }
}    

//ETAT 2,2 : AFFICHEUR  
if (etatTimer) { 
    sevseg.setNumber(n,2);
    }
sevseg.refreshDisplay();


//ETAT 2,3 : RECEVEUR : VALIDATION

//ETAT 2,4 : RECEVEUR : ERREUR

}

/** ---ETAT 3 : DEFAITE---  */
if (state == 3){
sevseg.setChars(ends); //after countdown shows endNumber, show this.
}

/** ---ETAT 4 : VICTOIRE---  */
if (state == 4){
sevseg.setChars(opens);
}

}
//**********************************************************************************************************************************
int irReceiver(){ // takes action based on IR code received

// describing Remote IR codes 
switch(results.value)  {
  case 0xFFA25D: //POWER 
  return 140 ; break; /** cas d'annulation complète */
  case 0xFFE21D: //VOL+
  break;
  case 0xFF629D: //FUNC/STOP
  return 120 ; break; /** cas de remise à zero */
  case 0xFF22DD: //FAST BACK 
  return 130 ; break; /** cas de retour arrière */
  case 0xFF02FD: //PLAY/PAUSE 
  return 110 ; break; /** cas de validation */
  case 0xFFC23D: //FAST FOWARD 
  return 110 ; break; /** cas de validation */
  case 0xFFE01F: //DOWN
  break;
  case 0xFFA857: //VOL-
  break;
  case 0xFF906F: //UP
  break;
  case 0xFF9867: //EQ
  break;
  case 0xFFB04F: //ST/REPT 
  return 120 ; break; /** cas de remise à zero */
  case 0xFF6897: //0
  return 0 ;    break;
  case 0xFF30CF: //1
  return 1 ;    break;
  case 0xFF18E7: //2
  return 2 ;    break;
  case 0xFF7A85: //3
  return 3 ;    break;
  case 0xFF10EF: //4
  return 4 ;    break;
  case 0xFF38C7: //5
  return 5 ;    break;
  case 0xFF5AA5: //6
  return 6 ;    break;
  case 0xFF42BD: //7
  break;
  case 0xFF4AB5: //8
  break;
  case 0xFF52AD: //9
  break;
  case 0xFFFFFFFF: //REPEAT
  break;  

  default: /** cas de remise à zero */
  return 120;
  }
  delay(300);
} //END irReceiver
