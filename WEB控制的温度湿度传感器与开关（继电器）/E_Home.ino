#include <SPI.h>
#include <Ethernet.h>
#include <dht11.h>
#include <SFE_BMP180.h>
#include <Wire.h>

/*
 * Enter a MAC address and IP address for your controller below.
 * The IP address will be dependent on your local network:
 */
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xBF
};
/*服务端IP地址 */
IPAddress ip( 10, 0, 0, 254 );


/*
 * Initialize the Ethernet server library
 * with the IP address and port you want to use
 * (port 80 is default for HTTP):
 */
EthernetServer server( 80 );


/* 设置继电器端口 */
#define RELAY1  12
#define RELAY2  11
#define RELAY3  9
#define RELAY4  8
/* 设置传感器端口 */
dht11 DHT11;
#define DHT11PIN 7      /* bmp180已经占用2号端口,13号端口无法使用，3-6不可用，10口与网卡版TX想通 */
SFE_BMP180 pressure;    /* 创建一个气压计对象 */

void setup()
{
  /* start the Ethernet connection and the server: */
  Ethernet.begin( mac, ip );
  server.begin();
  /* 初始化BMP180传感器 */
  pressure.begin();
  /* 设置端口为输出 */
  pinMode( RELAY1, OUTPUT );
  pinMode( RELAY2, OUTPUT );
  pinMode( RELAY3, OUTPUT );
  pinMode( RELAY4, OUTPUT );
  /* 设置继电器默认状态为关闭 */
  digitalWrite( RELAY1, HIGH );
  digitalWrite( RELAY2, HIGH );
  digitalWrite( RELAY3, HIGH );
  digitalWrite( RELAY4, HIGH );
}


void loop()
{
  /* listen for incoming clients */
  EthernetClient client = server.available();
  if ( client )
  {
    /* an http request ends with a blank line */
    boolean currentLineIsBlank = true;
    while ( client.connected() )
    {
      if ( client.available() )
      {
        char c = client.read();
        /*
         * 将收到的客户端信息发送到串口(调试用)
         * Serial.write(c);
         * if you've gotten to the end of the line (received a newline
         * character) and the line is blank, the http request has ended,
         * so you can send a reply
         */
        if ( c == '\n' && currentLineIsBlank ){
          //定义头部
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Access-Control-Allow-Origin: *"); //允许跨域请求
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println(); //头部后面需要一个空行才能正确显示网页
          /*
           * it is after the double cr-lf that the variables are
           * read another line!
           * String POST = "";  //定义POST变量
           */
          while ( client.available() )
          {
            c = client.read();
            /* POST += c;   //保存POST数据 */
          }

          if ( String( c ) == "1" ){ /* 执行POST请求对应的操作 */
            SetRelay( RELAY1 );
          }else if ( String( c ) == "2" ){
            SetRelay( RELAY2 ); /* 切换RGB模式需要按下按钮后再打开 */
            delay(500);
            SetRelay( RELAY2 );
          }else if ( String( c ) == "3" ){
            SetRelay( RELAY3 );
          }else if ( String( c ) == "4" ){
            SetRelay( RELAY4 );
          }
          
          /* 获取DHT11传感器读数 */
          DHT11.read( DHT11PIN );
          double a, p, t;
          p = getP();                       /* 获得一个气压值 */
          a = pressure.altitude( p, 1005 ); /* 获得基于基准气压的高度，第二个参数是基准气压 */
          /* 输出JSONP函数（使用JSONP支持跨域） */
          //client.print( "success_jsonpCallback({\"DHT11_Temperature\":" );
          /* 输出JSON */
          client.print( "{\"DHT11_Temperature\":" );
          client.print( (float) DHT11.temperature, 2 );
          client.print( ",\"DHT11_Humidity\":" );
          client.print( (float) DHT11.humidity, 2 );
          client.print( ",\"DHT11_Dew_Point\":" );
          client.print( dewPoint( DHT11.temperature, DHT11.humidity ) );
          client.print( ",\"BMP180_Temperature\":" );
          client.print( (float) getT(), 2 );
          client.print( ",\"BMP180_Pressure\":" );
          client.print( p, 1 );
          client.print( ",\"BMP180_Altitude\":" );
          client.print( a, 1 );
          client.print( ",\"Switch\":{" );
          client.print( "\"1\":" );
          PrintRelay( client, RELAY1, 1 ); /* 参数为"client标识,继电器端，继电器序号" */
          client.print( ",\"2\":" );
          PrintRelay( client, RELAY2, 2 );
          client.print( ",\"3\":" );
          PrintRelay( client, RELAY3, 3 );
          client.print( ",\"4\":" );
          PrintRelay( client, RELAY4, 4 );
          //client.print( "}});" ); /* JSONP响应格式 */
          client.print( "}}" ); /* JSON响应格式 */
          break;
        }
        if ( c == '\n' )
        {
          /* you're starting a new line */
          currentLineIsBlank = true;
        }else if ( c != '\r' )
        {
          /* you've gotten a character on the current line */
          currentLineIsBlank = false;
        }
      }
    }
    /* give the web browser time to receive the data */
    delay( 1 );
    /* close the connection: */
    client.stop();
  }
}


void SetRelay( char flag )  /* 设置继电器状态 */
{
  if ( digitalRead( flag ) == HIGH ){
    digitalWrite( flag, LOW );
  } else {
    digitalWrite( flag, HIGH );
  }
}


void PrintRelay( EthernetClient client, char flag, int i )    /* 输出继电器状态 */
{
  if ( digitalRead( flag ) == HIGH ){
    client.print( "\"OFF\"" );
  } else {
    client.print( "\"ON\"" );
  }
}


/*
 * 露点（点在此温度时，空气饱和并产生露珠）
 * 参考: [url=http://wahiduddin.net/calc/density_algorithms.htm]http://wahiduddin.net/calc/density_algorithms.htm[/url]
 */
double dewPoint( double celsius, double humidity )
{
  double  A0  = 373.15 / (273.15 + celsius);
  double  SUM = -7.90298 * (A0 - 1);
  SUM += 5.02808 * log10( A0 );
  SUM += -1.3816e-7 * (pow( 10, (11.344 * (1 - 1 / A0) ) ) - 1);
  SUM += 8.1328e-3 * (pow( 10, (-3.49149 * (A0 - 1) ) ) - 1);
  SUM += log10( 1013.246 );
  double  VP  = pow( 10, SUM - 3 ) * humidity;
  double  T = log( VP / 0.61078 ); /* temp var */
  return( (241.88 * T) / (17.558 - T) );
}


/* 获取BMP180气压读数 */
double getP()
{
  char  status;
  double  T, P, p0, a;
  /*
   * You must first get a temperature measurement to perform a pressure reading.
   * Start a temperature measurement:
   * If request is successful, the number of ms to wait is returned.
   * If request is unsuccessful, 0 is returned.
   */
  status = pressure.startTemperature();
  if ( status != 0 )
  {
    /* Wait for the measurement to complete: */
    delay( status );
    /*
     * Retrieve the completed temperature measurement:
     * Note that the measurement is stored in the variable T.
     * Use '&T' to provide the address of T to the function.
     * Function returns 1 if successful, 0 if failure.
     */

    status = pressure.getTemperature( T );
    if ( status != 0 )
    {
      /*
       * Start a pressure measurement:
       * The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
       * If request is successful, the number of ms to wait is returned.
       * If request is unsuccessful, 0 is returned.
       */
      status = pressure.startPressure( 3 );
      if ( status != 0 )
      {
        /* Wait for the measurement to complete: */
        delay( status );
        /*
         * Retrieve the completed pressure measurement:
         * Note that the measurement is stored in the variable P.
         * Use '&P' to provide the address of P.
         * Note also that the function requires the previous temperature measurement (T).
         * (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
         * Function returns 1 if successful, 0 if failure.
         */
        status = pressure.getPressure( P, T );
        if ( status != 0 )
        {
          return(P);
        }
      }
    }
  }
}


/* 获取BMP180温度读数 */
double getT()
{
  char  status;
  double  T, p0;
  status = pressure.startTemperature();
  if ( status != 0 )
  {
    delay( status );
    status = pressure.getTemperature( T );
    if ( status != 0 )
    {
      status = pressure.startPressure( 3 );
      return(T);
    }
  }
}
