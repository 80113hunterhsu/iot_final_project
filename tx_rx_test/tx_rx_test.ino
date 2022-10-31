#include <SPI.h>
#include <LoRa.h>
int counter = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
//  Serial.println("LoRa Sender");
  if (!LoRa.begin(433E6)) 
  {
    Serial.println("Starting LoRa failed!");
    while(1);
  }
}
int runtime_count = 0;
void loop() {
  Serial.print("Listening");
  while (runtime_count < 5) {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      counter = rx() + 1;
      break;
    }
    runtime_count+=1;
    Serial.print(".");
    delay(1000);
  }
  runtime_count = 0;
  Serial.println("");
  while (runtime_count < 5) {
    counter = tx();
    runtime_count+=1;
    delay(1000);
  }
  runtime_count = 0;
}

int tx() {
  String msg = String(counter);
  
  Serial.print("Sending message: \"");
  Serial.println(msg + "\"");
  
  LoRa.beginPacket();
  LoRa.print(msg);
  LoRa.endPacket();

  return counter;
}

int rx() {
  Serial.print("Received packet '");
  String str = "";
  while (LoRa.available()) {
    Serial.print((char)LoRa.read());
    str=str+((char)LoRa.read()); 
  }
  Serial.print("' with RSSI ");
  Serial.println(LoRa.packetRssi());

  return str.toInt();
}
