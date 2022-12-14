
#include "WiFiEsp.h"
#include "SoftwareSerial.h"

bool esp01_9600(const char *,const char *);
bool Send_Web_line(const char *,String,String);
bool send_open(const char *, String ,int, String , int, String, int);

SoftwareSerial ESP01(3, 4);
WiFiEspServer server(80);
RingBuffer buf(8);

bool esp01_9600(const char *my_id,const char *my_pw)
{
 int wifi_flag=0;
 int status = WL_IDLE_STATUS;
 ESP01.begin(9600);
 WiFi.init(&ESP01);
 if (WiFi.status() == WL_NO_SHIELD)
 {
    wifi_flag=1;    
 }

 if (wifi_flag==0)
 {
  // attempt to connect to WiFi network
  while (status != WL_CONNECTED)
  {
   status = WiFi.begin(my_id, my_pw);
   if (status ==WL_CONNECTED)
   {
    // Connect to WPA/WPA2 network
    LOGINFO1(F("SSID: "), WiFi.SSID());
    LOGINFO1(F("IP Address: "), WiFi.localIP());
    // start the web server on port 80
    server.begin();
    return true;
   }
  }   
 }
 return false;
}

bool Send_Web_Line(const char *url, String item,String val)
{
 byte i=0;
 WiFiEspClient client;
 String data = "GET /aiot/line.php?" + item + "=" + val + " HTTP/1.1";  

 if (client.connect(url,80))
 { 
  Serial.println(data);
  // This will send the request to the server
  client.print( data + " HTTP/1.1\r\n" +
               "Host: " + url + "\r\n" + 
               "Connection: close\r\n\r\n");
  while(client.connected())
  {
    if(client.available())
    {
      return true;
      //String line = client.readStringUntil('\r');
      //Serial.println(line);
     // Serial.println("Send Data Complete !!");      
    } else
    {
      // No data yet, wait a bit
      delay(100);
      i++;
    };
    if (i>5) return false; 
    return true;
  }
 }
 return true;
}

bool send_open(const char *D_host, String item1,int val1, String item2,int val2, String item3,int val3)
{
 bool connect_flag=false;
 byte i=0;  
 //const char *D_host = "192.168.43.36"; 
  Serial.print(F("Connecting to "));
  Serial.println(D_host);
  WiFiEspClient client;
  const int httpPort = 80;
  connect_flag=client.connect(D_host, httpPort); 
  while (!connect_flag)
  {
   Serial.println(F("Connection failed"));
   delay(200);
   connect_flag=client.connect(D_host, httpPort);
   if (i>5) return false;
   i++;
  }
  
  // Create a URI for the request
  String data = "/aiot/AddData.php?"+ item1 + "=" + val1 + "&" + item2 + "=" + val2 + "&" + item3 + "=" + val3;
  /*
  Serial.print("Requesting URL: ");
  Serial.println(data);
  */
  
  // This will send the request to the server
  client.print(String("GET ") + data + " HTTP/1.1\r\n" +
               "Host: " + D_host + "\r\n" + 
               "Connection: close\r\n\r\n");
  
  //Serial.println();
  
  //Serial.println("closing connection");
  
  client.stop();
  return true;
}  
