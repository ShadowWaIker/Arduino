#include <dht11.h>
#include <SFE_BMP180.h>
#include <Wire.h>
 
dht11 DHT11;
#define DHT11PIN 7 //bmp180已经占用2号端口
SFE_BMP180 pressure;// 创建一个气压计对象
 
void setup()
{
  Serial.begin(9600);
  pressure.begin();
}
 
void loop()
{
  //Serial.print("DHT11 LIBRARY VERSION: ");
  //Serial.println(DHT11LIB_VERSION);
  
  int chk = DHT11.read(DHT11PIN);
  double p,t;
  
  Serial.print("\nRead sensor: ");
  switch (chk)
  {
    case DHTLIB_OK: 
                Serial.println("OK"); 
                break;
    case DHTLIB_ERROR_CHECKSUM: 
                Serial.println("Checksum error"); 
                break;
    case DHTLIB_ERROR_TIMEOUT: 
                Serial.println("Time out error"); 
                break;
    default: 
                Serial.println("Unknown error"); 
                break;
  }
 
  Serial.print("Humidity (%): ");
  Serial.println((float)DHT11.humidity, 2);
 
  Serial.print("Temperature (oC): ");
  Serial.println((float)DHT11.temperature, 2);
 
  Serial.print("Temperature (oF): ");
  Serial.println(Fahrenheit(DHT11.temperature), 2);
 
  Serial.print("Temperature (K): ");
  Serial.println(Kelvin(DHT11.temperature), 2);
 
  Serial.print("Dew Point (oC): ");
  Serial.println(dewPoint(DHT11.temperature, DHT11.humidity));
 
  Serial.print("Dew PointFast (oC): ");
  Serial.println(dewPointFast(DHT11.temperature, DHT11.humidity));
  
  //bmp180
  p = getP();// 获得一个气压值
  Serial.print("Atmospheric pressure: ");
  if (p >= 0.0) Serial.print(" "); // 调整正数显示格式
  Serial.print(p,1);
  Serial.print(" hPa ");  
 
  t = getT();// 获得一个温度值
  Serial.print("temperature: ");
  Serial.print(t,1);
  Serial.println(" degrees");
  
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(2000);
}

double Fahrenheit(double celsius) 
{
        return 1.8 * celsius + 32;
}    //摄氏温度度转化为华氏温度
 
double Kelvin(double celsius)
{
        return celsius + 273.15;
}     //摄氏温度转化为开氏温度
 
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
 
// 快速计算露点，速度是5倍dewPoint()
// 参考: [url=http://en.wikipedia.org/wiki/Dew_point]http://en.wikipedia.org/wiki/Dew_point[/url]
double dewPointFast(double celsius, double humidity)
{
        double a = 17.271;
        double b = 237.7;
        double temp = (a * celsius) / (b + celsius) + log(humidity/100);
        double Td = (b * temp) / (a - temp);
        return Td;
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
