/*NERVTEK
 * CREATIVITY DEPARTMENT
 * ARDUINO PLUG AND PLAY DATA LOGGER: MENU AND NRF24L01 TEST CODE 1.3
 * SENDING CODE BY  SIR KAROL KONARSKI AND SIR TAM ANDERSON
 * 29/06/2021  09:34 AM
 */
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include<DHT.h>
#include<DS3231.h>
#include<nRF24L01.h>
#include<SPI.h>
#include<RF24.h>
#include<SD.h>

const int chipSelect = 4;

//LOGGER DETAILS
int sample_id = 12345;
int id_len = 9;
char logger_id[] = {'c','r','e','a','t','i','v','e','\0'};

//PIN ASSIGMENT FOR NRF MODULE
int ce = 5;
int csn = 6;
int debug_led =2;

//INTERNAL VARIABLES
//byte power_mode = "RF24_PA_MIN";      //Operating Power Mode
int rest_time =200;                     //rest time after transmitting
const uint64_t addresse = 0x1111111111; // address
int baud_rate = 9600;
int blink_duration_default = 500;
int error_blink_time = 20;

//TRANSMISSION VARIABLES
const int maximum_digits_to_transfer = 5;  //Arduino has issues handling more than 5 digits
const int char_length = maximum_digits_to_transfer;

//JSON DATA STRUCTURE 
 typedef struct{
  char id[char_length];
  char temp[char_length];
  char humidity[char_length];
  char moist[char_length];
 }
 sensorData;

 //************************************************************************************
 RF24 radio(ce, csn);               //Create RF24 Object
 sensorData logger;                 //Create instance of sensorData struct
 //************************************************************************************

 //Blink LED Function

 void blinkLED(int led, int blink_for = blink_duration_default){
  digitalWrite(led,HIGH);
  delay(blink_for);
  digitalWrite(led,LOW);
  delay(blink_for);
 }
 
//***************************************************************************************************8

//ASSIGN FUNCTION
void  assign(char sensor[],  int value_rec = 0,  char char_value[] = "!" ){
 int max_digit = maximum_digits_to_transfer; 

//Check if a CHAR PARAMETER HAS BEEN PASSED. CHAR CHARACTERS HAVE HIGHER PRECIDENCE 
if( char_value != "!"){     
  for (int j = 0; j<char_length; j++){
  sensor[j] = char(char_value[j]);
  }
  delay(rest_time);
  return;
} //***********************************

//If an Integer Value is passed
 int  value = value_rec/1, count = 0;   //Ensure value is int
 int remainder = value ;
 for(int index = 0; index< char_length; index ++){       //GET HOW MANY DIGITS IN VALUE
    count ++; remainder = remainder/ 10;
    if (remainder == 0) index = char_length;
  } 
  
  if (count>= max_digit +1){// Get how many digits are in string
     //delay(rest_time);
     Serial.println("Error Transmission value from  " + String(sensor) + String(value) + " too large !!!");
     blinkLED(debug_led, error_blink_time);
     return;
    }//********************************************
    
  String str = "0";   //Create String Str with value '0'
  str += value;       //join '0' to value passed
  str.remove(0, 1);   // remove leading zeros
  str.remove(count);  //remove trainling spaces after digit
  char char_buff[char_length];
  str.toCharArray(char_buff, char_length);
  for (int j = 0; j<char_length; j++){
  sensor[j] = char(char_buff[j]);
  }
  str = " ";
   delay(rest_time); return;
}
//***************************************************************************************

LiquidCrystal_I2C lcd(0x27, 20, 4);
RTClib myRTC;

#define DHTTYPE DHT22

#define reset 7     //RESET BUTTON

#define PORT1 9
#define PORT2 8
#define PORT3 A0

int a, b, c = 0;
int p;
int sMPin;
int sMValue;
int sensorType;

int upButton = 10;
int downButton = 11;
int okButton = 12;
int menu = 1;

int t;
int h;

void setup() {
  //RADIO TRANSMISSION SETUP
  Serial.begin(baud_rate);
  pinMode(debug_led, OUTPUT);
  Serial.println("Setting up Transmitter...");
  radio.begin();
  radio.stopListening();
  radio.openWritingPipe(addresse);
  Serial.println("Done");
  blinkLED(debug_led);

  //INITIALIZING MENU AND ON-BOARD SYSTEM
  
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(6,1);
  lcd.print("WELCOME");
  delay(1000);
  Serial.println("Initializing SD Card...");
  if(!SD.begin(chipSelect)){
    Serial.println("Card Failed, or Not Present");
    lcd.setCursor(2,1);
    lcd.print("INSERT VALID SD");
    lcd.setCursor(6,2);
    lcd.print("CARD!!");
    while(1);
  }
  Serial.println("Card Initialized");
  lcd.clear();
  lcd.print("Card Initialized");
  delay(1000);
  lcd.clear();
  digitalWrite(reset, HIGH);
  delay(200);
  pinMode(reset, OUTPUT);
  pinMode(PORT1, INPUT);
  pinMode(PORT2, INPUT);
  pinMode(PORT3, INPUT);
  pinMode(upButton, INPUT_PULLUP);
  pinMode(downButton, INPUT_PULLUP);
  pinMode(okButton, INPUT_PULLUP);
  updateMenu();
}

void loop() {
  if(!digitalRead(downButton)){
    menu++;
    updateMenu();
    delay(100);
    while(!digitalRead(downButton));
  }
  if(!digitalRead(upButton)){
    menu--;
    updateMenu();
    delay(100);
    while(!digitalRead(upButton));
  }
  if(!digitalRead(okButton)){
    executeAction();
    updateMenu();
    delay(100);
    while(!digitalRead(okButton));
  }
}

void updateMenu(){

  switch(menu){
    case 0:
        menu = 1;
        break;
    case 1:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(">Start");
        lcd.setCursor(0, 1);
        lcd.print(" Configure");
        lcd.setCursor(0, 2);
        lcd.print(" Reset");
        sendData();
        break;
    case 2:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(" Start");
        lcd.setCursor(0, 1);
        lcd.print(">Configure");
        lcd.setCursor(0, 2);
        lcd.print(" Reset");
        break;
    case 3:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(" Start");
        lcd.setCursor(0, 1);
        lcd.print(" Configure");
        lcd.setCursor(0, 2);
        lcd.print(">Reset");
        break;   
    case 4:
        menu = 3;
        break;
    case 5:
        menu = 6;
        break;
    case 6:
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("Configure Ports");
        lcd.setCursor(0, 1);
        lcd.print(">Port 1: ");
        lcd.setCursor(0, 2);
        lcd.print(" Port 2: ");
        lcd.setCursor(0, 3);
        lcd.print(" Port 3: ");
        break;
     case 7:
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print("Configure Ports");
        lcd.setCursor(0, 1);
        lcd.print(" Port 1: ");
        lcd.setCursor(0, 2);
        lcd.print(">Port 2: ");
        lcd.setCursor(0, 3);
        lcd.print(" Port 3: ");
        break;
     case 8:
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print("Configure Ports");
        lcd.setCursor(0, 1);
        lcd.print(" Port 1: ");
        lcd.setCursor(0, 2);
        lcd.print(" Port 2: ");
        lcd.setCursor(0, 3);
        lcd.print(">Port 3: ");
        break;
     case 9:
        menu = 8;
        break;
     case 10:
        menu = 11;
        break;
     case 11: 
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print("List of Sensors");
        lcd.setCursor(0, 1);
        lcd.print(">Temperature");
        lcd.setCursor(0, 2);
        lcd.print(" Humidity");
        lcd.setCursor(0, 3);
        lcd.print(" Soil Moisture");
        break;  
     case 12:
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print("List of Sensors");
        lcd.setCursor(0, 1);
        lcd.print(" Temperature");
        lcd.setCursor(0, 2);
        lcd.print(">Humidity");
        lcd.setCursor(0, 3);
        lcd.print(" Soil Moisture");
        break;
     case 13:
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print("List of Sensors");
        lcd.setCursor(0, 1);
        lcd.print(" Temperature");
        lcd.setCursor(0, 2);
        lcd.print(" Humidity");
        lcd.setCursor(0, 3);
        lcd.print(">Soil Moisture");
        break; 
     case 14:
        menu = 13;
        break;
     case 15:
        menu =16;
        break;
     case 16:
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(">OK");
        lcd.setCursor(0, 2);
        lcd.print(" Cancel");
        break;
     case 17:
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(" OK");
        lcd.setCursor(0, 2);
        lcd.print(">Cancel");
        break;
     case 18:
        menu = 17;
        break;
     case 19:
        menu = 20;
        break;
     case 20:
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(">Done");
        lcd.setCursor(0, 2);
        lcd.print(" Back");  
        break;
     case 21:
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(" Done");
        lcd.setCursor(0, 2);
        lcd.print(">Back");  
        break;
     case 22:
        menu = 21;
        break;
        
  }
}

void executeAction(){
  switch(menu){
    case 1:
        action1();
        break;
    case 2:
        menu = 6;
        break;
    case 3:
        action3();
        break;
    case 6:
        p = 1;
        menu = 11;
        break;
    case 7:
        p = 2;
        menu = 11;
        break;
    case 8:
        p = 3;
        menu = 11;
        break;
    case 11:
        sensorType = 1;
        menu = 16;
        break;
    case 12:
        sensorType = 2;
        menu = 16;
        break;
    case 13:
        sensorType = 3;
        menu = 16;
        break;
    case 16:
        if(sensorType == 1){
          a = p;
          temperatureConfig();    
        }
        else if(sensorType == 2){
          b = p;
          humidityConfig();
        }
        else if(sensorType == 3){
          c = p;
          soilMoistureConfig();
        }
        menu = 20;
        break;
    case 17:
        menu = 11;
        break;
    case 20:
        sendData();
        menu = 1;
        break;
    case 21:
        menu = 6;
        break;
  }
}

void action1(){
 while(1){
 lcd.clear();
  if(a>0){
    readTemp();
  }
  if(b>0){
    readHumid();
  }
  if(c>0){
    readSM();
  }
  sendData();
  DateTime now = myRTC.now();
  lcd.setCursor(5,3);
  lcd.print(now.hour(), DEC);
  lcd.print(":");
  lcd.print(now.minute(), DEC);
  lcd.print(":");
  lcd.print(now.second(), DEC);
    delay(1000);
    File dataLogger = SD.open("DataInfo.txt", FILE_WRITE);
    if(dataLogger){
      dataLogger.print(now.hour(),DEC);dataLogger.print(":");dataLogger.print(now.minute(),DEC);dataLogger.print(":");dataLogger.println(now.second(),DEC);
      dataLogger.print("Temperature: ");
      dataLogger.println(t);
      dataLogger.print("Humidity: ");
      dataLogger.println(h);
      dataLogger.print("Soil Moisture: ");
      dataLogger.println(sMValue);
      dataLogger.close();
    }
    if(!digitalRead(okButton))
    {break;} 
  } 
}

void action3(){
    digitalWrite( reset, LOW);  
}

float temperatureConfig(){
      if(a == 1){
         DHT dht(PORT1, DHTTYPE);
         dht.begin();
         delay(1000);
         float t = dht.readTemperature();
         return t;
      }
      else if(a == 2){
         DHT dht(PORT2, DHTTYPE);
         dht.begin();
         delay(1000);
         float t = dht.readTemperature();
         return t;
      }
      else if(a == 3){
         DHT dht(PORT3, DHTTYPE);
         dht.begin();
         delay(1000);
         float t = dht.readTemperature();
         return t;
      }
      
}

float humidityConfig(){
      if(b == 1){
         DHT dht(PORT1, DHTTYPE);
         dht.begin();
         delay(1000);
         float h = dht.readHumidity();
         return h;
      }
      else if(b == 2){
         DHT dht(PORT2, DHTTYPE);
         dht.begin();
         delay(1000);
         float h = dht.readHumidity();
         return h;
      }
      else if(b == 3){
         DHT dht(PORT3, DHTTYPE);
         dht.begin();
         delay(1000);
         float h = dht.readHumidity();
         return h;
      }  
}

void soilMoistureConfig(){
      if(c == 1){
         sMPin = PORT1;
         sMValue = analogRead(sMPin);
         sMValue = (100 -((sMValue/1023.00) *100));
      }
      else if(c == 2){
         sMPin = PORT2;
         sMValue = analogRead(sMPin);
         sMValue = (100 -((sMValue/1023.00) *100));
      }
      else if(c == 3){
         sMPin = PORT3;
         sMValue = analogRead(sMPin);
         sMValue = (100 -((sMValue/1023.00) *100));
      }
}

void readTemp(){
    t = temperatureConfig();
    lcd.setCursor(0,0);
    lcd.print("Temp: ");
    lcd.print(t);
    lcd.print("C");
}

void readHumid(){
    h = humidityConfig();
    lcd.setCursor(0,1);
    lcd.print("Humidity: ");
    lcd.print(h);
    lcd.print("%");    
}

void readSM(){
    soilMoistureConfig();
    lcd.setCursor(0,2);
    lcd.print("Soil Moist.: ");
    lcd.print(sMValue);
    lcd.print("%");
}

void sendData(){
  assign(logger.id, sample_id);
  if(a>0){
    int tempToLog = temperatureConfig();
    assign(logger.temp, tempToLog);
  }
  if(b>0){
    int humToLog = humidityConfig();
    assign(logger.humidity, humToLog);
  }
  if(c>0){
    int moistToLog = sMValue;
    assign(logger.moist, moistToLog);
  }
  Serial.println("\nHere is the assinged ID :" + String(logger.id) + "\n");
  Serial.println("\nHere is the assinged Temp VALUE :" + String(logger.temp) + "\n");
  Serial.println("\nHere is the assinged Humid :" + String(logger.humidity) + "\n"); 
  Serial.println("\nHere is the assinged MOIST VALUE :" + String(logger.moist) + "\n"); 
  radio.write(&logger, sizeof(logger));
  blinkLED(debug_led, rest_time);
}
