#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SoftwareSerial.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)
#define REQUEST_CNT 8
#define RESPONSE_CNT 9

#define LIGHT_MAX_VOLT 942.0 // max 4.6V
#define LIGHT_MAX_LUX 4100.0 // 4100lux at 4.6V

Adafruit_BME280 bme; // I2C

SoftwareSerial serial(11, 10);
const uint8_t getppm[REQUEST_CNT]         = {0xff, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t zerocalib[REQUEST_CNT]      = {0xff, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t spancalib[REQUEST_CNT]      = {0xff, 0x01, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t autocalib_on[REQUEST_CNT]   = {0xff, 0x01, 0x79, 0xA0, 0x00, 0x00, 0x00, 0x00};
const uint8_t autocalib_off[REQUEST_CNT]  = {0xff, 0x01, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00};

int co2ppm = 0;
int co2tmp = 0;
int co2stt = 0;
int light = 0;

String cmd = "";

void setup() {
    Serial.begin(115200);
    serial.begin(9600);

    bool status;
    status = bme.begin(0x76);  
    if (!status) {
        while (1);
    }
}

void loop() {
    if(Serial.available() > 0) {
        cmd = Serial.readStringUntil('\n');
    } else {
      cmd = "";
    }
    if(cmd == "get") {
        light = analogRead(0) / LIGHT_MAX_VOLT * LIGHT_MAX_LUX;
        getCO2Data();
        printValues();
    }
}

void printValues() {
    Serial.print("{");
    Serial.print("\"time\":");
    Serial.print(millis());
    Serial.print(",");
    Serial.print("\"temp\":");
    Serial.print(bme.readTemperature());
    Serial.print(",");
    Serial.print("\"co2temp\":");
    Serial.print(co2tmp);
    Serial.print(",");
    Serial.print("\"pressure\":");
    Serial.print(bme.readPressure() / 1000.0F);
    Serial.print(",");
    Serial.print("\"altutude\":");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.print(",");
    Serial.print("\"humidity\":");
    Serial.print(bme.readHumidity());
    Serial.print(",");
    Serial.print("\"co2\":");
    Serial.print(co2ppm);
    Serial.print(",");
    Serial.print("\"co2status\":");
    Serial.print(co2stt);
    Serial.print(",");
    Serial.print("\"luminance\":");
    Serial.print(light);
    Serial.println("}");
}

void getCO2Data() {
  uint8_t buf[RESPONSE_CNT];
  for(int i = 0; i < RESPONSE_CNT; i++) {
    buf[i] = 0;
  }
  int co2 = 0;
  int co2temp = 0;
  int co2status = 0;
  serial.write(getppm, REQUEST_CNT);
  serial.write(checksum(getppm));
  serial.flush();
  int i = 0;
  while(serial.available() == 0) {
    if(++i > 100) {
      return;
    }
    delay(10);
  }
  serial.readBytes(buf, RESPONSE_CNT);
  if(buf[0] == 0xff && buf[1] == 0x86 && checksum(buf) == buf[RESPONSE_CNT - 1]) {
    co2 = (buf[2] << 8) + buf[3];
    co2temp = buf[4] - 40;
    co2status = buf[5];
  } else {
    co2 = co2temp = co2status = -1;
  }
  co2ppm = co2;
  co2tmp = co2temp;
  co2stt = co2status;
}
uint8_t checksum( uint8_t cmd[] ) {
  uint8_t sum = 0x00;
  for(int i = 1; i < REQUEST_CNT; i++) {
    sum += cmd[i];
  }
  sum = 0xff - sum + 0x01;
  return sum;
}
