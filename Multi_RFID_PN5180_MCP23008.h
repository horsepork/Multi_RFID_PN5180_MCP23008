#include "Arduino.h"
#include "Adafruit_MCP23X08.h"
#include "PN5180.h"
#include "PN5180ISO14443.h"

// #define MULTI_RFID_DEBUG_ON 0
#ifdef MULTI_RFID_DEBUG_ON
#define MULTI_READ_DEBUG Serial.print
#else
#define MULTI_READ_DEBUG
#endif

class Multi_RFID_PN5180_MCP23008{
    public:
        Multi_RFID_PN5180_MCP23008(uint8_t _numReaders, TwoWire *_wire, uint8_t _address, SPIClass& _spi) :
            numReaders(_numReaders),
            wire(_wire),
            I2C_Address(_address),
            spi(_spi){
            for(int i = 0; i < numReaders; i++){
                nfc[i] = new PN5180ISO14443(i, &mcp, I2C_Address, spi);
            }
        }
    
    private:
        uint8_t numReaders;
        TwoWire *wire;
        Adafruit_MCP23X08 mcp;
        uint8_t I2C_Address;
        SPIClass& spi;
        PN5180ISO14443* nfc[6];
        const uint8_t busyPin = 7;
        const uint8_t resetPin = 6;
        uint32_t readDelay;
        uint8_t readDelayTime = 20;
        bool updated = false;
    
    public:
        void begin(){
            mcp.begin_I2C(I2C_Address, wire);
            mcp.pinMode(resetPin, OUTPUT);
            mcp.pinMode(busyPin, INPUT);
            mcp.digitalWrite(resetPin, HIGH); // low to reset
            for(int i = 0; i < numReaders; i++){
                nfc[i]->begin();
            }
            spi.begin();
        }

        void update(uint8_t readerSelect = 0b00111111){
            if(millis() - readDelay < readDelayTime) return;
            readDelay = millis();
            for(int i = 0; i < numReaders; i++){
                if(!bitRead(readerSelect, i)) continue;
                if(nfc[i]->update()){
                    updated = true;
                }
            }
            checkErrorState();
        }

        bool isUpdated(){
            if(updated){
                updated = false;
                return true;
            }
            return false;
        }

        void incrementalUpdate(uint8_t readerSelect = 0b00111111){ // so rfid reading does not block the program so frequently
            static uint8_t currentReader = 0;
            if(bitRead(readerSelect, currentReader)){
                nfc[currentReader]->update();
            }
            ++currentReader %= numReaders;
            checkErrorState();
        }

        void checkErrorState(){
            // Serial.print("error counter -- ");
            // for(int i = 0; i < numReaders; i++){
            //     Serial.print(nfc[i]->errorCounter);
            //     Serial.print(" ");
            // }
            // Serial.println();
            for(int i = 0; i < numReaders; i++){
                if(nfc[i]->errorCounter > 10){
                    hardReset();
                    break;
                }
            }
        }

        uint8_t *getTagData(uint8_t reader){
            if(reader >= numReaders) reader = 0;
            return nfc[reader]->getTagData();
        }

        void reset(){ 
            nfc[0]->reset();
        }

        bool mcpIsConnected() {
            wire->beginTransmission(I2C_Address);
            return wire->endTransmission() == 0;
        }

        bool writeReg(uint8_t reg, uint8_t value){
            wire->beginTransmission(I2C_Address);
            wire->write(reg);
            wire->write(value);
            return wire->endTransmission() == 0;
        }

        bool hardReset(){
            writeReg(0x06, 0xFF);
            mcp.pinMode(busyPin, INPUT);
            mcp.pinMode(resetPin, OUTPUT);
            mcp.digitalWrite(resetPin, LOW);
            for(int i = 0; i < numReaders; i++){
                mcp.pinMode(i, OUTPUT);
                mcp.digitalWrite(i, HIGH);

            }
            delay(100);
            mcp.digitalWrite(resetPin, HIGH);
            for(int i = 0; i < numReaders; i++){
                nfc[i]->errorCounter = 0;
            }
            //Serial.println("resetting");
            return mcpIsConnected();
        }
};