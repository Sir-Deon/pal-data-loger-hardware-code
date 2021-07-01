/* NERVTEK
 *  CREATIVITY DEPARTMENT
 *  ARDUINO PLUG AND PLAY DATA LOGGER: nRF24lO1 TEST CODE 1.1
 *  Reciever Code
 *  4th/3/2021
 *  By Tam Anderson, Karol Charles Konarski, Deon Achuo
 */

/*VARIABLE TREE Arduino/ESP32
 * NAME            PIN          DESCRIPTION
 *            ARDUINO | ESP
 * CE           D5    | D15       CHIP ENABLE
 * CSN          D6    | D22
 * MOSI         D11   | D23
 * MISO         D12   | D19
 * SCK          D13   | D18
 * ESP EN CONNECTED TO 3.3V
 * BOARD SELECTION:FireBeetle-ESP32
 * 
 * ****************************************************************************************************************************************
 */

//LIBRARIES 
#include<nRF24L01.h>                       
#include<SPI.h>
#include<RF24.h>
#include<cloud.h>
#include<WiFi.h>
#include<Arduino_JSON.h>

String api = "https://pal-data-logger-backend.herokuapp.com/api/logger/config";

//LOGGER DETAILS**********************************************************
int id_len = 9;
char logger_id[9];                                    //Sample Logger ID


// PIN VARIABLES/ PIN ASSIGNMENT*****************************************
int ce = 15 ;                                           //chip enable
int csn = 22;             
//byte power_mode = "RF24_PA_MIN";                      //Operating Power Mode
int debug_led = 2;                                    //Debug LED
/*int led_red  = 3;                                     // Debug LED
int led_blue = 4;                                     //transmiting LED
int buz_pin = 7;                                      //Test buzzer
*/

//INTERNAL VARIABLES*********************************************************
int rest_time = 500;                                   //rest time after transmitting.
const uint64_t address = 0x1111111111;              //address
int buz_freq = 1000;                                 //buzzer frequency
int baud_rate = 9600;                                //BAUD_RATE
int blink_duration_default= 100;                     //Default blink duration

//TRANSMISSION VARIABLES******************************************************
char  text[32] ;
int m;
const int char_length = 5;

//JSON STRUCTURES*************************************************************
typedef struct{
  char id[char_length];
  char temp[char_length];
  char humidity[char_length];
  char moist[char_length];
}
sensorData;                               

//******************************************************************************
RF24 radio(ce, csn);                                 //Create RF24 Object/RADIO OBJECT
sensorData logger;                                   //Create instance of sensorData structure


//BLINK LED FUNCTION// Parameters, led_pin and blink_duration(optional)*************
void blinkLed(int led, int blink_for = blink_duration_default){
  digitalWrite(led,HIGH);
  delay(blink_for);
  digitalWrite(led,LOW);
  delay(blink_for);
}

//SETUP FUNCTION*********************************************************************
void setup(){
  WiFi.disconnect();
  WiFi.begin("Karlzzz","Myfatherischris01&");
  Serial.begin(baud_rate);
  Serial.println("Setting up RECEIVER...");
  while((!(WiFi.status() == WL_CONNECTED))){
    delay(3000);
    Serial.print("..");
  }
  Serial.println("Connected to the internet");
  Serial.print("Your IP is: ");
  Serial.println((WiFi.localIP()));
  pinMode(debug_led, OUTPUT);
  radio.begin();                                     //initialize radio object
  radio.startListening();                             //Set as Receiver
  radio.openReadingPipe(0, address);                  //Open Reading pipe at addresse
  Serial.println("Done");
  blinkLed(debug_led);
}

//LOOP FUNCTION - MAIN**********************************************************************
void loop(){
  //READ INCOMING DATA
  while (radio.available()){
    radio.read(&logger,sizeof(logger));
    Serial.println("\n");
    Serial.print("Message received from logger: ");
    Serial.println(logger.id);
    
    Serial.println("SENSOR DATA");
    Serial.print("Temp: ");
    Serial.println(logger.temp);
    Serial.print("Hum: ");
    Serial.println(logger.humidity);
    Serial.print("Soil Moisture: ");
    Serial.println(logger.moist);
    delay(rest_time);
    blinkLed(debug_led, rest_time);

   JSONVar info;                                  //Operation used to stringify the incoming
   info["Logger ID"] = logger.id;                 //structure into a String easily read by the
   info["Temperature"] = logger.temp;             //cloud library.
   info["Humidity"] = logger.humidity;
   info["Soil Moisture"] = logger.moist;

   String postLogger = JSON.stringify(info); // Converts the indices and values into a String

    /*Enter ESP to Cloud Code Here********
     */
     Serial.println(cloud(api, postLogger)); // Takes in the values and post them to database
     delay(100);
    /*
   */

  }

// WHEN NOTHING IS RECEIVED**********************************************
   Serial.print("Waiting for Data ...");
   while (!radio.available()){
    Serial.print(".");
    delay(rest_time);
    blinkLed(debug_led);              //Blink LED for default blink time

    
    /*Enter ESP to Cloud Code Here********
    
   */
   }

 /*Enter ESP to Cloud Code Here********
    
 */
}
