#include "Arduino.h"
#include "Multi_RFID_PN5180_MCP23008.h"

#define I2C0_SDA 0
#define I2C0_SCL 1
#define I2C_Address 0x20

#define PN5180_MOSI 3
#define PN5180_MISO 4
#define PN5180_SCK 6

#define NUM_READERS 5 // max of 6. More readers would require a new multi_rfid object

Multi_RFID_PN5180_MCP23008 RFID_Readers(NUM_READERS, &Wire, I2C_Address, SPI);

void setup(){
    SPI.setRX(PN5180_MISO);
    SPI.setTX(PN5180_MOSI);
    SPI.setSCK(PN5180_SCK);
    Wire.setSCL(I2C0_SCL);
    Wire.setSDA(I2C0_SDA);

    Serial.begin(115200);
    SPI.begin();
    RFID_Readers.begin();
}

void loop(){
    RFID_Readers.update();
    uint8_t *tagData[NUM_READERS];
    for(int i = 0; i < NUM_READERS; i++){
        tagData[i] = RFID_Readers.getTagData(i);
        Serial.print("Reader ");
        Serial.print(i);
        Serial.print(" -- ");
        for(int j = 0; j < 4; j++){
            Serial.print(tagData[i][j]);
            Serial.print(' ');
        }
        Serial.println();
    }
}