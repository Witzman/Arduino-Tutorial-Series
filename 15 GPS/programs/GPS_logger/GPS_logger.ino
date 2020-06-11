/*
  Project: NEO-6M GY-GPS6MV2 GPS module, SD card or Micro SD card
  Function: This sketch listen to the GPS serial port,
  and when data received from the module, it's also sent to the serial monitor and saved to SD card or Micro SD card.

  Wiring GPS module:
  GPS module -> Arduino Uno
  GND           GND
  RX pin        Digital pin 3
  TX pin        Digital pin 4
  VCC pin       5V

  Wiring SD card module:
  SD card module -> Arduino Uno
  VCC           3.3V or 5V (check moduleâ€™s datasheet)
  CS            Digital pin 10. This can be the hardware SS pin - pin 10 (on most Arduino boards) or pin 53 (on the Mega) - or another pin specified in the call to SD.begin(). N
              ote that even if you don't use the hardware SS pin, it must be left as an output or the SD library won't work. Different boards use different
              pins for this functionality, so be sure youâ€™ve selected the correct pin in SD.begin().
  MOSI          Digital pin 11
  CLK           Digital pin 13
  MISO          Digital pin 12
  GND           GND

  Note: different Arduino boards have different SPI pins. If youâ€™re using another Arduino board, check the Arduino SPI documentation.
*/
//************************************************************
#include <SoftwareSerial.h>//include library code to allow serial communication on other digital pins of the Arduino board
#include <SPI.h>//include library code to communicate with SPI devices
#include <SD.h>//include library code for SD card
#include <TinyGPS++.h>//include the library code for GPS module
//***********************************************************
TinyGPSPlus gps;// The TinyGPS++ object
SoftwareSerial GPSModule(6, 7);// The serial connection to the GPS device
const int chipSelect = 4;
//*************************************************************
void setup()
{
  Serial.begin(9600);
  GPSModule.begin(9600);
  Serial.print("Initializing SD card...");//setup for the SD card
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present!");
    return;
  }
  Serial.println("Card initialized.");
  Serial.println("File opened. Start logging GPS data to SD card:");
  File dataFile = SD.open("gpsdata.csv", FILE_WRITE); //opens file
  if (dataFile) { //if the file opened ok, writes to it
    dataFile.println("Date, Time , Lat , Long , Sats , HDOP , Alt, Speed");//prints the headings for our data
  }
  dataFile.close();//file closed
  Serial.println(F("Sats HDOP Latitude   Longitude   Fix  Date       Time     Date Alt    Course Speed Card  Chars Sentences Checksum"));
  Serial.println(F("          (deg)      (deg)       Age                      Age  (m)    --- from GPS ----    RX    RX        Fail"));
  Serial.println(F("-----------------------------------------------------------------------------------------------------------------"));

}

void loop()
{
  printInt(gps.satellites.value(), gps.satellites.isValid(), 5);
  printInt(gps.hdop.value(), gps.hdop.isValid(), 5);
  printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
  printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
  printInt(gps.location.age(), gps.location.isValid(), 5);
  printDateTime(gps.date, gps.time);
  printFloat(gps.altitude.meters(), gps.altitude.isValid(), 7, 2);
  printFloat(gps.course.deg(), gps.course.isValid(), 7, 2);
  printFloat(gps.speed.kmph(), gps.speed.isValid(), 6, 2);
  printStr(gps.course.isValid() ? TinyGPSPlus::cardinal(gps.course.deg()) : "*** ", 6);
  printInt(gps.charsProcessed(), true, 6);
  printInt(gps.sentencesWithFix(), true, 10);
  printInt(gps.failedChecksum(), true, 9);
  Serial.println();
  //***************************************************************************
  // The routine for writing data to SD card:
     File dataFile = SD.open("gpsdata.csv", FILE_WRITE);


    if (dataFile) {// if the file is available, write to it:
     
    dataFile.print(gps.date.day());
    dataFile.print(F("."));
    dataFile.print(gps.date.month());
    dataFile.print(F("."));
    dataFile.print(gps.date.year());
     dataFile.print(",");   
    dataFile.print(gps.time.hour());
    dataFile.print(F(":"));
    dataFile.print(gps.time.minute());
    dataFile.print(F(":"));
    dataFile.print(gps.time.second());
    
    dataFile.print(",");   
     dataFile.print(gps.location.lat(),6);
     dataFile.print(",");
     dataFile.print(gps.location.lng(),6);
     dataFile.print(",");
     dataFile.print(gps.satellites.value(),5);
     dataFile.print(",");
     dataFile.print(gps.hdop.value(),5);
     dataFile.print(",");
     dataFile.print(gps.altitude.meters());
     dataFile.print(",");
     dataFile.print(gps.speed.kmph());
     dataFile.print("\n");
     dataFile.close();
    }

  //***************************************************************
  smartDelay(1000);

  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));
}

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (GPSModule.available())
      gps.encode(GPSModule.read());
  } while (millis() - start < ms);
}

static void printFloat(float val, bool valid, int len, int prec)
{
  if (!valid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i = flen; i < len; ++i)
      Serial.print(' ');
  }
  smartDelay(0);
}

static void printInt(unsigned long val, bool valid, int len)
{
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i = strlen(sz); i < len; ++i)
    sz[i] = ' ';
  if (len > 0)
    sz[len - 1] = ' ';
  Serial.print(sz);
  smartDelay(0);
}

static void printDateTime(TinyGPSDate &d, TinyGPSTime &t)
{
  if (!d.isValid())
  {
    Serial.print(F("********** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    Serial.print(sz);
  }

  if (!t.isValid())
  {
    Serial.print(F("******** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    Serial.print(sz);
  }

  printInt(d.age(), d.isValid(), 5);
  smartDelay(0);
}

static void printStr(const char *str, int len)
{
  int slen = strlen(str);
  for (int i = 0; i < len; ++i)
    Serial.print(i < slen ? str[i] : ' ');
  smartDelay(0);
}
