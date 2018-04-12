#include <SoftwareSerial.h> //시리얼 통신 라이브러리 호출
#include "Servo.h" //서보 라이브러리
 
Servo myservo; //서보객체
int blueTx=2;   //Tx (블투 보내는핀 설정)송신
int blueRx=3;   //Rx (블투 받는핀 설정)수신
SoftwareSerial BT(blueTx, blueRx);  //시리얼 통신을 위한 객체선언
String myString=""; //받는 문자열
 
void setup() {
  Serial.begin(9600);
  Serial.println("start");
  myservo.attach(12);   //서보 시그널 핀설정
  myservo.write(0);     //서보 초기각도 0도 설정
  
  BT.begin(9600); //블루투스 시리얼 개방
  
}

void loop(){
  while(BT.available()){//BT 값이 있으면
    char myChar = (char)BT.read();  //BT int형식의 값을 char형식으로 변환
    myString+=myChar;   //수신되는 문자열을 myString에 모두 붙임 (1바이트씩 전송되는 것을 모두 붙임)
    delay(5);           //수신 문자열 끊김 방지
  }
  
  if(!myString.equals(""))  //myString 값이 있다면
  {
    Serial.println("input value: "+myString); //시리얼모니터에 myString값 출력
      if(myString=="l\n")  //myString 값이 'l\n' 이라면
      {
        myservo.write(180);     //각도 180도로 움직임
      } else {
        myservo.write(0);   //각도 0도로 움직임
      }
    myString="";  //myString 변수값 초기화
  }
}
