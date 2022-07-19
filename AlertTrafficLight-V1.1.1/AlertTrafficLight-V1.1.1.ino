/*******************************************************************************************
 ** Autor: Mario Ammerschuber                                                             **
 ** Datum: Juni 2022                                                                      **
 ** Version: 1.11                                                                         ** 
 ** Lizenz:  Creative Commons Zero v1.0 Universal                                         **
 *******************************************************************************************
*/

/************************( Importieren der Bibliotheken )************************/

#include <ESP8266WiFi.h>                    // über Bibliothek installieren
#include <Timezone.h>                       
#include <TimeLib.h>                        
#include <WiFiUdp.h>
#include "LittleFS.h"                       // Little Filesystem - über Bibliothek installieren
#include <WiFiManager.h>                    // Bibliothek "WiFiManager" by tablatronix [Version 2.0.3-alpha]  or  [2.0.5-beta ]
#include <string.h>
#include <pitches.h>                        // for passiv Buzzer

/*#define ROT 2     // NodeMCU v3 D4 -> GPIO 2
  #define GELB 0    // NodeMCU v3 D3 -> GPIO 0
  #define GRUEN 4   // NodeMCU v3 D2 -> GPIO 4*/
#define GRUEN 2 // NodeMCU v3 D4 -> GPIO 2
#define GELB 0  // NodeMCU v3 D3 -> GPIO 0
#define ROT 4   // NodeMCU v3 D2 -> GPIO 4

// ####################################################################################################
const char* ssid = "";     // SSID of local network 
String lssid = ""; // SSID of local network 
const char* password = "";   // Password on network 
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

WiFiManager wifiManager; // Instanz von WiFiManager

// passiv Buzzer Init
const int buzzerPin = D1; //for ESP8266 Microcontroller (D4 dont work with ESP8266 Microcontroller)
// const int buzzerPin = -1; // no Buzzer

/********************( Definieren der Globale Variablen und Konstanten)********************/
int h, m, s, w, mo, ye, d; // Time
String fsh,fsm,fss;
int waitloop = 0;
int WiFiConnectLoop = 0;
int WiFiConnectLoop2 = 0;
int MaxWiFiConnectLoop = 30;  // Maximale Anzahl Loops bei Verbindung mit dem WLAN 
bool ClockWiFiConnectFail = false; // false standard
//flag for saving data WiFiManager
bool shouldSaveConfig = false;
unsigned long NoWiFiTime = 0;
bool WeekEnd = false;
int StartStopServers = 0;
int toggle = 0;

bool littleFSInit = true;
 
//callback notifying us of the need to save config WiFiManager
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;}

/*****************************************( Setup )****************************************/
void setup() {
 Serial.begin(115200); // Serielle Verbindung mit 115200 Baud
  delay(250); //waits for data
  clearscreen(); // Serial Monitor clear screen

  // Init passiv Buzzer
  if (buzzerPin != -1) {
  pinMode(buzzerPin, OUTPUT);
  noTone(buzzerPin);} // kein Ton

  LoadAdjustments(); // SSID und Pawwort für WiFi laden

// ***First Start***
     // Little-Filesystem formatieren
     // LittleFS.format(); // alle Dateien  danch wieder deaktivieren und nochmal hochladen  

// WiFiManager initialisieren
Serial.println("WiFiManager (Accees Point) initialisieren ");
if ((String)ssid == ""){wifiManager.resetSettings();} // alle Einstellungen zurücksetzen
// wifiManager.setCountry("DE"); // Ländereinstellung
wifiManager.setTimeout(650); // Nach 650 sekunden AccessPoint abschalten
wifiManager.setConnectTimeout(60);
// wifiManager.setSaveConnect(false); // do not connect, only save
wifiManager.setConfigPortalTimeout(600); //sets timeout until configuration portal gets turned off - useful to make it all retry or go to sleep in seconds
// wifiManager.setDarkMode(false); // Dunkler Hintergrund
wifiManager.setScanDispPerc(true);  // show scan RSSI as percentage, instead of signal stength graphic
wifiManager.setBreakAfterConfig(true); // needed to use saveWifiCallback
// wifiManager.setParamsPage(true); // move params to seperate page, not wifi, do not combine with setmenu!
wifiManager.setSaveConfigCallback(saveConfigCallback);
// wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0)); // andere IP als die aktuelle  
  
// Pins als Output festlegen.
pinMode(ROT, OUTPUT);
pinMode(GELB, OUTPUT);
pinMode(GRUEN, OUTPUT);

Serial.println("*** Alert-Traffic-Light ***");
Serial.println("");

if ((String)ssid == "") {
    Serial.println("");
    Serial.println("***** First Start  *****"); // erster Start der Uhr
    Serial.println("");
    } 

  // Initial LED's
  SwitchTrafficLight(true,false,false,true); // Rot
  MyWaitLoop(2000);
  SwitchTrafficLight(false,true,false,true); // gelb
  MyWaitLoop(2000); 
  SwitchTrafficLight(false,false,true,true); // grün
  MyWaitLoop(2000); 

  // WiFi Passwort - only for DebugModus
  // password = "wrongpass1234567890";

if ((String)ssid != ""){
  WiFi.begin(ssid, password);
  Serial.print("Verbinde mit WiFi SSID: ");
  Serial.println(ssid);
  wificonnect(); // WLAN Verbindung herstellen
  } else {
  ClockWiFiConnectFail = true;
  Serial.println("");
  Serial.println("");} 
  
  if (ClockWiFiConnectFail == false)
  {
  Serial.println("UDP-Server starten ...");
  WiFiUDPStart();  // UPD Server starten 
  MyWaitLoop(250); //waits for data
  
  getTimeLocal(); // aktuelle Zeit holen 
  
  // LED's Initialisieren
  if (FormatedTime() < 928){
  SwitchTrafficLight(false,false,true, true); // grün
  }
  if (FormatedTime() >= 928 && FormatedTime() < 930){
  SwitchTrafficLight(false,true,false, true); // gelb
  }
  if (FormatedTime() >= 930 && FormatedTime() < 1000){
  SwitchTrafficLight(true,false,false, true); // Rot
  }
  if (FormatedTime() >= 1000 && FormatedTime() < 1100){
  SwitchTrafficLight(false,false,true, true); // grün
  }
  if (FormatedTime() >= 1100 && FormatedTime() < 1110){
  SwitchTrafficLight(true,false,false, true); // Rot
  }
  if (FormatedTime() >= 1110 && FormatedTime() < 1200){
  SwitchTrafficLight(false,false,true, true); // grün
  }
  if (FormatedTime() >= 1200 && FormatedTime() < 1400){
  SwitchTrafficLight(true,false,false, true); // Rot
  }
  if (FormatedTime() >= 1401 && FormatedTime() < 1430){
  SwitchTrafficLight(false,false,true, true); // grün
  }
  if (FormatedTime() >= 1430 && FormatedTime() < 1500){
  SwitchTrafficLight(true,false,false, true); // Rot
  }
  if (FormatedTime() >= 1500 && FormatedTime() < 1530){
  SwitchTrafficLight(false,false,true, true); // grün
  }
  if (FormatedTime() >= 1530 && FormatedTime() < 1600){
  SwitchTrafficLight(true,false,false, true); // Rot
  }
  if (FormatedTime() >= 1600 && FormatedTime() < 1700){
  SwitchTrafficLight(false,false,true, true); // grün
  }
  // Nach 17 Uhr Ampel ausschalten
 if (FormatedTime() >= 1700){
 SwitchTrafficLight(false,false,false, true); // aus
  }

// Get all information of your LittleFS
    if (littleFSInit == true)
    {
    Serial.println(F("Little Filesystem Init - done."));
    FSInfo fs_info;
    LittleFS.info(fs_info);
    Serial.println("File sistem info.");
    Serial.print("Total space:      ");
    Serial.print(fs_info.totalBytes);
    Serial.println("byte");
    Serial.print("Total space used: ");
    Serial.print(fs_info.usedBytes);
    Serial.println("byte");
    Serial.print("Block size:       ");
    Serial.print(fs_info.blockSize);
    Serial.println("byte");
    Serial.print("Page size:        ");
    Serial.print(fs_info.totalBytes);
    Serial.println("byte");
    Serial.print("Max open files:   ");
    Serial.println(fs_info.maxOpenFiles);
    Serial.print("Max path length:  ");
    Serial.println(fs_info.maxPathLength);
    Serial.println();
    // Open dir folder
    Dir dir = LittleFS.openDir("/");
    // Cycle all the content
    while (dir.next()) {
        // get filename
        Serial.print(dir.fileName());
        Serial.print(" - ");
        // If element have a size display It else write 0
        if(dir.fileSize()) {
            File f = dir.openFile("r");
            Serial.println(f.size());
            f.close();
        }else{
            Serial.println("0");
        }   
    }
   Serial.println("_______________________________");
   Serial.println("");
   }

  BuzzerBeep(); // Start Beep
  Serial.println("");
  Serial.println("Alert-Traffic-Light gestartet ...");
  }
  else
  { // ************* kein Password für SSID ***************
  // Serial.println("[Loaded-WIFI] SSID: " + (String)ssid); // Only for Debuging 
  // Serial.println("[Loaded-WIFI] PASS: " + (String)password); // Only for Debuging 
     
  if ((String)ssid != ""){ 
  Serial.println("WLAN-Verbindung fehlgeschlagen!");  
  MyWaitLoop(100);} // kurze Pause
  
  
  else {
  Serial.println("Alert-Traffic-Light  - Starte Access Point ... [Alert-Traffic-Light]"); 
  SwitchTrafficLight(false,false,false,true); // Rot/Gelb/Grün - Aus
  Serial.println("");
  
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  wifiManager.autoConnect("Alert-Traffic-Light",""); // SSID,  SSID-Passwort
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if (shouldSaveConfig && littleFSInit == true){
    
  SaveAdjustments(); // Eingaben im FileSystem speichern
  
  // Reset auslösen
  Serial.println("Alert-Traffic-Light wird neu gestartet"); 
  ESP.restart();  
  }
   else
   { // Zugangsdaten wurden nicht gespeichert
  SwitchTrafficLight(true,false,false,false); // Rot  
  Serial.println("+++ WiFI - Zugangsdaten wurden nicht gespeichert ! +++");
   // Reset auslösen 
  Serial.println("Alert-Traffic-Light wird neu gestartet");
  ESP.restart();  
    } 
       }
  }    
}
/*************************************( Setup - Ende )*************************************/
 
/*************************************(Hauptprogramm)**************************************/
void loop() {
// Check ob Internetverbindung noch besteht
  WiFi.mode(WIFI_STA);
  if (WiFi.status() != WL_CONNECTED) 
  {
  wificonnect(); // WLAN-Verbindung herstellen
  if (ClockWiFiConnectFail == true)
  { // Wenn WiFi Connection fail
  Serial.println("WLAN-Verbindung: getrennt - "+FormatedDateTime());
  WiFiConnectLoop2 += 1; 
  if (WiFiConnectLoop2 > 10) {
  WiFiUDPStop(); // UDP Server stoppen  
  // Nach 10 durchläufen Access Point starten
  WiFiConnectLoop2 = 0;
  // Initial LED's
  SwitchTrafficLight(true,true,true,true); // Alle An
  MyWaitLoop(2000);
  SwitchTrafficLight(false,false,false,true); // Alle Aus
  // Little-Filesystem formatieren
  LittleFS.format(); // alle Dateien löschen 
  Serial.println("");
  Serial.println("Alert-Traffic-Light wird neu gestartet");
  ESP.restart();}}}

  if (ClockWiFiConnectFail == false)
  {  // wenn WiFi Connection OK
  getTimeLocal(); // aktuelle Uhrzeit holen
  
 //  *** nur Werktags ***
 if (w!= 1 && w != 7) {
 WeekEnd = false; 
 if (FormatedTime() >= 928 && FormatedTime() < 930) {beep(NOTE_E3, 150);BlinkLED(GELB);}
 if  (String(h)+":"+String(m)+":"+String(s) == "9:30:1"){
 SwitchTrafficLight(true,false,false,false); // Rot
 }
 if  (String(h)+":"+String(m)+":"+String(s) == "10:0:1"){
 SwitchTrafficLight(false,true,false,false); // gelb
 }
 if  (String(h)+":"+String(m)+":"+String(s) == "10:5:1"){
 SwitchTrafficLight(false,false,true,false); // grün
 }
 if (FormatedTime() >= 1055 && FormatedTime() < 1100) {beep(NOTE_E3, 150);BlinkLED(GELB);}
 if  (String(h)+":"+String(m)+":"+String(s) == "11:0:1"){
 SwitchTrafficLight(true,false,false,false); // Rot
 }
 if  (String(h)+":"+String(m)+":"+String(s) == "11:10:1"){
 SwitchTrafficLight(false,true,false,false); // gelb
 }
 if  (String(h)+":"+String(m)+":"+String(s) == "11:15:1"){
 SwitchTrafficLight(false,false,true,false); // grün
 }
 if (FormatedTime() >= 1155 && FormatedTime() < 1200) {beep(NOTE_E3, 150);BlinkLED(GELB);}
 if  (String(h)+":"+String(m)+":"+String(s) == "12:0:1"){
 SwitchTrafficLight(true,false,false,false); // Rot
 }
 if  (String(h)+":"+String(m)+":"+String(s) == "14:0:1"){
 SwitchTrafficLight(false,true,false,false); // gelb
 }
 if  (String(h)+":"+String(m)+":"+String(s) == "14:3:1"){
 SwitchTrafficLight(false,false,true,false); // grün
 }
 if (FormatedTime() >= 1425 && FormatedTime() < 1430) {beep(NOTE_E3, 150);BlinkLED(GELB);}
 if  (String(h)+":"+String(m)+":"+String(s) == "14:30:1"){
 SwitchTrafficLight(true,false,false,false); // Rot
 }
 if  (String(h)+":"+String(m)+":"+String(s) == "15:0:1"){
 SwitchTrafficLight(false,true,false,false); // gelb
 }
 if  (String(h)+":"+String(m)+":"+String(s) == "15:3:1"){
 SwitchTrafficLight(false,false,true,false); // grün
 }
 if (FormatedTime() >= 1525 && FormatedTime() < 1530) {beep(NOTE_E3, 150);BlinkLED(GELB);}
 if  (String(h)+":"+String(m)+":"+String(s) == "15:30:1"){
 SwitchTrafficLight(true,false,false,false); // Rot
 }
 if  (String(h)+":"+String(m)+":"+String(s) == "15:58:1"){
 SwitchTrafficLight(false,true,false,false); // gelb
 }
 if (w != 6) {
 if  (String(h)+":"+String(m)+":"+String(s) == "16:2:1"){
 SwitchTrafficLight(false,false,true,false); // grün
 }

 // Feierabend steht bevor ... (Montag-Donnerstag)
 if (FormatedTime() >= 1658 && FormatedTime() < 1700) {beep(NOTE_E3, 150);BlinkLED(GRUEN);}
 
 // Ampel ausschalten (Feierabend)
 if  (String(h)+":"+String(m)+":"+String(s) == "17:0:1"){
 SwitchTrafficLight(false,false,false,false); // aus
 }
 } else
     {
   // Feierabend steht bevor ... (Freitag)
   if (FormatedTime() >= 1558 && FormatedTime() < 1600) {beep(NOTE_E3, 150);BlinkLED(GRUEN);} 
        
   // Ampel ausschalten (Feierabend) - Freitag
   if  (String(h)+":"+String(m)+":"+String(s) == "16:0:1"){
   SwitchTrafficLight(false,false,false,false); // aus
     }      
     }
  } else
     {
    // *** Am Wochenende die Ampel ausschalten *** 
    if (!WeekEnd) { 
    SwitchTrafficLight(false,false,false,false);} // aus
    WeekEnd = true;
     }
  } else
    { // wenn kein WLAN
    BlinkLED(ROT);
    }   
}
 /*************************************(Hauptprogramm - Ende)**************************************/
