##This code is simplified as all serial communication lines have been removed.

//Librarys------------------------- 
#include <Arduino_MKRENV.h> //MKR ENV shield
#include <Seeed_HM330X.h> //dust sensor
#include <Wire.h> //I2C connection
#include <ArduinoLowPower.h> //SleepLibrary
#include <SD.h> //SD-Shield
#include <SPI.h>
#include "DHT.h" //Temperature and Humidity Sensor

//sensor variables--------------------------

  float h2;
  float h2_1;
  float h2_2;
  float h2_3;
  float h2_4;
  float h2_5;
  
  float humidity;
  float humidity_1;
  float humidity_2;
  float humidity_3;
  float humidity_4;
  float humidity_5;
  
  float pressure;
  float pressure_1;
  float pressure_2;
  float pressure_3;
  float pressure_4; 
  float pressure_5;
  
  float t2;
  float t2_1;
  float t2_2;
  float t2_3;
  float t2_4;
  float t2_5;
  
  float temperature;
  float temperature_1;
  float temperature_2;
  float temperature_3;
  float temperature_4;
  float temperature_5;
  
  int PM10;
  int PM25;
  
  int uva;
  int uvb;
  int lux;


//Time---------------------------------------
#include "DS1307.h" //RTC
DS1307 clock;//define a object of DS1307 class
int year;
int month;
int day;
int hour;
int minute;
int second;

//Temp and Humi Sensor DHT11---------------------------
#define DHTPIN 5  
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE, 30);


// chip select for SD card----------------------------
const int SD_CS_PIN = 4;  

//workaround for PM---------------------------------
int PM25_value;
int PM25_value1;
int PM25_value2;
int PM25_value3;
int PM25_value4;

int PM10_value;
int PM10_value1;
int PM10_value2;
int PM10_value3;
int PM10_value4;

//SD File object-------------------------------------
File myFile;
char filename[16];
int t=0;
int n=0;
//deepSleep counter---------------------------------
int dS=0;

///SAMD Serial structure------------------------------
#ifdef  ARDUINO_SAMD_VARIANT_COMPLIANCE
#define SERIAL_OUTPUT SerialUSB
#else
#define SERIAL_OUTPUT Serial
#endif

//Definitions of HM3301 Laser Dust Sensor----------------
HM330X hm3301;
u8 buf[30];

HM330XErrorCode parse_PM25(u8 *data)
{
  u16 PM25_value = 0;
  u16 PM25_value1 = 0;
  u16 PM25_value2 = 0;
  u16 PM25_value3 = 0;
  u16 PM25_value4 = 0;
  
  if (NULL == data)
    return ERROR_PARAM;
  
    PM25_value = (u16)data[3 * 2] << 5 | data[3 * 2 + 1]; 
    PM25_value1 = (u16)data[3 * 2] << 5 | data[3 * 2 + 1]; 
    PM25_value2 = (u16)data[3 * 2] << 5 | data[3 * 2 + 1]; 
    PM25_value3 = (u16)data[3 * 2] << 5 | data[3 * 2 + 1]; 
    PM25_value4 = (u16)data[3 * 2] << 5 | data[3 * 2 + 1]; 
    PM25 = int((PM25_value+PM25_value1+PM25_value2+PM25_value3+PM25_value4)/5);

  return NO_ERROR;
}

HM330XErrorCode parse_PM10(u8 *data)
{
  u16 PM10_value = 0;
  u16 PM10_value1 = 0;
  u16 PM10_value2 = 0;
  u16 PM10_value3 = 0;
  u16 PM10_value4 = 0;
  if (NULL == data)
    return ERROR_PARAM;
  
    PM10_value = (u16)data[4 * 2] << 5 | data[4 * 2 + 1]; 
    PM10_value1 = (u16)data[4 * 2] << 5 | data[4 * 2 + 1]; 
    PM10_value2 = (u16)data[4 * 2] << 5 | data[4 * 2 + 1]; 
    PM10_value3 = (u16)data[4 * 2] << 5 | data[4 * 2 + 1]; 
    PM10_value4 = (u16)data[4 * 2] << 5 | data[4 * 2 + 1]; 
    PM10 = int((PM10_value+PM10_value1+PM10_value2+PM10_value3+PM10_value4)/5);
    
   

  return NO_ERROR;
}

//Setup------------------------------------------------------

void setup() {
 // Initialize SERIAL_OUTPUT and wait for port to open:
  SERIAL_OUTPUT.begin(9600);
  
  
  //SD Card-------------------------------------------------------
  // init SPI
  SPI.begin();
  delay(10000);
  
  if(!SD.begin(SD_CS_PIN)) {
    SERIAL_OUTPUT.println("Failed to initialize SD card!");
    while (1);
  }
  snprintf(filename, sizeof(filename), "data%03d.txt", n); // includes a three-digit sequence number in the file name
    myFile = SD.open(filename, FILE_READ);
  SERIAL_OUTPUT.print("n =");
  SERIAL_OUTPUT.println(n);
  SERIAL_OUTPUT.print("Filename = ");
  SERIAL_OUTPUT.println(filename);
   myFile.close();
  myFile = SD.open(filename, FILE_WRITE);
  myFile.print("Time,      Humi_top, Temp_top, Humi_ENV,  press_ENV,  Temp_ENV,  lux,  uva, uvb,  PM25, PM10, Identifier "); 
  myFile.println(" ");
  myFile.close();


  
//Dust Sensor------------------------------
    if(hm3301.init()){
      SERIAL_OUTPUT.println("Failed to initialize Dust Sensor HM3301!");
      while(1);
    }
//ENV Shield-------------------------------
    if (!ENV.begin()) {
      SERIAL_OUTPUT.println("Failed to initialize MKR ENV shield!");
      while (1);
    }

//DHT initiate------------------------------
  dht.begin();
  
//DS1307 Real Time Clock initiate----------------------------------
  clock.begin();


} //Setup End



//LOOP---------------------------------------------------------
void loop() {
  

//couting the cycles for better data storage
  t++;


//Check HM3301 Dust Sensor for errors
    if (hm3301.read_sensor_value(buf, 29))
  {
    SERIAL_OUTPUT.println("HM330X read result failed!!!");
  }
  
 
 //print the Time in PosiX format (POSIXct(format = "%Y-%m-%d %H:%M:%S "))----------------------------------------------------

 myFile = SD.open(filename, FILE_WRITE);
 clock.getTime();
int year = (clock.year+2000); 
myFile.print(clock.year);
    myFile.print("-");
    
int month = (clock.month);
if(month <= 9) {
      myFile.print("0");
      myFile.print(month);
    }
    else { myFile.print(month);    }
    
int day = (clock.dayOfMonth);
myFile.print("-");
if(day <= 9) {
      myFile.print("0");
      myFile.print(day);
 }
 else {myFile.print(day);}

int hour = (clock.hour);
  myFile.print(" ");
  if(hour <= 9) {
      myFile.print("0");
      myFile.print(hour);
  }
  else {myFile.print(hour);}
  
int minute = (clock.minute);
myFile.print(":");
if(minute <= 9) {
      myFile.print("0");
      myFile.print(minute);
  }
  else {myFile.print(minute);}
  
int second = (clock.second);
myFile.print(":");
if(second <= 9) {
      myFile.print("0");
      myFile.print(second);
  }
  else {myFile.print(second);}

    myFile.print(",  ");
    myFile.close();
    
//------------------------------------------------
 
 //Sensoric Data from ENV Shield---------------------
 //humidity-----------------
 humidity_1 = float(ENV.readHumidity());
 delay(100);
 humidity_2 = float(ENV.readHumidity());
 delay(100);
 humidity_3 = float(ENV.readHumidity());
 delay(100);
 humidity_4 = float(ENV.readHumidity());
 delay(100);
 humidity_5 = float(ENV.readHumidity());
 humidity =  (humidity_1 + humidity_2 + humidity_3 + humidity_4 + humidity_5)/5;
 
 //pressure-----------------
 pressure_1 = float(ENV.readPressure());
 delay(100);
 pressure_2 = float(ENV.readPressure());
 delay(100);
 pressure_3 = float(ENV.readPressure());
 delay(100);
 pressure_4 = float(ENV.readPressure()); 
 delay(100);
 pressure_5 = float(ENV.readPressure());
 pressure =  (pressure_1 + pressure_2 + pressure_3 +pressure_4 + pressure_5)/5; 
 
 //temperature---------------
 temperature_1 = float(ENV.readTemperature());
 delay(100);
 temperature_2 = float(ENV.readTemperature());
 delay(100);
 temperature_3 = float(ENV.readTemperature());
 delay(100);
 temperature_4 = float(ENV.readTemperature());
 delay(100);
 temperature_5 = float(ENV.readTemperature());
 delay(100);
 temperature = (temperature_1+temperature_2+temperature_3+temperature_4+temperature_5)/5;
 
 //light---------------------
 uva = int(ENV.readUVA());
 uvb = int(ENV.readUVB());
 lux = int(ENV.readIlluminance());
 
 //Other Sensors---------------------------
 //particulate matter------------
 parse_PM25(buf);
 parse_PM10(buf);
 
 //Humidity and Temperature-----------
 temphumi(); //saves the DHT11 temp & humidity in extra void
 
    
 //save to SD-----------------------------
 myFile = SD.open(filename, FILE_WRITE);
 myFile.print(",    ");
 myFile.print(humidity);
 myFile.print(",     ");
 myFile.print(pressure);
 myFile.print(",     ");
 myFile.print(temperature);
 myFile.print(",     ");
 myFile.print(lux);
 myFile.print(",     ");
 myFile.print(uva);
 myFile.print(",     ");
 myFile.print(uvb);
 myFile.print(",     ");
 myFile.print(PM25);
 myFile.print(",     ");
 myFile.print(PM10);
 myFile.print(",     ");
 myFile.print("C1");  //Indentifier 
 myFile.println(" ");
 myFile.close();

//Counter for File Management-----------------------
  if(t>100){
   while(SD.exists(filename)) {
    n++;
    snprintf(filename, sizeof(filename), "data%03d.txt", n);
     }
  t = 0;
  }


//Counter for Sleep Management----------------------
 if(dS>5){LowPower.deepSleep(60000);
 dS = 0;
 }
 else {delay(5000);
 dS++;
 }
 
} //Loop End



//Temp&Humidity DHT11 Void-------------------
void temphumi()
 {
    float h2_1 = dht.readHumidity();
    delay(100);
    float h2_2 = dht.readHumidity();
    delay(100);
    float h2_3 = dht.readHumidity();
    delay(100);
    float h2_4 = dht.readHumidity();
    delay(100);
    float h2_5 = dht.readHumidity();
    float h2 = (h2_1+h2_2+h2_3+h2_4+h2_5)/5;
    
    float t2_1 = dht.readTemperature();
    delay(100);
    float t2_2 = dht.readTemperature();
    delay(100);
    float t2_3 = dht.readTemperature();
    delay(100);
    float t2_4 = dht.readTemperature();
    delay(100);
    float t2_5 = dht.readTemperature();
    float t2 = (t2_1+t2_2+t2_3+t2_4+t2_5)/5;
    
  // Check if any reads failed and exit early (to try again).
  if (isnan(h2) || isnan(t2) ) {
    SERIAL_OUTPUT.println("Failed to read from DHT sensor!");
    return;
  }

//Store DHT11 data to SD card--------------------
  myFile = SD.open(filename, FILE_WRITE);
  myFile.print(h2);
  myFile.print(" ,    ");
  myFile.print(t2);
  myFile.print("      ");
  myFile.close();
  
} //DHT Void End














