#include <Adafruit_MCP3008.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <LP50XX.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>

#include "WiFi.h"
#include "esp_wpa2.h"
#include "esp_wifi.h"

#define PIN_SPI_CS_ADC  D2
#define PIN_SPI_CS_SD   D3

#define EAP_TLS  0
#define EAP_PEAP 1
#define EAP_TTLS 2

#define EXAMPLE_WIFI_SSID       "<<<user_ssid>>>"                   // SSID (network name) for the example to connect to.
#define EXAMPLE_EAP_METHOD      EAP_PEAP                            // EAP method (TLS, PEAP or TTLS) for the example to use.
#define EXAMPLE_EAP_ID          "<<<user_name>>>"                   // Identity in phase 1 of EAP procedure.
#define EXAMPLE_EAP_USERNAME    "<<<user_name>>>"                   // Username for EAP method (PEAP and TTLS).
#define EXAMPLE_EAP_PASSWORD    "<<<user_password>>>"    			// Password for EAP method (PEAP and TTLS).

String HOST_NAME    = "<<<user_URL>>>";  							// change to your Servers address
String PATH_NAME    = "/insert_data.php";
String queryString  = "";

Adafruit_MCP3008  mcp3008;
LP50XX            lp5012;
File              myFile;
String            fileline;

int       count = 0;
int       light_level;
int       adc_Array[8];

void initialise_wifi(void)
{

    Serial.println("Connecting to WIFI");

//Home
    //WiFi.mode(WIFI_STA);
    //WiFi.begin("<<<home_SSID>>>", "<<<home_password>>>");

  //Enterprise
    WiFi.begin(EXAMPLE_WIFI_SSID, NULL, 0 , NULL, false);

    if( esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EXAMPLE_EAP_ID, strlen(EXAMPLE_EAP_ID)) ){
        Serial.println("Failed to set WPA2 Identity");
        return;
    }
    if (EXAMPLE_EAP_METHOD == EAP_PEAP || EXAMPLE_EAP_METHOD == EAP_TTLS) {
        if( esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EXAMPLE_EAP_USERNAME, strlen(EXAMPLE_EAP_USERNAME)) ){
            Serial.println("Failed to set WPA2 Username");
            return;
        }
        if( esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EXAMPLE_EAP_PASSWORD, strlen(EXAMPLE_EAP_PASSWORD)) ){
            Serial.println("Failed to set WPA2 Password");
            return;
        }
    }
    if( esp_wifi_sta_wpa2_ent_enable() ){
        Serial.println("Failed to enable WPA2");
        return;
    }
    if( esp_wifi_connect() != ESP_OK) {
      Serial.println("Failed to connect to wifi");
      return;
    }
 
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");        
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address set: ");
    Serial.print("IP:  ");
    Serial.println(WiFi.localIP());
    Serial.print("MASK:  ");
    Serial.println(WiFi.subnetMask());
    Serial.print("GATEWAY:  ");
    Serial.println(WiFi.gatewayIP());
}


void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while(!Serial);              // Execute after turning on the serial monitor
  delay(500);

  pinMode(PIN_SPI_CS_ADC, OUTPUT);
  pinMode(PIN_SPI_CS_SD,  OUTPUT);
  
  //WIFI
    Serial.println("Initializing WIFI...");
      initialise_wifi();

  //Setup LP5012
    Serial.println("Initializing LP5012...");
      lp5012.Begin();      

  //Setup MCP3008 Chip Select
    Serial.println("Initializing MCP3008...");
      mcp3008.begin(PIN_SPI_CS_ADC); 

  //SD
    Serial.println("Initializing SD card...");
    
    if (!SD.begin(PIN_SPI_CS_SD)) {
      Serial.println("initialization failed!");
      return;
    }
    Serial.println("initialization done.");

  //Setup done : LED Display
    for (int i = 0; i <= 11; i++) {
      //All OFF
      for (int j = 0; j <= 11; j++) {
        lp5012.SetOutputColor(j, 0);
      }      
      //Just one ON
      lp5012.SetOutputColor(i, 255);
      delay(100);
    }
    //Last one OFF again
    lp5012.SetOutputColor(11,0);

} //Setup



void loop() {

  //#########################################################
  // ADCs
  //#########################################################
  queryString = "?SensorString=ADCs";

  Serial.print("["); Serial.print(count); Serial.print("]\t");
  
  for (int chan=0; chan<8; chan++) {

    adc_Array[chan] = mcp3008.readADC(chan);
    
    Serial.print(adc_Array[chan]); Serial.print("\t");

    queryString = queryString + "[" + adc_Array[chan] + "]";
  }
  Serial.println( );  

  //add a # to end
  //  queryString = queryString + "#";  
  //#########################################################



  //#########################################################
  // Update SD
  //#########################################################
    //Open File
      myFile = SD.open("/log.txt", FILE_APPEND);
    //Write to File
      fileline = "ADCs";
      for (int chan=0; chan<8; chan++) {
        fileline = fileline + "[" + adc_Array[chan] + "]";
      }
      myFile.println( fileline );
    // close the file:
      myFile.close();



  //#########################################################
  // LEDs (Light Level from 0 to 3072 (12*256) ) or ADC reading*3
  //#########################################################
  light_level = adc_Array[7] * 3;
  for (int i = 0; i <= 11; i++) {
    if ( light_level > 255 ) {
      lp5012.SetOutputColor(i, 255);  
      light_level = light_level - 255;
      Serial.print("M");
    }
    else {
      lp5012.SetOutputColor(i, light_level);  
      light_level = 0;      
      Serial.print("L");
    }
  }
  Serial.println();



  count++;
  delay(1000);
}
