#include <SoftwareSerial.h> //시리얼통신 라이브러리 호출
#include <EEPROM.h>
#include <string.h>


int blueTx=2;   //Tx (보내는핀 설정)at
int blueRx=3;   //Rx (받는핀 설정)
SoftwareSerial mySerial(blueTx, blueRx);  //시리얼 통신을 위한 객체선언

int i = 0;
int j = 0;
int cnt = 0;
String jj="";
char tmp;
void setup() 
{
  Serial.begin(9600);   //시리얼모니터
  mySerial.begin(9600); //블루투스 시리얼
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    
      EEPROM.write(i, '\0');//NULL로 초기화
    
  }
}
void loop()
{
  if(mySerial.available()){
    while(mySerial.available()){
      tmp = (char)mySerial.read();
      jj += tmp;
      EEPROM.write(j, tmp);
      tmp = "";
      cnt++;
      j++;
    }
  }
  
  if(cnt > 0){
      Serial.print("i = ");
      Serial.print(i);
      Serial.print("\t");
      Serial.print(EEPROM.read(i), DEC);
      Serial.print("\t");
      Serial.print((char)EEPROM.read(i));
      Serial.println();
      Serial.print(jj);
      Serial.println();
      Serial.print("==================");
      Serial.println();
      i++;
      cnt--;
  }
}

