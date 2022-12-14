#include <EEPROM.h>
#include <RHRouter.h>
#include <RHMesh.h>
#include <RH_RF95.h>
//#include <RHGenericDriver.h>

#define RH_HAVE_SERIAL
#define LED 8
#define BTN 7
#define N_NODES 4

uint8_t rounds = 0;
uint8_t rx_done = 0;

bool data_set_status = 0;
//bool start_data_setting = 0;
uint8_t nodeId = -1; //unique number
uint8_t groupId = -1;  //group name set by group organizer
uint8_t memberNum = -1;  //count of group members

uint8_t routes[N_NODES]; // full routing table for mesh
int16_t rssi[N_NODES]; // signal strength info
uint8_t groups[N_NODES]; // group info
uint8_t offline[N_NODES]; // offline info

// Singleton instance of the radio driver
RH_RF95 rf95;

// Class to manage message delivery and receipt, using the driver declared above
RHMesh *manager;

// message buffer
char buf[RH_MESH_MAX_MESSAGE_LEN];

int freeMem() {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void LEDblink(int code) {
  if (code == 0) {
    digitalWrite(LED, HIGH);
    delay(1000);
    digitalWrite(LED, LOW);
  }
  else {
    for (int i = 0; i < code; i++) {
      digitalWrite(LED, HIGH);delay(100);
      digitalWrite(LED, LOW);delay(100);
    }
  }

  return;
}

void setBasicData(uint8_t *p_nodeId, uint8_t *p_groupId, uint8_t *p_memberNum) {
//  uint8_t nodeid = *p_nodeId; //unique number
//  uint8_t groupid = *p_groupId;  //group name set by group organizer
//  uint8_t membernum = *p_memberNum;  //count of group members
  while (!data_set_status) {
    LEDblink(10);
    while (!Serial.available());
    String serialIn = Serial.readString();
    Serial.println(serialIn);
    if (serialIn.indexOf("nodeId-->") >= 0) {
      Serial.print(F("toPython-->nodeId-->"));
      serialIn.replace("nodeId-->", "");
      *p_nodeId = serialIn.toInt();
      EEPROM.write(0, *p_nodeId);
      Serial.println(String(*p_nodeId));
    }
    if (serialIn.indexOf("groupId-->") >= 0) {
      Serial.print(F("toPython-->groupId-->"));
      serialIn.replace("groupId-->", "");
      *p_groupId = serialIn.toInt();
      EEPROM.write(1, *p_groupId);
      Serial.println(String(*p_groupId));
    }
    if (serialIn.indexOf("memberNum-->") >= 0) {
      Serial.print(F("toPython-->memberNum-->"));
      serialIn.replace("memberNum-->", "");
      *p_memberNum = serialIn.toInt();
      EEPROM.write(2, *p_memberNum);
      Serial.println(String(*p_memberNum));
    }
    if (*p_nodeId != -1 && *p_groupId != -1 && *p_memberNum != -1) {
      data_set_status = 1;
      Serial.print(F("nodeId: "));
      Serial.println(*p_nodeId);
      Serial.print(F("groupId: "));
      Serial.println(*p_groupId);
      Serial.print(F("memberNum: "));
      Serial.println(*p_memberNum);
    }
    else {
      delay(200);
    }
    //serialIn = "";
  }
}

void printFreeMem() {
  Serial.print(F("mem = "));
  Serial.println(freeMem());

  return;
}

void setup() {
  randomSeed(analogRead(0));
  pinMode(BTN, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  LEDblink(0);
  Serial.begin(115200);
  Serial.setTimeout(1);
  while (!Serial) ; // Wait for serial port to be available

  unsigned long waitforbtn = millis() + 3000;
  while (waitforbtn > millis()) {
    Serial.print(F("Waiting..."));
    Serial.println((waitforbtn - millis()) / 1000);
    Serial.println(!digitalRead(BTN));
    if (!digitalRead(BTN)) {
      digitalWrite(LED, HIGH);
      delay(5000);
      digitalWrite(LED, LOW);
      Serial.println(F("StartDataWrite-->"));
      setBasicData(&nodeId, &groupId, &memberNum);
    }
    else {
      nodeId = EEPROM.read(0);
      groupId = EEPROM.read(1);
      memberNum = EEPROM.read(2);
    }
    delay(500);
  }
  
  //setBasicData(&nodeid, &groupid, &memberNum);
  //Serial.println(F("test"));
  Serial.println(nodeId);
  Serial.println(groupId);
  Serial.println(memberNum);
  
  if (nodeId > 10) {
    Serial.print(F("EEPROM nodeId invalid: "));
    Serial.println(nodeId);
    nodeId = 1;
  }
  Serial.print(F("initializing node "));
  Serial.print(nodeId);
  Serial.print(F(" at group "));
  Serial.println(groupId);

  manager = new RHMesh(rf95, nodeId);

  if (!manager->init()) {
    Serial.println(F("init failed"));
  } else {
    Serial.println(F("init done"));
  }
  rf95.setTxPower(23, false);
  rf95.setFrequency(433.0);
  rf95.setHeaderTo(groupId);
  rf95.setCADTimeout(500);

  // Possible configurations:
  // Bw125Cr45Sf128 (the chip default)
  // Bw500Cr45Sf128
  // Bw31_25Cr48Sf512
  // Bw125Cr48Sf4096

  // long range configuration requires for on-air time
  boolean longRange = false;
  if (longRange) {
    RH_RF95::ModemConfig modem_config = {
      0x78, // Reg 0x1D: BW=125kHz, Coding=4/8, Header=explicit
      0xC4, // Reg 0x1E: Spread=4096chips/symbol, CRC=enable
      0x08  // Reg 0x26: LowDataRate=On, Agc=Off.  0x0C is LowDataRate=ON, ACG=ON
    };
    rf95.setModemRegisters(&modem_config);
    if (!rf95.setModemConfig(RH_RF95::Bw125Cr48Sf4096)) {
      Serial.println(F("set config failed"));
    }
  }

  Serial.println(F("RF95 ready"));

  for (uint8_t n = 1; n <= memberNum; n++) {
    routes[n - 1] = 0;
    rssi[n - 1] = 0;
    groups[n - 1] = 0;
    offline[n - 1] = 0;
  }
  printFreeMem();
}

const __FlashStringHelper* getErrorString(uint8_t error) {
  switch (error) {
    case 1: return F("invalid length");
      break;
    case 2: return F("no route");
      break;
    case 3: return F("timeout");
      break;
    case 4: return F("no reply");
      break;
    case 5: return F("unable to deliver");
      break;
  }
  return F("unknown");
}

double rssitoDistance(double rssi, int a = 67, double n = 1.192913874) {
  return pow(10, ((abs(rssi) - a) / (10 * n)));
}

void updateRoutingTable() {
  for (uint8_t n = 1; n <= memberNum; n++) {
    RHRouter::RoutingTableEntry *route = manager->getRouteTo(n);
    if (n == nodeId) {
      routes[n - 1] = 255; // self
      groups[n - 1] = groupId;
    } else {
      routes[n - 1] = route->next_hop;
      if (routes[n - 1] == 0) {
        // if we have no route to the node, reset the received signal strength
        //rssi[n - 1] = 0;
      }
    }
  }
}

int getRecvGroupId() {
  uint8_t fgroupId = (uint8_t)(buf[strlen(buf) - 2] - '0');
//  Serial.println(fgroupId);
  return fgroupId;
}

// Create a JSON string with the routing info to each node
void getRouteInfoString(char *p, size_t len) {
  p[0] = '\0';
  strcat(p, "[");
  for (uint8_t n = 1; n <= memberNum; n++) {
    strcat(p, "{\"n\":");
    sprintf(p + strlen(p), "%d", routes[n - 1]);
    strcat(p, ",");
    strcat(p, "\"g\":");
    sprintf(p + strlen(p), "%d", groups[n - 1]);
    strcat(p, ",");
    strcat(p, "\"r\":");
    sprintf(p + strlen(p), "%d", rssi[n - 1]);
    strcat(p, "}");
    if (n < memberNum) {
      strcat(p, ",");
    }
    else {
      strcat(p, ",gId=");
      sprintf(p + strlen(p), "%d", groupId);
    }
  }
  strcat(p, "]");
}

void printNodeInfo(uint8_t node, char *s) {
  Serial.print(F("node: "));
  Serial.print(F("{"));
  Serial.print(F("\""));
  Serial.print(node);
  Serial.print(F("\""));
  Serial.print(F(": "));
  Serial.print(s);
  Serial.println(F("}"));
}

void loop() {
  rounds++;
  
  Serial.print(F("======= "));
  Serial.print(rounds);
  Serial.println(F(" ======="));

  rx_done = 0;

  for (int i = 0; i < memberNum; i++) {
//    Serial.print(i);
//    Serial.print(F(": "));
//    Serial.println(offline[i]);
    if (offline[i] > 5) {
      Serial.print(F("node "));
      Serial.print(i + 1);
      Serial.print(F(" offline: "));
      Serial.println(rssitoDistance(rssi[i]));

      LEDblink(i + 1);
    }
  }

  for (uint8_t i = 0; i < memberNum; i++) {
    int8_t n = (nodeId + i) % 4 + 1;
    // int n = i + 1;
    if (n == nodeId) {
      continue; // self
    }

    updateRoutingTable();
    getRouteInfoString(buf, RH_MESH_MAX_MESSAGE_LEN);

    Serial.print(F("->"));
    Serial.print(n);
    Serial.print(F(" :"));
    Serial.print(buf);

    // send an acknowledged message to the target node
    manager -> setTimeout(5000);
    uint8_t error = manager->sendtoWait((uint8_t *)buf, strlen(buf), n, groupId);
    // Serial.println("HERE");
    if (error != RH_ROUTER_ERROR_NONE) {
      Serial.println();
      Serial.print(F(" ! "));
      Serial.println(getErrorString(error));

      offline[n - 1]++;

      // LEDblink(error);
    } else {
      Serial.println(F(" OK"));
      rx_done++;

      offline[n - 1] = 0;

      // LEDblink(0);

      // we received an acknowledgement from the next hop for the node we tried to send to.
      RHRouter::RoutingTableEntry *route = manager->getRouteTo(n);
      if (route->next_hop != 0) {
        if (groups[route->next_hop - 1] == groupId) {
          rssi[route->next_hop - 1] = rf95.lastRssi();
        }
        Serial.print(F("<< "));
        Serial.print(rf95.lastRssi());
        Serial.print(F(", "));
        Serial.print(rssitoDistance(rf95.lastRssi()));
        Serial.println(F("m >>"));
      }
    }
    if (nodeId == 1) printNodeInfo(nodeId, buf); // debugging

    // listen for incoming messages. Wait a random amount of time before we transmit
    // again to the next node
    unsigned long nextTransmit = millis() + random(3000, 5000);
    // unsigned long nextTransmit = millis() + 2000;
    while (nextTransmit > millis()) {
      int waitTime = nextTransmit - millis();
      uint8_t len = sizeof(buf);
      uint8_t from;
      //uint8_t gid = manager->headerFrom();
      //Serial.println(gid);
      uint8_t gid;
      if (manager->recvfromAckTimeout((uint8_t *)buf, &len, waitTime, &from, NULL, NULL, &gid)) {
        Serial.print(F("flags: "));
        Serial.println(gid);
        if (gid == groupId) {
          buf[len] = '\0'; // null terminate string
          Serial.print(from);
          Serial.print(F("->"));
          Serial.print(F(" :"));
          Serial.println(buf);
          if (nodeId == 1) printNodeInfo(from, buf); // debugging
          
          // we received data from node 'from', but it may have actually come from an intermediate node
          RHRouter::RoutingTableEntry *route = manager->getRouteTo(from);
          if (route->next_hop != 0) {
  //          Serial.println(getRecvGroupId());
            groups[route->next_hop - 1] = getRecvGroupId();
  //          Serial.println(groups[route->next_hop - 1]);
            if (groups[route->next_hop - 1] == groupId) {
              rssi[route->next_hop - 1] = rf95.lastRssi();
            }
            Serial.print(F("\t<< "));
            Serial.print(rf95.lastRssi());
            Serial.print(F(", "));
            Serial.print(rssitoDistance(rf95.lastRssi()));
            Serial.println(F("m >>"));
          }
        }
        else {
          if (manager->deleteRouteTo(from)) {
            Serial.print(F("node "));
            Serial.print(from);
            Serial.println(F(" was not in same group."));
          }
        }
      }
    }
  }
  Serial.print(F("rx_done: "));
  Serial.print(rx_done);
  Serial.println(F("/3"));
//  printFreeMem();
}
