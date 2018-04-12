#include <SoftwareSerial.h> //시리얼 통신 라이브러리 호출
#include "Servo.h" //서보 라이브러리
#include <SimpleTimer.h>

SimpleTimer timer;
Servo myservo; //서보객체
int blueTx = 2; //Tx (블투 보내는핀 설정)송신
int blueRx = 3; //Rx (블투 받는핀 설정)수신
int servo = 12;//서보 꼽을곳
SoftwareSerial BT(blueTx, blueRx);  //시리얼 통신을 위한 객체선언
String myString = ""; //받는 문자열

int result= 0;

void sendingVolt(){
  result = readVcc();//milli volt단위의 전압을 result변수에 넣는다
  Serial.print("Volt :");
  Serial.println(result);//시리얼 모니터에 보인다.
  BT.print("volt : ");
  BT.println(result);//블루투스로 데이터를 송신한다.
}
long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}


void setup() {
  Serial.begin(9600);
  Serial.println("start");
  myservo.attach(servo);   //서보 시그널 핀설정
  myservo.write(0);     //서보 초기각도 0도 설정
  BT.begin(9600); //블루투스 시리얼 개방
  timer.setInterval(5000, sendingVolt);
}

void loop() {
  while (BT.available()) { //BT 값이 있으면
    char myChar = (char)BT.read();  //BT int형식의 값을 char형식으로 변환
    myString += myChar; //수신되는 문자열을 myString에 모두 붙임 (1바이트씩 전송되는 것을 모두 붙임)
    delay(5);           //수신 문자열 끊김 방지
  }
  
  if (!myString.equals("")) //myString 값이 있다면
  {
    Serial.println("input value: " + myString); //시리얼모니터에 myString값 출력
    if (myString == "open") //myString 값이 'l\n' 이라면
    {
      myservo.write(180);     //각도 180도로 움직임
    } else if(myString == "close"){
      myservo.write(0);   //각도 0도로 움직임
    }else{
    }
    myString = ""; //myString 변수값 초기화
  }
  
  timer.run();
}
