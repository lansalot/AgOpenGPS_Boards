// OK, this is a template class for CANBUS
// Until we get proper CANBUS support in AOG, this will have to do
// You'll need to check your board version and use the correct #define in the FlexCAN_T4 line

#include "CANBUS.h"

// AIO 2.x version board, pins 16 and 17, use CAN3
// AIO 2.x version board, pins 18 and 19, use CAN2
// AIO 4.x version board, pins 16 and 17, use CAN3
// 
// Change  vvvv
FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_256> TeensyCAN;
// Change  ^^^^

const int8_t filterID = 0;
const int8_t byteOffset = 1;
const int8_t ANDValue = 2;
uint32_t lastCANCommand; // Need to delay processing CANBUS messages for a bit, so we don't process multiple messages

// UNCOMMENT ONLY ONE CANINFO LINE BELOW !!!

// CaseNH
//int CANInfo[3] = { 0x18FFB306 , 2, 0x10 }; // Case Puma CVX 160 2015, button behind joystick
//int CANInfo[3] = { 0x18FFB031 , 2, 0x10}; // Case Puma CVX 160 2015, steer button on armrest
//int CANInfo[3] = { 0x14FF7706 , 0, 0x82}; // Case Puma CVX 165 2022, steer button on armrest (note, this is full byte, not bit)
//int CANInfo[3] = { 0x14FF7706 , 0, 0xB2}; // Case Puma CVX 165 2022, steer button on armrest (note, this is full byte, not bit)
//int CANInfo[3] = { 0x14FF7706 , 2, 0xC1}; // Case Puma CVX 165 2022, steer button on armrest (note, this is full byte, not bit)
//int CANInfo[3] = { 0x14FF7706 , 2, 0xC4}; // Case Puma CVX 165 2022, steer button on armrest (note, this is full byte, not bit)
int CANInfo[3] = { 0x18FF1A03 , 2, 0x15 }; // NH, steer button on joystick

// Massey Ferguson
//int CANInfo[3] = { 0x45a , 1, 0x04 }; // MF headland management button
//int CANInfo[3] = { 0xCFF2621 , 3, 0x04}; // MF S-series steering engage
// int CANInfo[3] = { 0x210, 1, 0x20}; // MF 7720 Headland management button

// Codes below here need validated and checked !!!

// Fendt
//int CANInfo[3] = { 0x613 , 1, 0x04}; // Fendt arm rest button
//int CANInfo[3] = { 0xCFFD899 , 3, 0xF6}; // FendtOne engage // not a bit state

// Claas
//int CANInfo[3] = { 0x18EF1CD2 , 2, 0xC4}; // Claas engage

// Valtra
//int CANInfo[3] = { 0x18EF1C32 , 2, 0x01}; // Valtra engage  // code in firmware is unclear for bit states here
//int CANInfo[3] = { 0x18EF1C00 , 2, 0xC4}; // Valtra engage (alternative)

// JCB
//int CANInfo[3] = { 0x18EFAB27 , 2, 0x01}; // code in firmware is unclear for bit states here

// Caterpillar MT
//int CANInfo[3] = { 0x18EF1CF0 , 0, 0x0F}; //code unsure here, could be && [1] == 0x60 as well


// ============================================
// You don't need to change anything below here
// ============================================



bool debugCANBUS = false;
CAN_message_t CANBUSData;

void CAN_Setup() {
	// we're only listening, so no need to claim an address?
	lastCANCommand = millis();
	TeensyCAN.begin();
	TeensyCAN.setBaudRate(250000); // , LISTEN_ONLY);
	TeensyCAN.enableFIFO();
	TeensyCAN.setFIFOFilter(REJECT_ALL);
	delay(1000);
	if (CANInfo[filterID] > 0xffff) {
		Serial.print("Setting extended filter 0x"); Serial.println(CANInfo[filterID], HEX);
		TeensyCAN.setFIFOFilter(0, CANInfo[filterID], EXT);
	}
	else {
		Serial.print("Setting standard filter 0x"); Serial.println(CANInfo[filterID], HEX);
		TeensyCAN.setFIFOFilter(0, CANInfo[filterID], STD);
	}
	if (debugCANBUS) Serial.println("Initialised CANBUS");
	delay(3000);
}

void CANBUS_Receive() {
	if (TeensyCAN.read(CANBUSData)) {
		// if we receive a filtered message, do the bit checking
		// There should be no need to check a CANBUS ID because we're filtering on it - should only receive filtered messages
		if (debugCANBUS) {
			Serial.print("CANBUS ID: ");
			Serial.print(CANBUSData.id, HEX); Serial.print("  ");
			Serial.print("Byte of interest at offset "); Serial.print(CANInfo[byteOffset]); Serial.print("  Value: ");
			Serial.println(CANBUSData.buf[CANInfo[byteOffset]], HEX);
		}
		if ((CANBUSData.buf[CANInfo[byteOffset]] & CANInfo[ANDValue]) && millis() - lastCANCommand > 1000) {
			lastCANCommand = millis();
			if (debugCANBUS) Serial.println("CANBUS bit state: true");
			// not 100% these are correct, lemme know !
			// wasn't correct - thanks to Valentin and BSE for letting me know :D
			if (currentState == 1)
			{
				currentState = 0;
				steerSwitch = 0;
				if (debugCANBUS) Serial.println("Engaging steering");
			}
			else
			{
				if (debugCANBUS) Serial.println("Disengaging steering");
				currentState = 1;
				steerSwitch = 1;
			}
		}
	}
}
