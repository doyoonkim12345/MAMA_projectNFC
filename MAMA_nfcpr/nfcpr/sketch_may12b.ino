#include <SPI.h>
#include <MFRC522.h>
#include <MFRC522Hack.h>
#include <Keypad.h>
#include <EEPROM.h>

constexpr uint8_t RST_PIN = 9;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 10;     // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance.
MFRC522Hack mfrc522Hack(&mfrc522);  // Create MFRC522Hack instance.

/* Set your new UID here! */
byte newUid[3][4] = {{0x00, 0x00, 0x00, 0x01},{0x00, 0x00, 0x00, 0x02},{0x00, 0x00, 0x00, 0x03}};

MFRC522::MIFARE_Key key;

bool writeMode = true;

byte selectNum = 0;

byte temp = 0;

const byte ROWS = 4;    // 행(rows) 개수    // 열(columns) 개수
byte keys[ROWS] = {2,1,4,3};

byte rowPins[ROWS] = {4, 5, 6, 7};   // R1, R2, R3, R4 단자가 연결된 아두이노 핀 번호
// C1, C2, C3, C4 단자가 연결된 아두이노 핀 번호

Keypad keypad = Keypad( makeKeymap(keys), rowPins, {}, ROWS, 1 );

byte addr = 0;

void setup() {
  
  Serial.begin(9600);  // Initialize serial communications with the PC
  while (!Serial);     // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();         // Init SPI bus
  mfrc522.PCD_Init();  // Init MFRC522 card
  Serial.println(F("Warning: this example overwrites the UID of your UID changeable card, use with care!"));
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  ReadData();
}

void loop() {
  
  Input();
  if(writeMode){
    //writeMode
    Serial.println("writemode");
    FindAndReadNewCard();
    SetNewUID(selectNum);
  }else{
    //readMode
    Serial.println("readmode");
    Input();
    FindAndReadNewCard();
    GetUID(selectNum);
    StopReadCard();
    writeMode = !writeMode;
    delay(10);
  }
}

void FindAndReadNewCard(){
  
  // Look for new cards , and select one if present
  while( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
    Serial.print(".");
    delay(25);
  }
  
}

void StopReadCard(){
  mfrc522.PICC_HaltA();
}

void SetNewUID(byte upLoadTarget){
  // Set new UID - if completed to Set, then 
  if ( mfrc522Hack.MIFARE_SetUid(newUid[upLoadTarget-1], (byte)4, true) ) {
    Serial.println(F("Wrote new UID to card."));
  }
  //SetNewUID and automatically stop to read things without UID
  StopReadCard();
}

void Print(){
  // Dump the new memory contents
  Serial.println(F("New UID and contents:"));
  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
  //halt 기능포함 되어 있음
}

void GetUID(byte saveTarget){
  Serial.println(saveTarget);
  for (byte i = 0; i < mfrc522.uid.size; ++i) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i],HEX);
    newUid[saveTarget-1][i] = mfrc522.uid.uidByte[i];
  } 
  Serial.println();
  SaveData();
 }

void PrintUIDs(){
  for(byte j = 0; j < 3; ++j){ 
      Serial.print(j+1);
      Serial.print("번째 저장소 : ");
          for (byte i = 0; i < mfrc522.uid.size; ++i) {
            Serial.print(newUid[j][i]);
            Serial.print(" ");
          }
      Serial.println(); 
   }
}

void Input(){
  Serial.println("input");
  while(1){
    temp = keypad.getKey();
    if(temp){
      selectNum = temp;
      Sound(temp);
      if(temp == 4){
        writeMode = !writeMode;
      }else{
         
      }
      break;
    }
  }
}

void SaveData(){
  addr = 0;
  for(byte i = 0; i < 3; i++){
    for(byte j = 0; j < 4; j++){
      EEPROM.write(addr, newUid[i][j]);
      Serial.print(newUid[i][j]);
      Serial.print("(");
      Serial.print(addr);
      Serial.print(")");
      addr = addr + 1;
      if (addr == EEPROM.length()) {
        addr = 0;
      }
    }
    Serial.println();
  }
}

void ReadData(){

  addr = 0;
  for(byte i = 0; i < 3; i++){
    for(byte j = 0; j < 4; j++){
      newUid[i][j] = EEPROM.read(addr);
      Serial.print(newUid[i][j]);
      Serial.print("(");
      Serial.print(addr);
      Serial.print(")");
      addr = addr + 1;
      if (addr == EEPROM.length()) {
        addr = 0;
      }
    }
    Serial.println();
  }
}

void Sound(byte count){
  for(byte i = 0; i < count; i++){
     tone(8,500,50);
     delay(75); 
  }
}

void Reset(){
  for(byte i = 0; i < 12; i++){
    EEPROM.write(i, 0);
  }
  newUid[3][4] = {};
}

