
#include <EEPROM.h>

// change this to be the ID of your node in the mesh network
int nodeId = 4;
int groupId = 2;
int memberNum = 3;

void setup() {
  Serial.begin(115200);
  while (!Serial) ; // Wait for serial port to be available

  Serial.println("setting nodeId...");

  EEPROM.write(0, nodeId);
  EEPROM.write(1, groupId);
  EEPROM.write(2, memberNum);
  Serial.print(F("set nodeId = "));
  Serial.println(nodeId);
  Serial.print(F("set groupId = "));
  Serial.println(groupId);
  Serial.print(F("set memberNum = "));
  Serial.println(memberNum);

  int read_nodeId = EEPROM.read(0);
  int read_groupId = EEPROM.read(1);
  int read_memberNum = EEPROM.read(2);

  Serial.print(F("read nodeId: "));
  Serial.println(read_nodeId);
  Serial.print(F("read groupId: "));
  Serial.println(read_groupId);
  Serial.print(F("read memberNum: "));
  Serial.println(read_memberNum);

  if (nodeId != read_nodeId) {
    Serial.println(F("*** nodeId FAIL ***"));
  } else {
    Serial.println(F("nodeId SUCCESS"));
  }
  if (groupId != read_groupId) {
    Serial.println(F("*** groupId FAIL ***"));
  } else {
    Serial.println(F("groupId SUCCESS"));
  }
  if (memberNum != read_memberNum) {
    Serial.println(F("*** memberNum FAIL ***"));
  } else {
    Serial.println(F("memberNum SUCCESS"));
  }
}

void loop() {

}
