#include <SPI.h>
#include <mcp_can.h>

#define spiCSpin 10 //Don't use const values when you don't need them, because #define doesn't take up memory and works the same way
MCP_CAN CAN(spiCSpin); //Define which pin CS is connected to, I guess?

uint8_t canDataIn[8]; //8 bytes per standard CAN frame, each value is 0-255.
uint8_t len = 0; //Not exactly sure how/what this does.

uint16_t fullDataIn1; //This is a 2-byte data type, I used uint16_t instead of "word" because uint16_t is always the same size across all platforms
uint16_t fullDataIn2;
uint16_t fullDataIn3;
uint16_t fullDataIn4;

//These are Enerdel specific data variables:
//Floats are pretty big, and the math takes longer.
//What if we used unsigned ints, multiplied everything by 10 or 100, and then only once multiplied by a decimal?
float packCurrent;
float packVoltage;
float packDOD;
float packAvgTemp;

float packMaxCurrent;
float packMaxVoltage;
bool EPOFlag = false;

float packMaxCellVoltage;
int8_t packMaxCellTemp; //uintX_t is unsigned data type (positive only), inX_t is signed (has + and - values)
int8_t packMinCellTemp;

uint32_t canID;
uint32_t prevCanID = 0;



void setup() { //////////////// BEGIN SETUP \\\\\\\\\\\\\\\\

  Serial.begin(1000000); //The faster the baudrate, the less time we take transmitting serial data!
  //Eventually, we won't be using Serial at all, so our program will have more free time to process CAN frames.

  while (CAN_OK != CAN.begin(CAN_500KBPS, MCP_8MHz)) { //If the CAN isn't available, we give an error.
    Serial.println("CAN Initiation failed.");
    delay(100);
  }

  Serial.println("CAN initiated!");

  pinMode(2, INPUT);

  CAN.init_Mask(0, 0, 0x7f0); //Mask number (0 or 1), whether or not it's extended type
  //(0 for standard 11-bit type), which bits to compare filter to. 0x7ff is all of them.
  CAN.init_Mask(1, 0, 0x7ff);

  //Mask 0 applies to filters 0 and 1 and is higher priority. Mask 2 applies to 2-5 and is lower priority.

  //Which filter to use (0-5), Ext: 1 or Standard: 0, specific identifier to filter for.
  //(range of identifiers is determined by setting bits to 0 in the mask so it doesn't care about them.)
  CAN.init_Filt(0, 0, 0x300);// Filter all 0x30x messages
  CAN.init_Filt(2, 0, 0x610);


} //////////////// END SETUP \\\\\\\\\\\\\\\\



void loop() { //////////////// BEGIN LOOP \\\\\\\\\\\\\\\\

  if (!digitalRead(2)) {
    CAN.readMsgBuf(&len, canDataIn); //Get the CAN data and (it's length?)
    canID = CAN.getCanId(); //Get the CAN ID

    switch (canID) {
      case 0x301: //At-time current, voltage, and temp
        packCurrent = (((canDataIn[0] * 256) + canDataIn[1]) * 0.1);
        packVoltage = (((canDataIn[2] * 256) + canDataIn[3]) * 0.1);
        packDOD = (((canDataIn[4] * 256) + canDataIn[5]) * 0.1 );
        packAvgTemp = (((canDataIn[6] * 256) + canDataIn[7]) * 0.1);
        break;
      case 0x303: //Charge limits: current and voltage + contactor status
        packMaxVoltage = (((canDataIn[0] * 256) + canDataIn[1]) * 0.1);
        packMaxCurrent = (((canDataIn[2] * 256) + canDataIn[3]) * 0.1);
        //EPOFlag = bitRead(canDataIn[6], 3);
        break;
      case 0x610: //Min/max cell voltages, plus cell min/max temps
        //packMaxCellVoltage = (((canDataIn[0] * 256) + canDataIn[1]) * 0.00244);
        packMaxCellTemp = canDataIn[4];
      //packMinCellTemp = canDataIn[5];
      default: //What do we do when it's any other message? IGNORE IT HAH
        break;
    }

    Serial.print("A: ");
    Serial.print(packCurrent);
    Serial.print(" - V: ");
    Serial.print(packVoltage);
    Serial.print(" - %: ");
    Serial.print(100 - packDOD);
    //Serial.print("Pack avg temp in F: ");
    //Serial.println(packAvgTemp * 1.8 + 32);
    Serial.print(" - Max Chg. A: ");
    Serial.print(packMaxCurrent);
    Serial.print(" - Max Chg. V: ");
    Serial.print(packMaxVoltage);
    //Serial.print("Max cell voltage: ");
    //Serial.println(packMaxCellVoltage);
    //Serial.print("Pack min cell temp in F: ");
    //Serial.println((packMinCellTemp * 1.8) + 32);
    Serial.print(" - Max temp (F): ");
    Serial.print((packMaxCellTemp * 1.8) + 32);
    Serial.println("");
  }

}//////////////// END OF LOOP \\\\\\\\\\\\\\\\
