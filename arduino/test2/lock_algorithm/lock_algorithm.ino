#include <SoftwareSerial.h> //시리얼 통신 라이브러리 호출
#include "Servo.h" //서보 라이브러리
#include <SimpleTimer.h>
#include <string.h>
#include <EEPROM.h>

SimpleTimer timer;

int servo = 8; //서보 꼽을곳
Servo myservo; //서보객체

int led = 13;//led 포트
float minVolt = 4.8;//4.8volt보다 작으면 베터리 교체 신호를 보내기 위해 지정
int secs = 60000;//60초마다 전압확인
int blueTx = 2; //Tx (블루투스 보내는핀 설정)송신
int blueRx = 3; //Rx (블루투스 받는핀 설정)수신
SoftwareSerial BT(blueTx, blueRx);  //시리얼 통신을 위한 객체선언

String rcvedPW = ""; //받는 문자열
String openCode = "";//0~9,* 로 이루어진 비밀번호
String closeCode = "ZOCIEwDAUW5PkH3caPNqzOukX1q1liip2l4oSEkyhnRmk2uDCemfZO+vd3dSSOlh21DCMrsbAx06bLsyIjVQLg==";
String masterKey = "";
String checkingCode = "";

int idx;
int startPoint = 40;//사용자 비밀번호 위치 시작점
int endPoint = 40;//사용자 비밀번호 위치 끝점 초기값
int pwMaxlength = 89;//해쉬값 길이88, 비번변경시 **, 마지막 NULL값
int pwflag = 0;//비밀번호 변경 flag
int openflag = 1;//현재 잠금장치가 열려있는 것으로 초기화
int batteryflag = 0;//베터리가 부족한지 아닌지 확인을 위한 flag

char myChar;
void setup() {
    BT.begin(9600); //블루투스 시리얼 개방
    pinMode(led, OUTPUT);
    Serial.begin(9600);
    
    int openflag = 1;//초기엔 잠금장치가 열린다
    servoWrite(0);//잠금장치를 연다

    for(int i = 0; i<40; i++){//ROM에 저장된 마스터키를 받아온다.
        masterKey += (char)EEPROM.read(i);
    }
    
    for(int i = startPoint; i < startPoint+pwMaxlength; i++){//ROM에 저장된 사용자 비밀번호를 받아온다.
        if(EEPROM.read(i) != '\0'){//NULL이 아니면
            openCode += (char)EEPROM.read(i);//openCode변수에 사용자 비밀번호 저장
            endPoint++;//ROM의 사용자 비밀번호 위치 끝점 업데이트
        }
    }

    Serial.println("-----start-----");
    checkingVolt();//시작과 동시에 베터리 상태를 파악
    timer.setInterval(secs, checkingVolt);//60.000초마다 volt값을 확인하고 모니터에 출력한다.
}

void loop(){
    // Serial.print("rcvedPW = ");
    // Serial.print(rcvedPW);
    // Serial.println();

    while(BT.available()){
        myChar = (char)BT.read();
        rcvedPW += myChar;
        delay(1);
    }
    if(rcvedPW.equals(masterKey));
    else if((idx = rcvedPW.indexOf('\n')) != -1 && rcvedPW.length() >= 88) rcvedPW.remove(idx);
    else if(rcvedPW.length() != 88 || rcvedPW.length() != 90) rcvedPW = "";

    if(!rcvedPW.equals("")){
        

        // Serial.print("openCode = ");
        // Serial.print(openCode);
        // Serial.println();
        // Serial.print("rcvedPW = ");
        // Serial.print(rcvedPW);
        // Serial.println();
        

        //비번변경 flag가 안켜져있고, *로 시작하며 끝나고, * 사이의 문자열이 사용자비밀번호와 일치한다면
        if(canChangePW()){
            Serial.println("changing pw");

            pwflag = 1;//비번변경 flag를 켠다
            rcvedPW = "";//받은 문자열은 초기화한다.

        //비번변경 flag가 켜져있고, 받은 문자열이 빈것이 아니고, closeCode가 아니고, *문자가 들어있지않고, 비번 최대값 이하라면
        }else if(isPossiblePW()){
            Serial.println("possible pw");
            
            checkingCode = rcvedPW;
            pwflag = 2;//비번변경 flag를 다음단계로 바꿔준다.
            rcvedPW = "";//받은 문자열은 초기화 해준다.

        }else if(checkingTwice()){
            Serial.println("checking twice");

            int newidx = 0;//새로운 비번의 인덱스번호를 참조하기 위해 선언
            endPoint = startPoint;//ROM의 끝점을 다시 맞추기위해 초기화
            openCode = "";//사용자 비번을 재설정하기전에 초기화

            for(int i = startPoint; i < startPoint+pwMaxlength; i++){//ROM의 사용자 비번 시작점부터 비번최대값까지
                EEPROM.write(i, '\0');//NULL로 초기화하고

                if(i < (rcvedPW.length()+startPoint)){//i 가 받은 문자열길이보다 작다면
                    EEPROM.write(i, rcvedPW.charAt(newidx));//새로운 비번의 각 인덱스의 값을 ROM에 저장한다.
                    endPoint++;//끝 주소값을 하나 증가한다.
                    newidx++;//인덱스의 값을 하나 증가 한다.
                }
            }
            openCode = rcvedPW;//사용자 비번을 받은 문자열로 재 설정한다.
            pwflag = 0;//비번변경 flag를 꺼준다.
            rcvedPW = "";//받은 문자열은 초기화 해준다.
        }else{
            checkingCode = "";
            pwflag = 0;//비번변경을 취소시킨다.
        }
    

    // if(!rcvedPW.equals("")){//받은 문자열이 빈문자열이 아니라면
        //받은 문자열이 사용자 비번으로 시작하거나 끝나거나 마스터키라면
        if(openflag == 0 && rcvedPW.equals(openCode) || rcvedPW.equals(masterKey)){
            if(rcvedPW.equals(masterKey)){//마스터키가 입력됐다면 0000으로 초기화 한다.

                endPoint = startPoint;//ROM의 끝점을 다시 맞추기위해 초기화
                openCode = "8M7ookDio1H3+ecSuwzU5W+S1bbWs1gVabmZrMmNnxeCHFfFT7PARmLcUthPPNKhkrGLgOy3TPBRN4iBvvUHfQ==";//사용자 비번을 재설정하기전에 초기화

                for(int i = startPoint; i < startPoint+pwMaxlength; i++){//ROM의 사용자 비번 시작점부터 비번최대값까지
                    EEPROM.write(i, '\0');//NULL로 초기화하고
                    if(i < (openCode.length()+startPoint)){//0000길이만큼
                        EEPROM.write(i, openCode.charAt(i-masterKey.length()));//각 인덱스의 값을 ROM에 저장한다.
                        endPoint++;//끝 주소값을 하나 증가한다.
                    }
                }
            }
            servoWrite(0);//잠금장치를 연다
            openflag = 1;

        }else if(openflag==1 && rcvedPW.equals(closeCode)){//close 값을 넘겨받았다면
            servoWrite(110);//닫는다
            openflag = 0;
        }
        
        rcvedPW = "";//받은 문자열을 초기화한다.
    }

    if(batteryflag) turnOnLED();
    else turnOffLED();

    timer.run();
}

void checkingVolt(){
    float result = (float)readVcc()/1000.0;//volt단위의 전압을 result변수에 넣는다
    Serial.print("volt : ");
    Serial.println(result);//시리얼 모니터에 보인다.
    
    if(result < minVolt){
        batteryflag = 1;
    }else batteryflag = 0;
    
}

void turnOnLED(){
    digitalWrite(led, HIGH);
}

void turnOffLED(){
    digitalWrite(led,LOW);
}

void servoWrite(int angle){//서보모터를 움직이는 함수
    myservo.attach(servo);//서보모터를 사용하게 한다 
    delay(50);//0.05초 멈춘다.
    myservo.write(angle);//angle만큼 움직인다
    delay(400);//1초 멈춘다.
    myservo.detach();//서보모터를 해체한다
}

int canChangePW(void){
    if(pwflag == 0 && rcvedPW.startsWith("*") && rcvedPW.endsWith("*") && openCode.equals(rcvedPW.substring(1, rcvedPW.length()-1))) return 1;
    else return 0;
}

int isPossiblePW(void){
    if(pwflag == 1 && !rcvedPW.equals("") && !rcvedPW.equals(closeCode) && rcvedPW.indexOf("*")==-1 && rcvedPW.length() <= pwMaxlength) return 1;
    else return 0;
}

int checkingTwice(void){
    if(pwflag == 2 && rcvedPW.equals(checkingCode) && !rcvedPW.equals(closeCode) && rcvedPW.indexOf("*")==-1 && rcvedPW.length() <= pwMaxlength) return 1;
    else return 0;
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




/*
    if(BT.available()>0){//블루투스에 값이 있으면
        char myChar = (char)BT.read();
        int i=1;
        while(myChar != '\n'){//값이 없을때 까지 반복한다.
            Serial.print(i);
            rcvedPW += myChar;
            myChar = (char)BT.read();
            i++;
            delay(1);//끊김방지를 위해 넣음
        }
        Serial.print("openCode = ");
        Serial.print(openCode);
        Serial.println();
        Serial.print("rcvedPW = ");
        Serial.print(rcvedPW);
        Serial.println();
        

        //비번변경 flag가 안켜져있고, *로 시작하며 끝나고, * 사이의 문자열이 사용자비밀번호와 일치한다면
        if(canChangePW()){
            Serial.println("chaning pw");
            pwflag = 1;//비번변경 flag를 켠다
            rcvedPW = "";//받은 문자열은 초기화한다.

        //비번변경 flag가 켜져있고, 받은 문자열이 빈것이 아니고, closeCode가 아니고, *문자가 들어있지않고, 비번 최대값 이하라면
        }else if(isPossiblePW()){
            Serial.println("possible pw");
            int newidx = 0;//새로운 비번의 인덱스번호를 참조하기 위해 선언
            
            endPoint = startPoint;//ROM의 끝점을 다시 맞추기위해 초기화
            openCode = "";//사용자 비번을 재설정하기전에 초기화

            for(int i = startPoint; i < startPoint+pwMaxlength; i++){//ROM의 사용자 비번 시작점부터 비번최대값까지
                EEPROM.write(i, '\0');//NULL로 초기화하고

                if(i < (rcvedPW.length()+startPoint)){//i 가 받은 문자열길이보다 작다면
                    EEPROM.write(i, rcvedPW.charAt(newidx));//새로운 비번의 각 인덱스의 값을 ROM에 저장한다.
                    endPoint++;//끝 주소값을 하나 증가한다.
                    newidx++;//인덱스의 값을 하나 증가 한다.
                }
            }
            openCode = rcvedPW;//사용자 비번을 받은 문자열로 재 설정한다.
            pwflag = 0;//비번변경 flag를 꺼준다.
            rcvedPW = "";//받은 문자열은 초기화 해준다.

        }else{
            pwflag = 0;//비번변경을 취소시킨다.
        }
    }

    if(!rcvedPW.equals("")){//받은 문자열이 빈문자열이 아니라면
        //받은 문자열이 사용자 비번으로 시작하거나 끝나거나 마스터키라면
        if(openflag == 0 && rcvedPW.equals(openCode) || rcvedPW.equals(masterKey)){
            if(rcvedPW.equals(masterKey)){//마스터키가 입력됐다면 0000으로 초기화 한다.

                endPoint = startPoint;//ROM의 끝점을 다시 맞추기위해 초기화
                openCode = "8M7ookDio1H3+ecSuwzU5W+S1bbWs1gVabmZrMmNnxeCHFfFT7PARmLcUthPPNKhkrGLgOy3TPBRN4iBvvUHfQ==";//사용자 비번을 재설정하기전에 초기화

                for(int i = startPoint; i < startPoint+pwMaxlength; i++){//ROM의 사용자 비번 시작점부터 비번최대값까지
                    EEPROM.write(i, '\0');//NULL로 초기화하고
                    if(i < (openCode.length()+startPoint)){//0000길이만큼
                        EEPROM.write(i, openCode.charAt(i-masterKey.length()));//각 인덱스의 값을 ROM에 저장한다.
                        endPoint++;//끝 주소값을 하나 증가한다.
                    }
                }
            }
            servoWrite(0);//잠금장치를 연다
            openflag = 1;

        }else if(openflag==1 && rcvedPW.equals(closeCode)){//close 값을 넘겨받았다면
            servoWrite(110);//닫는다
            openflag = 0;
        }
        
        rcvedPW = "";//받은 문자열을 초기화한다.
    }
    if(batteryflag) turnOnLED();
    else turnOffLED();

    timer.run();
*/
