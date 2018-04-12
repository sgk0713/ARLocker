#include <EEPROM.h>
#include <String.h>

String openCode = "8M7ookDio1H3+ecSuwzU5W+S1bbWs1gVabmZrMmNnxeCHFfFT7PARmLcUthPPNKhkrGLgOy3TPBRN4iBvvUHfQ==";//88
String masterKey = "0DE8587C6572AD5BBCD7DD7B034126EBA3A07806";//40자리

void setup() {
  Serial.begin(9600);
  
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    if(i < masterKey.length()){//masterKey 입력
      EEPROM.write(i, masterKey.charAt(i));
      delay(10);
    }else if(i < openCode.length()+masterKey.length()){//0000 으로 비밀번호 초기화
      EEPROM.write(i, openCode.charAt(i-masterKey.length()));
      delay(10);
    }else{
      EEPROM.write(i, '\0');//NULL로 초기화
      delay(10);
    }
  }
  
  for (int i = 0 ; i < EEPROM.length() ; i++) {//출력으로 값 확인
    if(i == masterKey.length() || i == masterKey.length() + openCode.length()) {
      Serial.println();
      Serial.print("=================================");
      Serial.println();
    }
    if(i>=masterKey.length()+openCode.length()){
      Serial.print(EEPROM.read(i), DEC);
      
      if(EEPROM.read(i) == 0){
        Serial.print("\ttrue");
      }
      Serial.println();
    }else{
      Serial.print("i = ");
      Serial.print(i);
      Serial.print("\t");
      Serial.print(EEPROM.read(i), DEC);
      Serial.print("\t");
      Serial.print((char)EEPROM.read(i));
      
      Serial.print("\t");
      if(EEPROM.read(i) == 0){
        Serial.print("true");
      }
      Serial.println();
    }
  }
}
void loop() {
  delay(5000);
}
