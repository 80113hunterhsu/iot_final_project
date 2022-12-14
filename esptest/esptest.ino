
#include "esp_2_3_line.h"
//#include <LiquidCrystal_I2C.h>

const char *SID = "mc2";
const char *PWD = "210210210";

//LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

int relayPin = 12;
int buttonPin1 = 7;
int buttonPin2 = 6;
int buttonPin3 = 5;
int buttonPin4 = 4;
int buttonPin_ppl = A2;
int buttonPin = A3;
int buttonState1 = 0;
int buttonState2 = 0;
int buttonState3 = 0;
int buttonState4 = 0;
int buttonState_ppl = 0;
int buttonState = 0;
int maxGas = 0;
int maxLiquid = 0;
bool data_flag = false;
bool send_flag = false;

void setup() 
{
  Serial.begin(9600);
  Serial.println(F("Start"));

  bool esp01;
  esp01=esp01_9600(SID,PWD);
  if(esp01)
    Serial.println(F("Connecting OK!"));
//  pinMode(buttonPin,INPUT);
//  pinMode(relayPin, OUTPUT);  
  
//  lcd.begin(16, 2);
//  lcd.setCursor(0, 0); // 設定游標位置在第一行行首
//  lcd.print(F("Start"));
//  delay(1000);
//  lcd.clear(); //顯示清除
}

void loop() 
{
  Serial.print(F("Humidity: "));
  Serial.print(F(" %\t"));
  Serial.print(F("Temperature: "));
  Serial.println(F(" *C\t "));

//  lcd.setCursor(0, 0); // 設定游標位置在第一行行首
//  lcd.print(F("Humi: "));
//  lcd.print(h);
//  lcd.setCursor(0, 1); // 設定游標位置在第二行行首
//  lcd.print(F("Temp: "));
//  lcd.print(t);
//  delay(2000);
//  lcd.clear(); //顯示清除

//  if(h>70)
//  {
//    digitalWrite(relayPin, HIGH);
//  }
//  else
//  {
//    digitalWrite(relayPin, LOW);
//  }

//  buttonState_ppl = digitalRead(buttonPin_ppl);
//  buttonState1 = digitalRead(buttonPin1);
//  buttonState2 = digitalRead(buttonPin2);
//  buttonState3 = digitalRead(buttonPin3);
//  buttonState4 = digitalRead(buttonPin4);

//  if(buttonState_ppl == HIGH)
//  {
//    if(maxGas<sensor_volt)
//    {
//      maxGas = sensor_volt;
//    }
//    if(maxLiquid<sensors.getTempCByIndex(0))
//    {
//      maxLiquid = sensors.getTempCByIndex(0);
//    }
//    if(buttonState1 == HIGH)
//      check_ppl(maxGas, maxLiquid, 1);
//    if(buttonState2 == HIGH)
//      check_ppl(maxGas, maxLiquid, 2);
//    if(buttonState3 == HIGH)
//      check_ppl(maxGas, maxLiquid, 3);
//    if(buttonState4 == HIGH)
//      check_ppl(maxGas, maxLiquid, 4);
//  }


//  
//  buttonState = digitalRead(buttonPin);
//  if (buttonState == HIGH)
//  {
//    Serial.print(F("Tissue Empty!"));
//    StepMoto_THH('B', 8, 9, 10, 11, 1);
//
//    lcd.setCursor(0, 0); // 設定游標位置在第一行行首
//    lcd.print(F("Tissue Empty!"));
//    delay(2000);
//    lcd.clear(); //顯示清除
//
//    send_flag=send_open("taso2309.ygto.com","Paper",1,"Paper1",1,"Paper2",1);
//    send_flag=Send_Web_Line("taso2309.ygto.com","Paper","沒衛生紙了！");
//    if (send_flag) Serial.println(F("Send Data Complete !!"));    
//  } 
}

//bool check_ppl(int gas, int liquid, int num)
//{
//    lcd.setCursor(0, 0); // 設定游標位置在第一行行首
//    lcd.print(F("Gas: "));
//    lcd.print(gas);
//    lcd.setCursor(0, 1); // 設定游標位置在第二行行首
//    lcd.print(F("Liquid : "));
//    lcd.print(liquid);
//    delay(2000);
//    lcd.clear(); //顯示清除
//    send_flag=send_open("taso2309.ygto.com","People",num,"Liquid",maxLiquid,"Gas",maxGas);
//    Serial.println(F("Data Send"));
//    maxGas = 0;
//    maxLiquid = 0;
//}
