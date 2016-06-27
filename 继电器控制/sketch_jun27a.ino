int jdqPin=13;//定义数字检测接
void setup()
{
  pinMode(jdqPin,OUTPUT);//设定数字接口13、12、11为输入接口
  pinMode(12,OUTPUT);//设定数字接口13、12、11为输入接口
  Serial.begin(9600);//设置串口波特率为9600kbps
}
void loop()
{
digitalWrite(13,HIGH);
digitalWrite(12,HIGH);
delay(1000);
digitalWrite(12,LOW);
digitalWrite(13,LOW);
delay(1000);
}
