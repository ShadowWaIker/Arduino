#include <SPI.h>
#include <Ethernet.h>
#include <dht11.h>
#include <SFE_BMP180.h>
#include <Wire.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xBF };
  //服务端IP地址
IPAddress ip(192,168,1,178);
// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);
//设置继电器端口
#define RELAY1 8
#define RELAY2 9
#define RELAY3 11
#define RELAY4 12
//设置传感器端口
dht11 DHT11;
#define DHT11PIN 7 //bmp180已经占用2号端口,13号端口无法使用
SFE_BMP180 pressure;// 创建一个气压计对象

void setup() {
 // Open serial communications and wait for port to open:
  /*
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  */
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  /*
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  */
  //初始化BMP180传感器
  pressure.begin();
  //设置端口为输出
  pinMode(RELAY1,OUTPUT);
  pinMode(RELAY2,OUTPUT);
  pinMode(RELAY3,OUTPUT);
  pinMode(RELAY4,OUTPUT);
  //设置继电器默认状态为关闭
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);
  digitalWrite(RELAY3, HIGH);
  digitalWrite(RELAY4, HIGH);
}

void loop() {
  // listen for incoming clients
  EthernetClient client = server.available(); 
  if (client) {
    //向串口输出信息(新客户端上线)
    /*
    Serial.println("new client");
    */
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // 将收到的客户端信息发送到串口(调试用)
        //Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank){
          // it is after the double cr-lf that the variables are
          // read another line!
          //String POST = "";  //定义POST变量
          while(client.available())
          {           
            c = client.read();
            //POST += c;   //保存POST数据
          }
          //Serial.print(POST); // 将收到的post内容发送到串口(调试用)
          
         if(String(c)=="1"){  //执行POST请求对应的操作
                   SetRelay(RELAY1);
                   }else if(String(c)=="2"){
                    SetRelay(RELAY2);
                   }else if(String(c)=="3"){
                    SetRelay(RELAY3);
                   }else if(String(c)=="4"){
                    SetRelay(RELAY4);
          }
        
          //打印输出网页代码
         client.print("<html><head><meta charset=\"UTF-8\"><title>智能e家</title>"
                        "<style>*{font-family:Arial;}body{background-color:#f2f2f2;}h1{color:#222;}</style>"
                        "<body><div style='text-align:center;'>");

         //获取DHT11传感器读数
         DHT11.read(DHT11PIN);
        /*
         client.print("传感器状态: ");
         int chk = DHT11.read(DHT11PIN);
         switch (chk)
         {
            case DHTLIB_OK: 
                 client.println("OK"); 
                 break;
            case DHTLIB_ERROR_CHECKSUM: 
                 client.println("Checksum error"); 
                 break;
            case DHTLIB_ERROR_TIMEOUT: 
                 client.println("Time out error"); 
                 break;
            default: 
                 client.println("Unknown error"); 
                 break;
          }
          */
         client.print("当前室内环境<br/>相对湿度:");
         client.print((float)DHT11.humidity, 2);
         client.print("%<br/>室内温度:");
         client.print(((float)DHT11.temperature+getT())/2, 2);
         client.print("℃<br/>露点温度:");
         client.print(dewPoint(DHT11.temperature, DHT11.humidity));
         client.print("℃<br/>海拔高度:");
         double a,p,t;
         p = getP();// 获得一个气压值
         a = pressure.altitude(p,1005);//获得基于基准气压的高度，第二个参数是基准气压
         client.print(a,1);
         client.print("米<br/>大气压强:");
         client.print(p,1);
         client.print("hPa<br/><br/>");
         client.println("<div>点击修改开关状态"
	                "<table width='30%'  align='center'>"
                        "<tr><td>"
			"<table width='100%' height='20'  cellspacing='1'><tbody><tr>");
         PrintRelay(client,RELAY1,1); //参数为"client标识,继电器端，继电器序号"
         PrintRelay(client,RELAY2,2);
         PrintRelay(client,RELAY3,3);
         PrintRelay(client,RELAY4,4);
         client.println("</tr></tbody></table>"
                        "<table  width='100%'  id='sw'>");
         client.print("<tbody><tr><td width='100%' align='center' bgcolor='");                        
         client.print("'><form method='post' id='swform'>");
         client.print("<input type='text' name='sw' id='sw' value=''style='display:none'></form></td></tr></tbody></table></td></tr></table></div></div>");
         
         client.println("</body></html>");    
         break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
   delay(1);
    // close the connection:
   client.stop();
   //向串口输出信息(客户端离线)
   //Serial.println("client disonnected");
  }
}

void SetRelay(char flag)  //设置继电器状态
{
      if(digitalRead(flag)==HIGH)
       digitalWrite(flag, LOW);
      else
       digitalWrite(flag, HIGH);
}

void PrintRelay(EthernetClient client,char flag,int i)   //输出继电器状态
{
            if(digitalRead(flag)==HIGH)
            {
               client.print("<td style='text-align:center;' bgColor=#FF0000 onClick='swform.sw.value=");//按下时提交数据
               client.print(i);
               client.print(";swform.submit()'>状态:关闭</td>");//提交表单
            }
            else
            {
               client.print("<td style='text-align:center;' bgColor=#28FF28 onClick='swform.sw.value=");//按下时提交数据
               client.print(i);
               client.print(";swform.submit()'>状态:开启</td>");//提交表单
            }
            
}

// 露点（点在此温度时，空气饱和并产生露珠）
// 参考: [url=http://wahiduddin.net/calc/density_algorithms.htm]http://wahiduddin.net/calc/density_algorithms.htm[/url] 
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

//获取BMP180气压读数
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
      }
    }
  }
}

//获取BMP180温度读数
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
  }
}
