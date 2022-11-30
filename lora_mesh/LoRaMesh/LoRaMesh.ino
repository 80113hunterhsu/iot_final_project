#include <EEPROM.h>
#include <math.h>
#include <RHRouter.h>
#include <RHMesh.h>
#include <RH_RF95.h>
#define RH_HAVE_SERIAL
#define LED 8
#define N_NODES 30
//#define N_NODES 2 //test

int rounds = 0;
int rx_done = 0;

int data_set_status = 0;
int nodeid = -1; //unique number
int groupid = -1;  //group name set by group organizer
int memberNum = -1;  //count of group members

int nodeId;
int groupId;

uint8_t routes[N_NODES]; // full routing table for mesh
int16_t rssi[N_NODES]; // signal strength info
int offline[N_NODES];

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
      digitalWrite(LED, HIGH);
      delay(100);
      digitalWrite(LED, LOW);
      delay(100);
    }
  }

  //  Serial.println("done");

  return;
}

void setBasicData(int *p_nodeId, int *p_groupId, int *p_memberNum) {
  int nodeid = *p_nodeId; //unique number
  int groupid = *p_groupId;  //group name set by group organizer
  int memberNum = *p_memberNum;  //count of group members
  while (!data_set_status) {
    // Serial.println(F("toPython-->Started"));
    String str = "";
    while (!Serial.available());
    str = Serial.readString();
    if (str) {
      // Serial.println(F("get str"));
    }
    if (str.indexOf("nodeId-->") >= 0) {
      Serial.print(F("toPython-->nodeId-->"));
      str.replace("nodeId-->", "");
      nodeid = str.toInt();
      EEPROM.write(0, nodeid);
      Serial.println(String(nodeid));
    }
    if (str.indexOf("groupId-->") >= 0) {
      Serial.print(F("toPython-->groupId-->"));
      str.replace("groupId-->", "");
      groupid = str.toInt();
      EEPROM.write(1, groupid);
      Serial.println(String(groupid));
    }
    if (str.indexOf("memberNum-->") >= 0) {
      Serial.print(F("toPython-->memberNum-->"));
      str.replace("memberNum-->", "");
      memberNum = str.toInt();
      EEPROM.write(2, memberNum);
      Serial.println(String(memberNum));
    }
    if (nodeid != -1 && groupid != -1 && memberNum != -1) {
      data_set_status = 1;
      // Serial.println(F("toPython-->all data done. "));
      // break;
    }
    else {
      // Serial.println(F("toPython-->redo "));
      delay(1000);
    }
  }
}

void printFreeMem() {
  Serial.print(F("mem = "));
  Serial.println(freeMem());

  return;
}

void setup() {
  randomSeed(analogRead(0));
  pinMode(LED, OUTPUT);
  LEDblink(0);
  Serial.begin(115200);
  Serial.setTimeout(1);
  while (!Serial) ; // Wait for serial port to be available

  // setBasicData(&nodeid, &groupid, &memberNum);
  nodeId = EEPROM.read(0);
  groupId = EEPROM.read(1);
  memberNum = EEPROM.read(2);
//  Serial.println(nodeId);
//  Serial.println(groupId);
//  Serial.println(N_NODES);

  // Serial.println("");
  // for (int i = 0; i < EEPROM.length(); i++) {
    // Serial.print(i);
    // Serial.print(": ");
    // Serial.println(EEPROM.read(i));
  // }
  // Serial.println("");
  
  if (nodeId > 10) {
    Serial.print(F("EEPROM nodeId invalid: "));
    Serial.println(nodeId);
    nodeId = 1;
  }
  Serial.print(F("initializing node "));
  Serial.println(String(nodeId));
  Serial.print(F("group id: "));
  Serial.println(String(groupId));
  Serial.print(F("member count: "));
  Serial.println(String(memberNum));
  
  manager = new RHMesh(rf95, nodeId);
  Serial.println(F("here"));

  if (!manager->init()) {
    Serial.println(F("init failed"));
  } else {
    Serial.println("done");
  }
  rf95.setTxPower(23, false);
  rf95.setFrequency(433.0);
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

  Serial.println("RF95 ready");

  for (uint8_t n = 1; n <= N_NODES; n++) {
    routes[n - 1] = 0;
    rssi[n - 1] = 0;
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
  double ra = abs(rssi);
  double ka = (ra - a) / (10 * n);
  double distance = pow(10, ka);

  return distance;
}

void updateRoutingTable() {
  for (uint8_t n = 1; n <= N_NODES; n++) {
    RHRouter::RoutingTableEntry *route = manager->getRouteTo(n);
    if (n == nodeId) {
      routes[n - 1] = 255; // self
    } else {
      routes[n - 1] = route->next_hop;
      if (routes[n - 1] == 0) {
        // if we have no route to the node, reset the received signal strength
        rssi[n - 1] = 0;
      }
    }
  }
}

// Create a JSON string with the routing info to each node
void getRouteInfoString(char *p, size_t len) {
  p[0] = '\0';
  strcat(p, "[");
  for (uint8_t n = 1; n <= N_NODES; n++) {
    strcat(p, "{\"n\":");
    sprintf(p + strlen(p), "%d", routes[n - 1]);
    strcat(p, ",");
    strcat(p, "\"r\":");
    sprintf(p + strlen(p), "%d", rssi[n - 1]);
    strcat(p, "}");
    if (n < N_NODES) {
      strcat(p, ",");
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

//  String str = "";
//  Serial.println(F("Here"));
//  while (!Serial.available());
//  str = Serial.readString();
//  if (str.indexOf("BasicData-->") >= 0) {
//    Serial.println(F("StartDataWrite-->"));
//    setBasicData(&nodeid, &groupid, &memberNum);
//  }
  
  Serial.println("======= " + String(rounds) + " =======");

  rx_done = 0;

  for (int i = 0; i < memberNum; i++) {
    if (offline[i] > (memberNum+1)) {
      Serial.print(F("node "));
      Serial.print(i + 1);
      Serial.println(F(" is offline! "));

      LEDblink(i + 1);
    }
  }

  for (uint8_t i = 0; i < N_NODES; i++) {
    int n = (nodeId + i) % 4 + 1;
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
    uint8_t error = manager->sendtoWait((uint8_t *)buf, strlen(buf), n);
    // Serial.println("HERE");
    if (error != RH_ROUTER_ERROR_NONE) {
      Serial.println();
      Serial.print(F(" ! "));
      Serial.println(getErrorString(error));

      offline[n - i]++;

      // LEDblink(error);
    } else {
      Serial.println(F(" OK"));
      rx_done++;

      offline[n - 1] = 0;

      // LEDblink(0);

      // we received an acknowledgement from the next hop for the node we tried to send to.
      RHRouter::RoutingTableEntry *route = manager->getRouteTo(n);
      if (route->next_hop != 0) {
        rssi[route->next_hop - 1] = rf95.lastRssi();
        Serial.print(F("<< "));
        Serial.print(rf95.lastRssi());
        Serial.print(F(", "));
        Serial.print(rssitoDistance(rf95.lastRssi()));
        Serial.println(F("m >>"));
      }
    }
    //    if (nodeId == 1) printNodeInfo(nodeId, buf); // debugging

    // listen for incoming messages. Wait a random amount of time before we transmit
    // again to the next node
    unsigned long nextTransmit = millis() + random(3000, 5000);
    // unsigned long nextTransmit = millis() + 2000;
    while (nextTransmit > millis()) {
      int waitTime = nextTransmit - millis();
      uint8_t len = sizeof(buf);
      uint8_t from;
      if (manager->recvfromAckTimeout((uint8_t *)buf, &len, waitTime, &from)) {
        buf[len] = '\0'; // null terminate string
        Serial.print(from);
        Serial.print(F("->"));
        Serial.print(F(" :"));
        Serial.println(buf);
        //        if (nodeId == 1) printNodeInfo(from, buf); // debugging
        // we received data from node 'from', but it may have actually come from an intermediate node
        RHRouter::RoutingTableEntry *route = manager->getRouteTo(from);
        if (route->next_hop != 0) {
          rssi[route->next_hop - 1] = rf95.lastRssi();
          Serial.print(F("\t<< "));
          Serial.print(rf95.lastRssi());
          Serial.print(F(", "));
          Serial.print(rssitoDistance(rf95.lastRssi()));
          Serial.println(F("m >>"));
        }
      }
    }
  }
  Serial.print(F("rx_done: "));
  Serial.print(String(rx_done));
  Serial.println(F("/3"));
  printFreeMem();
}
