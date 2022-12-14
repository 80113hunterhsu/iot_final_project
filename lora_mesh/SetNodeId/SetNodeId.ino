
#include <EEPROM.h>

// change this to be the ID of your node in the mesh network
uint8_t nodeId = 3;
uint8_t groupId = 2;
uint8_t memberNum = 3;

void setup() {
  Serial.begin(115200);
  while (!Serial) ; // Wait for serial port to be available

  Serial.println(F("setting nodeId..."));
  EEPROM.write(0, nodeId);
  Serial.print(F("set nodeId = "));
  Serial.println(nodeId);
  Serial.println(F("setting groupId..."));
  EEPROM.write(1, groupId);
  Serial.print(F("set groupId = "));
  Serial.println(groupId);
  Serial.println(F("setting memberNum..."));
  EEPROM.write(2, memberNum);
  Serial.print(F("set memberNum = "));
  Serial.println(memberNum);

  uint8_t readVal_nodeId = EEPROM.read(0);
  uint8_t readVal_groupId = EEPROM.read(1);
  uint8_t readVal_memberNum = EEPROM.read(2);

  Serial.print(F("read nodeId: "));
  Serial.println(readVal_nodeId);
  Serial.print(F("read groupId: "));
  Serial.println(readVal_groupId);
  Serial.print(F("read memberNum: "));
  Serial.println(readVal_memberNum);

  if (nodeId != readVal_nodeId || groupId != readVal_groupId || memberNum != readVal_memberNum) {
    Serial.println(F("*** FAIL ***"));
  } else {
    Serial.println(F("SUCCESS"));
  }
}

void loop() {

}
