#include <SPI.h>
#include <Ethernet.h>
#include <dht11.h>
#include <SFE_BMP180.h>
#include <Wire.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1,177);
EthernetServer server(80);
EthernetClient client;

dht11 DHT11;
#define DHT11PIN 7 //bmp180已经占用2号端口
#define RelayPIN1 8 //bmp180已经占用2号端口
#define RelayPIN2 9 //bmp180已经占用2号端口
#define RelayPIN3 10 //bmp180已经占用2号端口
#define RelayPIN4 11 //bmp180已经占用2号端口
SFE_BMP180 pressure;// 创建一个气压计对象

void setup() {
  Serial.begin(9600);
  //初始化Ethernet通信
  Ethernet.begin(mac, ip);
  server.begin();
  pressure.begin();
  pinMode(RelayPIN1,OUTPUT);
  pinMode(RelayPIN2,OUTPUT);
  pinMode(RelayPIN3,OUTPUT);
  pinMode(RelayPIN4,OUTPUT);
  digitalWrite(RelayPIN1, HIGH);
  digitalWrite(RelayPIN2, HIGH);
  digitalWrite(RelayPIN3, HIGH);
  digitalWrite(RelayPIN4, HIGH);
}

void loop() {
  // 监听连入的客户端
  client = server.available();
  if (client) {
    Serial.println("new client");
    boolean currentLineIsBlank = false;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        readString += c;
        if (c == '\n') {
          Serial.println(readString);
          //检查收到的信息中是否有”on”，有则开灯
          if(readString.indexOf("?on") >0) {
            digitalWrite(Light, HIGH);
            Serial.println("Led On");
            break;
          }
          //检查收到的信息中是否有”off”，有则关灯
          if(readString.indexOf("?off") >0) {
            digitalWrite(Light, LOW);
            Serial.println("Led Off");
            break;
          }
          //检查收到的信息中是否有”getBrightness”，有则读取光敏模拟值，并返回给浏览器
          if(readString.indexOf("?getBrightness") >0) {
            client.println(analogRead(Sensor));
            break;
          }
          //发送HTML文本
          SendHTML();
          break;
        }       
      }
    }
    delay(1);
    client.stop();
    Serial.println("client disonnected");
    readString="";
  }
}

// 用于输出HTML文本的函数
void SendHTML()
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html><head><meta charset=\"UTF-8\"><title>智能E家</title><script type=\"text/javascript\">");
  client.println("function send2arduino(){var xmlhttp;if (window.XMLHttpRequest)xmlhttp=new XMLHttpRequest();else xmlhttp=new ActiveXObject(\"Microsoft.XMLHTTP\");element=document.getElementById(\"sw1\");if (element.innerHTML.match(\"Turn on\")){element.innerHTML=\"Turn off\"; xmlhttp.open(\"GET\",\"?on1\",true);}else{ element.innerHTML=\"Turn on\";xmlhttp.open(\"GET\",\"?off1\",true); }xmlhttp.send();}");
  //client.println("function getBrightness(){var xmlhttp;if (window.XMLHttpRequest)xmlhttp=new XMLHttpRequest();else xmlhttp=new ActiveXObject(\"Microsoft.XMLHTTP\");xmlhttp.onreadystatechange=function(){if (xmlhttp.readyState==4 && xmlhttp.status==200)document.getElementById(\"ReadSensor\").innerHTML=xmlhttp.responseText;};xmlhttp.open(\"GET\",\"?getBrightness\",true); xmlhttp.send();}window.setInterval(getBrightness,1000);</script>");
  client.println("</script>");
  client.println("</head><body><div align=\"center\"><h1>Arduino Web Server</h1><div>brightness:</div><div id=\"brightness\">"); 
  //client.println(analogRead(Sensor));
  client.println("</div><button id=\"light\" type=\"button\" onclick=\"send2arduino()\">Turn on</button><button type=\"button\" onclick=\"alert('OpenJumper Web Server')\">About</button></div></body></html>");
}

double dewPoint(double celsius, double humidity)
{
        double A0= 373.15/(273.15 + celsius);
        double SUM = -7.90298 * (A0-1);
        SUM += 5.02808 * log10(A0);
        SUM += -1.3816e-7 * (pow(10, (11.344*(1-1/A0)))-1) ;
        SUM += 8.1328e-3 * (pow(10,(-3.49149*(A0-1)))-1) ;
        SUM += log10(1013.246);
        double VP = pow(10, SUM-3) * humidity;
        double T = log(VP/0.61078);   // temp var
        return (241.88 * T) / (17.558-T);
}

double getP()
{
  char status;
  double T,P,p0,a;
  // You must first get a temperature measurement to perform a pressure reading.
  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.
  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);
    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Use '&T' to provide the address of T to the function.
    // Function returns 1 if successful, 0 if failure.
 
    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.
      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);
        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Use '&P' to provide the address of P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.
        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          return P;
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
}
 
double getT()
{
  char status;
  double T,p0;
  status = pressure.startTemperature();
  if (status != 0)
  {
    delay(status);
    status = pressure.getTemperature(T);
    if (status != 0)
    {
      status = pressure.startPressure(3);
      return T;
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
}
