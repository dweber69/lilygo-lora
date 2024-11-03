// ============================================================================
//
//   RadioLib LoRaWAN with Lilygo T3S3 LoRa E-Paper
//
//   https://www.lilygo.cc/products/ts-s3-epaper
//   https://www.bastelgarage.ch/lilygo/lilygo-lora-t3s3-e-paper-esp32-s3-868mhz-sx1262
//
// ----------------------------------------------------------------------------
//   (c) 2024 Daniel Weber
// ============================================================================
#include "secrets.h"
#include <FS.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <GxEPD.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxDEPG0213BN/GxDEPG0213BN.h> // 2.13" b/w  form DKE GROUP
#include <RadioLib.h>
#include <ArduinoBLE.h>
#include "WiFi.h"



// SD definition
#define SDCARD_SCLK     14
#define SDCARD_MISO     2
#define SDCARD_MOSI     11
#define SDCARD_CS       13
SPIClass SDSPI(HSPI);


// EPD definition
// EPD sharing SPI with SD card
#define EDP_BUSY_PIN            48
#define EDP_RSET_PIN            47
#define EDP_DC_PIN              16
#define EDP_CS_PIN              15
GxIO_Class io(SDSPI, EDP_CS_PIN, EDP_DC_PIN, EDP_RSET_PIN);
GxEPD_Class display(io, EDP_RSET_PIN, EDP_BUSY_PIN);


// LoRa radio definition
#define USING_SX1262
#define RADIO_CS_PIN    7
#define RADIO_DIO1_PIN  33
#define RADIO_RST_PIN   8
#define RADIO_BUSY_PIN  34
#define RADIO_SCLK_PIN  5
#define RADIO_MISO_PIN  3
#define RADIO_MOSI_PIN  6
SX1262 radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);

// LoRa OTA definitons
// define them in secrets.h to keep them separate from code
#ifndef LORA_TTN_JOIN_EUI
#define LORA_TTN_JOIN_EUI   0x................
#endif
#ifndef LORA_TTN_DEV_EUI
#define LORA_TTN_DEV_EUI    0x................
#endif
#ifndef LORA_TTN_APP_KEY
#define LORA_TTN_APP_KEY    0x.., 0x.., 0x.., 0x.., 0x.., 0x.., 0x.., 0x.., 0x.., 0x.., 0x.., 0x.., 0x.., 0x.., 0x.., 0x..
#endif
#ifndef LORA_TTN_NW_KEY
#define LORA_TTN_NW_KEY     0x.., 0x.., 0x.., 0x.., 0x.., 0x.., 0x.., 0x.., 0x.., 0x.., 0x.., 0x.., 0x.., 0x.., 0x.., 0x..
#endif
uint64_t joinEUI =   LORA_TTN_JOIN_EUI;
uint64_t devEUI  =   LORA_TTN_DEV_EUI;
uint8_t appKey[] = { LORA_TTN_APP_KEY };
uint8_t nwkKey[] = { LORA_TTN_NW_KEY };
const LoRaWANBand_t region = EU868;
const uint8_t subBand = 0;

// create the LoRaWAN node
LoRaWANNode node(&radio, &region, subBand);

// RadioLib result code to text - these are error codes that can be raised when using LoRaWAN
// however, RadioLib has many more - see https://jgromes.github.io/RadioLib/group__status__codes.html for a complete list
String stateDecode(const int16_t result) {
  switch (result) {
  case RADIOLIB_ERR_NONE:
    return "ERR_NONE";
  case RADIOLIB_ERR_CHIP_NOT_FOUND:
    return "ERR_CHIP_NOT_FOUND";
  case RADIOLIB_ERR_PACKET_TOO_LONG:
    return "ERR_PACKET_TOO_LONG";
  case RADIOLIB_ERR_RX_TIMEOUT:
    return "ERR_RX_TIMEOUT";
  case RADIOLIB_ERR_CRC_MISMATCH:
    return "ERR_CRC_MISMATCH";
  case RADIOLIB_ERR_INVALID_BANDWIDTH:
    return "ERR_INVALID_BANDWIDTH";
  case RADIOLIB_ERR_INVALID_SPREADING_FACTOR:
    return "ERR_INVALID_SPREADING_FACTOR";
  case RADIOLIB_ERR_INVALID_CODING_RATE:
    return "ERR_INVALID_CODING_RATE";
  case RADIOLIB_ERR_INVALID_FREQUENCY:
    return "ERR_INVALID_FREQUENCY";
  case RADIOLIB_ERR_INVALID_OUTPUT_POWER:
    return "ERR_INVALID_OUTPUT_POWER";
  case RADIOLIB_ERR_NETWORK_NOT_JOINED:
	  return "RADIOLIB_ERR_NETWORK_NOT_JOINED";
  case RADIOLIB_ERR_DOWNLINK_MALFORMED:
    return "RADIOLIB_ERR_DOWNLINK_MALFORMED";
  case RADIOLIB_ERR_INVALID_REVISION:
    return "RADIOLIB_ERR_INVALID_REVISION";
  case RADIOLIB_ERR_INVALID_PORT:
    return "RADIOLIB_ERR_INVALID_PORT";
  case RADIOLIB_ERR_NO_RX_WINDOW:
    return "RADIOLIB_ERR_NO_RX_WINDOW";
  case RADIOLIB_ERR_INVALID_CID:
    return "RADIOLIB_ERR_INVALID_CID";
  case RADIOLIB_ERR_UPLINK_UNAVAILABLE:
    return "RADIOLIB_ERR_UPLINK_UNAVAILABLE";
  case RADIOLIB_ERR_COMMAND_QUEUE_FULL:
    return "RADIOLIB_ERR_COMMAND_QUEUE_FULL";
  case RADIOLIB_ERR_COMMAND_QUEUE_ITEM_NOT_FOUND:
    return "RADIOLIB_ERR_COMMAND_QUEUE_ITEM_NOT_FOUND";
  case RADIOLIB_ERR_JOIN_NONCE_INVALID:
    return "RADIOLIB_ERR_JOIN_NONCE_INVALID";
  case RADIOLIB_ERR_N_FCNT_DOWN_INVALID:
    return "RADIOLIB_ERR_N_FCNT_DOWN_INVALID";
  case RADIOLIB_ERR_A_FCNT_DOWN_INVALID:
    return "RADIOLIB_ERR_A_FCNT_DOWN_INVALID";
  case RADIOLIB_ERR_DWELL_TIME_EXCEEDED:
    return "RADIOLIB_ERR_DWELL_TIME_EXCEEDED";
  case RADIOLIB_ERR_CHECKSUM_MISMATCH:
    return "RADIOLIB_ERR_CHECKSUM_MISMATCH";
  case RADIOLIB_ERR_NO_JOIN_ACCEPT:
    return "RADIOLIB_ERR_NO_JOIN_ACCEPT";
  case RADIOLIB_LORAWAN_SESSION_RESTORED:
    return "RADIOLIB_LORAWAN_SESSION_RESTORED";
  case RADIOLIB_LORAWAN_NEW_SESSION:
    return "RADIOLIB_LORAWAN_NEW_SESSION";
  case RADIOLIB_ERR_NONCES_DISCARDED:
    return "RADIOLIB_ERR_NONCES_DISCARDED";
  case RADIOLIB_ERR_SESSION_DISCARDED:
    return "RADIOLIB_ERR_SESSION_DISCARDED";
  }
  return "See https://jgromes.github.io/RadioLib/group__status__codes.html";
}

// WLAN definition
// define them in secrets.h to keep them separate from code
#ifndef WIFI_SSID
#define WIFI_SSID      insert_your_wlan_ssid
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD  insert_your_wlan_password
#endif

// other defines
#define BOARD_LED_PIN   37


// global variables
ulong  lora_timer = 0;
uint16_t lora_uplink_interval_seconds = 5 * 60;


// ============================================================================
//
// initialize board
//
// ============================================================================
void initializeBoard() {

  // setup serial port
  Serial.begin(115200);
  delay(5000);
  Serial.println(F("\nInitializing board... "));

  // turn on green led
  pinMode(BOARD_LED_PIN, OUTPUT);
//  digitalWrite(BOARD_LED_PIN, HIGH);
  
  //setup SD card
  Serial.println(F("Initializing sd card"));
  pinMode(SDCARD_MISO, INPUT_PULLUP);
  SDSPI.begin(SDCARD_SCLK, SDCARD_MISO, SDCARD_MOSI, SDCARD_CS);
  if (!SD.begin(SDCARD_CS, SDSPI)) {
      Serial.println("SD init failed");
  } else {
      Serial.println("SD init success");
      Serial.printf("SD card size: %.2f GB\n",SD.cardSize() / 1024.0 / 1024.0 / 1024.0);
  }


  // initialize display
  Serial.println(F("Initializing display"));
  display.init(); 
  display.setTextColor(GxEPD_BLACK);
  display.setRotation(3);
  display.fillScreen(GxEPD_WHITE);
//  display.setFont(&FreeMonoBold9pt7b);
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Display Ready");
  display.update();


  // initialize radio SPI
  SPI.begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN);


  // initialize LoRa radio
  Serial.println("Initializing the radio");
  int16_t state = radio.begin();
  debug(state != RADIOLIB_ERR_NONE, "Initializing radio failed", state, true);

  // Setup the OTAA session information
  state = node.beginOTAA(joinEUI, devEUI, nwkKey, appKey);
  debug(state != RADIOLIB_ERR_NONE, "Initializing node failed", state, true);

  Serial.println("Joining the LoRaWAN Network");
  state = node.activateOTAA();
  debug(state != RADIOLIB_LORAWAN_NEW_SESSION, "Join failed", state, true);

  Serial.println("LoRa ready");


  // scan wifi
  Serial.println("Sanning Wifi...");
  WiFi.mode(WIFI_STA);
  int n = WiFi.scanNetworks();
  Serial.println("Scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    Serial.println("Nr | SSID                             | RSSI | CH");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.printf("%2d", i + 1);
      Serial.print(" | ");
      Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
      Serial.print(" | ");
      Serial.printf("%4ld", WiFi.RSSI(i));
      Serial.print(" | ");
      Serial.printf("%2ld", WiFi.channel(i));
      Serial.println();
    }
  }
  Serial.println("");
  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();


  // start bluetooth and scan for Shelly H&T sensors
  Serial.println("Sanning Bluetooth LE...");
  if(!BLE.begin()) {
    Serial.println("Starting Bluetooth Low Energy module failed!");
  }
  Serial.println("Scanning...");
  BLE.scanForName("SBHT-003C");


}


// ============================================================================
//
// helper function to display any issues
//
// ============================================================================
void debug(bool failed, const char* message, int state, bool halt) {
  if(failed) {
    Serial.print(message);
    Serial.print(" - ");
    Serial.print(stateDecode(state));
    Serial.print(" (");
    Serial.print(state);
    Serial.println(")");
    while(halt) { delay(1); }
  }
}


// ============================================================================
//
// helper function to display a byte array
//
// ============================================================================
void arrayDump(uint8_t *buffer, uint16_t len) {
  for(uint16_t c = 0; c < len; c++) {
    char b = buffer[c];
    if(b < 0x10) { Serial.print('0'); }
    Serial.print(b, HEX);
  }
  Serial.println();
}


// ============================================================================
//
//  Decode Shelly H&T Sensor Data
//
// ============================================================================
void decodeSensor(uint8_t adv[], int len, String adr) {

  int  index = 0;
  int  bat;
  int  temp;
  int  hum;
  int eir_len;
  int eir_type;

  while(index < len) {
      eir_len = adv[index];
      eir_type = adv[index+1];


      if(eir_type == 0x16) {
        // custom advertisement, what we want

        int i = index+5;
        while(i < index+eir_len) {

          if(adv[i] == 0x00) {
            // packet number
            Serial.print(" #:");
            Serial.print(adv[i+1]);
          }
          if(adv[i] == 0x01) {
            // battery
            bat = adv[i+1];
            Serial.print(" Bat:");
            Serial.print(bat);
          }
          if(adv[i] == 0x2e) {
            // humidity
            hum = adv[i+1];
            Serial.print(" Hum:");
            Serial.print(hum);
          }
          if(adv[i] == 0x3a) {
            // button
            Serial.print(" Button");
          }
          if(adv[i] == 0x45) {
            // temperature
            temp = adv[i+1] + adv[i+2]*256;
            Serial.print(" Temp:");
            Serial.print((float)temp / 10, 1);
            i++;
          }
          i += 2;

        }

      }
      index += eir_len+1;
  }
  Serial.println("");

}
  

// ============================================================================
//
//  Setup routine, called when device starts
//
// ============================================================================
void setup() {

  initializeBoard();


}


// ============================================================================
//
//  Working loop
//
// ============================================================================
void loop() {


  // check if time for next lora uplink
  if((lora_timer == 0) || (millis() > (lora_timer + (lora_uplink_interval_seconds * 1000)))) {

    Serial.print("Sending uplink ");

    uint16_t uplinkIntervalSeconds = 5 * 60;
    
    // This is the place to gather the sensor inputs
    // Instead of reading any real sensor, we just generate some random numbers as example
    uint8_t value1 = radio.random(100);
    uint16_t value2 = radio.random(2000);

    // Build payload byte array
    uint8_t uplinkPayload[3];
    uplinkPayload[0] = value1;
    uplinkPayload[1] = highByte(value2);   // See notes for high/lowByte functions
    uplinkPayload[2] = lowByte(value2);
    
    arrayDump(uplinkPayload, sizeof(uplinkPayload));

    // Perform an uplink
    int16_t state = node.sendReceive(uplinkPayload, sizeof(uplinkPayload));    
    debug(state < RADIOLIB_ERR_NONE, "Error in sendReceive", state, false);

    // Check if a downlink was received 
    // (state 0 = no downlink, state 1/2 = downlink in window Rx1/Rx2)
    if(state > 0) {
      Serial.println("Received a downlink");
    } else {
      Serial.println("No downlink received");
    }

    Serial.printf("Next LoRaWan uplink in %d seconds\n", lora_uplink_interval_seconds);
    lora_timer = millis();
  }

  // check if we have bluetooth scan results
  BLEDevice peripheral = BLE.available();
  if(peripheral) {

    Serial.print("Sensor Address: ");
    Serial.print(peripheral.address());

    if(peripheral.hasLocalName()) {
      Serial.print("  Name: ");
      Serial.print(peripheral.localName());
    }

    uint8_t advertisement[64] = {0};
    int adLength = peripheral.advertisementData(advertisement, 64);

    //Serial.print(" Advertisement: ");
    //for(int i=0; i < adLength; i++) {
    //  Serial.print(advertisement[i], HEX);
    //  Serial.print(" ");
    //}
    //Serial.println("");

    decodeSensor(advertisement, adLength, peripheral.address());
  }
  else {
    // if we don't have bluetooth data we delay a bit, no need to keep the cpu running full steam through the loop
    delay(1000);
  }

}


