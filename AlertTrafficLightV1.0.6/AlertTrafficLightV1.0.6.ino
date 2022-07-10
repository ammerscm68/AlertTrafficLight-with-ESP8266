/*******************************************************************************************
 ** Autor: Mario Ammerschuber                                                             **
 ** Datum: Juni 2022                                                                      **
 ** Version: 1.06                                                                         ** 
 ** Lizenz:  Creative Commons Zero v1.0 Universal                                         **
 *******************************************************************************************
*/

/************************( Importieren der Bibliotheken )************************/

#include <ESP8266WiFi.h>                    // über Bibliothek installieren
#include <Timezone.h>                       
#include <TimeLib.h>                        
#include <WiFiUdp.h>
#include <string.h>

#define ROT 2     // NodeMCU v3 D4 -> GPIO 2
#define GELB 0    // NodeMCU v3 D3 -> GPIO 0
#define GRUEN 4   // NodeMCU v3 D2 -> GPIO 4

// ####################################################################################################
const char* ssid = "**********";     // SSID of local network 
String lssid = ""; // SSID of local network 
const char* password = "***************";   // Password on network 
String lpassword = ""; // SSID of local network 
// ####################################################################################################

// NTP Serverpool für Deutschland:
static const char ntpServerName[] = "de.pool.ntp.org";    //Finde lokale Server unter http://www.pool.ntp.org/zone/de
const int timeZone = 0;                     // 0 wenn mit <Timezone.h> gearbeitet wird !
WiFiUDP Udp;
unsigned int localPort = 1701; // lokaler Port zum Abhören von UDP-Paketen
time_t getNtpTime();
void sendNTPpacket(IPAddress &address);

// - Timezone. - //
// Bearbeiten Sie diese Zeilen entsprechend Ihrer Zeitzone und Sommerzeit.
// TimeZone Einstellungen Details https://github.com/JChristensen/Timezone
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Time (Frankfurt, Paris)
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       //Central European Time (Frankfurt, Paris)
Timezone CE(CEST, CET);

/********************( Definieren der Globale Variablen und Konstanten)********************/
int h, m, s, w, mo, ye, d; // Time
String fsh,fsm,fss;
int waitloop = 0;
int WiFiConnectLoop = 0;
int MaxWiFiConnectLoop = 30;  // Maximale Anzahl Loops bei Verbindung mit dem WLAN 
bool ClockWiFiConnectFail = false; // false standard
//flag for saving data WiFiManager
bool shouldSaveConfig = false;
int dots = 0;
unsigned long NoWiFiTime = 0;
bool WeekEnd = false;

/*****************************************( Setup )****************************************/
void setup() {
 Serial.begin(115200); // Serielle Verbindung mit 115200 Baud
  delay(250); //waits for data
  clearscreen(); // Serial Monitor clear screen
  
// Pins als Output festlegen.
pinMode(ROT, OUTPUT);
pinMode(GELB, OUTPUT);
pinMode(GRUEN, OUTPUT);

Serial.println("*** Raucherampel ***");
Serial.println("");

if ((String)ssid == "") {
    Serial.println("");
    Serial.println("***** First Start  *****"); // erster Start der Uhr
    Serial.println("");
    } 

  // Initial LED's
  digitalWrite(ROT, HIGH);
  digitalWrite(GELB, LOW);
  digitalWrite(GRUEN, LOW); 
  MyWaitLoop(2000);
  digitalWrite(ROT, LOW);
  digitalWrite(GELB, HIGH);
  digitalWrite(GRUEN, LOW);
  MyWaitLoop(2000); 
  digitalWrite(ROT, LOW);
  digitalWrite(GELB, LOW);
  digitalWrite(GRUEN, HIGH);  
  MyWaitLoop(2000); 

if ((String)ssid != ""){
  WiFi.begin(ssid, password);
  Serial.print("Verbinde mit WiFi SSID: ");
  Serial.println(ssid);
  wificonnect(); // WLAN Verbindung herstellen
  } else {
  ClockWiFiConnectFail = true;
  Serial.println("WiFi Verbindung fehlgeschlagen !");} 
  
  if (ClockWiFiConnectFail == false)
  {
  Serial.println("UDP-Server starten ...");
  WiFiUDPStart();  // UPD Server starten 
  MyWaitLoop(250); //waits for data
  
  getTimeLocal(); // aktuelle Zeit holen 
  
  // LED's Initialisieren
  if (FormatedTime() < 800){
  digitalWrite(ROT, LOW);
  digitalWrite(GELB, LOW);
  digitalWrite(GRUEN, HIGH);  
  }
  if (FormatedTime() >= 925 && FormatedTime() < 930){
  digitalWrite(ROT, LOW);
  digitalWrite(GELB, HIGH);
  digitalWrite(GRUEN, LOW);  
  }
  if (FormatedTime() >= 930 && FormatedTime() < 1000){
  digitalWrite(ROT, HIGH);
  digitalWrite(GELB, LOW);
  digitalWrite(GRUEN, LOW);  
  }
  if (FormatedTime() >= 1000 && FormatedTime() < 1100){
  digitalWrite(ROT, LOW);
  digitalWrite(GELB, LOW);
  digitalWrite(GRUEN, HIGH);  
  }
 if (FormatedTime() >= 1100 && FormatedTime() < 1110){
  digitalWrite(ROT, HIGH);
  digitalWrite(GELB, LOW);
  digitalWrite(GRUEN, LOW);  
  }
 if (FormatedTime() >= 1110 && FormatedTime() < 1200){
  digitalWrite(ROT, LOW);
  digitalWrite(GELB, LOW);
  digitalWrite(GRUEN, HIGH);  
  }
 if (FormatedTime() >= 1200 && FormatedTime() < 1400){
  digitalWrite(ROT, HIGH);
  digitalWrite(GELB, LOW);
  digitalWrite(GRUEN, LOW);  
  }
 if (FormatedTime() >= 1401 && FormatedTime() < 1430){
  digitalWrite(ROT, LOW);
  digitalWrite(GELB, LOW);
  digitalWrite(GRUEN, HIGH);
  }
 if (FormatedTime() >= 1430 && FormatedTime() < 1500){
  digitalWrite(ROT, HIGH);
  digitalWrite(GELB, LOW);
  digitalWrite(GRUEN, LOW);  
  }
 if (FormatedTime() >= 1500 && FormatedTime() < 1530){
  digitalWrite(ROT, LOW);
  digitalWrite(GELB, LOW);
  digitalWrite(GRUEN, HIGH);
  }
 if (FormatedTime() >= 1530 && FormatedTime() < 1600){
  digitalWrite(ROT, HIGH);
  digitalWrite(GELB, LOW);
  digitalWrite(GRUEN, LOW);  
  }
  if (FormatedTime() >= 1600 && FormatedTime() < 1700){
  digitalWrite(ROT, LOW);
  digitalWrite(GELB, LOW);
  digitalWrite(GRUEN, HIGH);  
  }
  // Nach 17 Uhr Ampel ausschalten
 if (FormatedTime() >= 1700){
  digitalWrite(ROT, LOW);
  digitalWrite(GELB, LOW);
  digitalWrite(GRUEN, LOW);
  }
  Serial.println("");
  Serial.println("Raucherwarnampel gestartet ...");
  }   
}
/*****************************************( Setup - Ende )****************************************/
 
/*************************************(Hauptprogramm)**************************************/
void loop() {
  if (ClockWiFiConnectFail == false)
  {  
  getTimeLocal(); // aktuelle Uhrzeit holen
 
 //  *** nur Werktags ***
 if (w!= 1 && w != 7) {
 WeekEnd = false; 
 if (FormatedTime() >= 925 && FormatedTime() < 930) {BlinkLED(GELB);}
 if  (String(h)+":"+String(m)+":"+String(s) == "9:30:1"){
 digitalWrite(ROT, HIGH);
 digitalWrite(GELB, LOW);
 digitalWrite(GRUEN, LOW);
 }
 if  (String(h)+":"+String(m)+":"+String(s) == "10:0:1"){
 digitalWrite(ROT, LOW);
 digitalWrite(GELB, HIGH);
 digitalWrite(GRUEN, LOW);
 }
 if  (String(h)+":"+String(m)+":"+String(s) == "10:5:1"){
 digitalWrite(ROT, LOW);
 digitalWrite(GELB, LOW);
 digitalWrite(GRUEN, HIGH);
 }
 if (FormatedTime() >= 1055 && FormatedTime() < 1100) {BlinkLED(GELB);}
 if  (String(h)+":"+String(m)+":"+String(s) == "11:0:1"){
 digitalWrite(ROT, HIGH);
 digitalWrite(GELB, LOW);
 digitalWrite(GRUEN, LOW);
 }
 if  (String(h)+":"+String(m)+":"+String(s) == "11:10:1"){
 digitalWrite(ROT, LOW);
 digitalWrite(GELB, HIGH);
 digitalWrite(GRUEN, LOW);
 }
 if  (String(h)+":"+String(m)+":"+String(s) == "11:15:1"){
 digitalWrite(ROT, LOW);
 digitalWrite(GELB, LOW);
 digitalWrite(GRUEN, HIGH);
 }
 if (FormatedTime() >= 1155 && FormatedTime() < 1200) {BlinkLED(GELB);}
 if  (String(h)+":"+String(m)+":"+String(s) == "12:0:1"){
 digitalWrite(ROT, HIGH);
 digitalWrite(GELB, LOW);
 digitalWrite(GRUEN, LOW);
 }
 if  (String(h)+":"+String(m)+":"+String(s) == "14:0:1"){
 digitalWrite(ROT, LOW);
 digitalWrite(GELB, HIGH);
 digitalWrite(GRUEN, LOW);
 }
 if  (String(h)+":"+String(m)+":"+String(s) == "14:3:1"){
 digitalWrite(ROT, LOW);
 digitalWrite(GELB, LOW);
 digitalWrite(GRUEN, HIGH);
 }
 if (FormatedTime() >= 1425 && FormatedTime() < 1430) {BlinkLED(GELB);}
 if  (String(h)+":"+String(m)+":"+String(s) == "14:30:1"){
 digitalWrite(ROT, HIGH);
 digitalWrite(GELB, LOW);
 digitalWrite(GRUEN, LOW);
 }
 if  (String(h)+":"+String(m)+":"+String(s) == "15:0:1"){
 digitalWrite(ROT, LOW);
 digitalWrite(GELB, HIGH);
 digitalWrite(GRUEN, LOW);
 }
 if  (String(h)+":"+String(m)+":"+String(s) == "15:3:1"){
 digitalWrite(ROT, LOW);
 digitalWrite(GELB, LOW);
 digitalWrite(GRUEN, HIGH);
 }
 if (FormatedTime() >= 1525 && FormatedTime() < 1530) {BlinkLED(GELB);}
 if  (String(h)+":"+String(m)+":"+String(s) == "15:30:1"){
 digitalWrite(ROT, HIGH);
 digitalWrite(GELB, LOW);
 digitalWrite(GRUEN, LOW);
 }
 if  (String(h)+":"+String(m)+":"+String(s) == "15:58:1"){
 digitalWrite(ROT, LOW);
 digitalWrite(GELB, HIGH);
 digitalWrite(GRUEN, LOW);
 }
 if (w != 6) {
 if  (String(h)+":"+String(m)+":"+String(s) == "16:2:1"){
 digitalWrite(ROT, LOW);
 digitalWrite(GELB, LOW);
 digitalWrite(GRUEN, HIGH);
 }

 // Feierabend steht bevor ...
 if (FormatedTime() >= 1658 && FormatedTime() < 1700) {BlinkLED(GRUEN);}
 
 // Ampel ausschalten (Feierabend)
 if  (String(h)+":"+String(m)+":"+String(s) == "17:0:1"){
 digitalWrite(ROT, LOW);
 digitalWrite(GELB, LOW);
 digitalWrite(GRUEN, LOW);
 }
 } else
     {
   // Feierabend steht bevor ... (Freitag)
   if (FormatedTime() >= 1558 && FormatedTime() < 1600) {BlinkLED(GRUEN);} 
        
   // Ampel ausschalten (Feierabend) - Freitag
   if  (String(h)+":"+String(m)+":"+String(s) == "16:0:1"){
   digitalWrite(ROT, LOW);
   digitalWrite(GELB, LOW);
   digitalWrite(GRUEN, LOW);
     }      
     }
  } else
     {
    // *** Am Wochenende die Ampel ausschalten *** 
    if (!WeekEnd) { 
    digitalWrite(ROT, LOW);
    digitalWrite(GELB, LOW);
    digitalWrite(GRUEN, LOW);} 
    WeekEnd = true;
     }
  } else
    { // wenn kein WLAN
    BlinkLED(ROT);
    }   
}
 /*************************************(Hauptprogramm - Ende)**************************************/
