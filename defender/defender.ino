#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <SPI.h>
/*
 * ethernet contoller steup! we will eventually DHCP for getting an IP address, for testing I will be using manual IP
 * See best practices in setting up to get things working right with the server and knowing what readers are where
 */

//IP manual settings
//uncomment this for testing or if you really feel like using a manual IP
byte ip[] = { 10, 0, 1, 100 };
byte gateway[] = { 10, 0, 1, 1 };
byte subnet[] = { 255, 255, 0, 0 };

//if you need to change the MAC address do this (replace with mac of your sheild)
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xEF, 0xDE };

//HTTP Port (Default at 80) may change at some point for "security"
EthernetServer server = EthernetServer(80);

//Number of outputs to switch (number of doors on controller)
int strikeQuantity = 1;

//pin starting from
int outputLowest = 8;

/*
 * The following has been adapted from: 
 * HID RFID Reader Wiegand Interface for Arduino Uno
 * Written by Daniel Smith, 2012.01.30
 * www.pagemac.com 
 * 
 * This portion of the software is what is reading the RFID tag.
 * This is just a starting point the next additions will be sending it to some kind of database for access
*/
 
#define doorPin 5
#define redPin  6
#define beepPin 7
#define greenPin 4 
#define MAX_BITS 100                 // max number of bits 
#define WEIGAND_WAIT_TIME  3000      // time to wait for another weigand pulse.  
 
unsigned char databits[MAX_BITS];    // stores all of the data bits
unsigned char bitCount;              // number of bits currently captured
unsigned char flagDone;              // goes low when data is currently being captured
unsigned int weigand_counter;        // countdown until we assume there are no more bits
 
unsigned long facilityCode=0;        // decoded facility code
unsigned long cardCode=0;            // decoded card code
 
// interrupt that happens when INTO goes low (0 bit)
void ISR_INT0()
{
  //Serial.print("0");   // uncomment this line to display raw binary
  bitCount++;
  flagDone = 0;
  weigand_counter = WEIGAND_WAIT_TIME;  
 
}
 
// interrupt that happens when INT1 goes low (1 bit)
void ISR_INT1()
{
  //Serial.print("1");   // uncomment this line to display raw binary
  databits[bitCount] = 1;
  bitCount++;
  flagDone = 0;
  weigand_counter = WEIGAND_WAIT_TIME;  
}
 
void setup()
{
  pinMode(4, OUTPUT);  // LED
  pinMode(2, INPUT);     // DATA0 (INT0)
  pinMode(3, INPUT);     // DATA1 (INT1)
 
  Serial.begin(9600);
  Serial.println("RFID Readers");
 
  // binds the ISR functions to the falling edge of INTO and INT1
  attachInterrupt(0, ISR_INT0, FALLING);  
  attachInterrupt(1, ISR_INT1, FALLING);
  //connect to door strike realy (update this eventually with relay pin, or define it)
  pinMode(doorPin, OUTPUT);
  Ethernet.begin(mac, ip, gateway, subnet);
  server.begin();
  Serial.print("Defender is at ");
  Serial.println(Ethernet.localIP());
 
 
  weigand_counter = WEIGAND_WAIT_TIME;
}
 
void loop()
{
  // This waits to make sure that there have been no more data pulses before processing data
  if (!flagDone) {
    if (--weigand_counter == 0)
      flagDone = 1;  
  }
 
  // if we have bits and we the weigand counter went out
  if (bitCount > 0 && flagDone) {
    unsigned char i;
 
    Serial.print("Read ");
    Serial.print(bitCount);
    Serial.print(" bits. ");
 
    // we will decode the bits differently depending on how many bits we have
    // see www.pagemac.com/azure/data_formats.php for mor info
    if (bitCount == 35)
    {
      // 35 bit HID Corporate 1000 format
      // facility code = bits 2 to 14
      for (i=2; i<14; i++)
      {
         facilityCode <<=1;
         facilityCode |= databits[i];
      }
 
      // card code = bits 15 to 34
      for (i=14; i<34; i++)
      {
         cardCode <<=1;
         cardCode |= databits[i];
      }
 
      printBits();
    }
    else if (bitCount == 26)
    {
      // standard 26 bit format
      // facility code = bits 2 to 9
      for (i=1; i<9; i++)
      {
         facilityCode <<=1;
         facilityCode |= databits[i];
      }
 
      // card code = bits 10 to 23
      for (i=9; i<25; i++)
      {
         cardCode <<=1;
         cardCode |= databits[i];
      }
 
      printBits();  
    }
    else {
      // you can add other formats if you want!
     Serial.println("Unable to decode."); 
    }
 
     // cleanup and get ready for the next card
     bitCount = 0;
     facilityCode = 0;
     cardCode = 0;
     for (i=0; i<MAX_BITS; i++) 
     {
       databits[i] = 0;
     }
  }
}
 
void printBits()
{
      // I really hope you can figure out what this function does
      Serial.print("FC = ");
      Serial.print(facilityCode);
      Serial.print(", CC = ");
      Serial.println(cardCode); 
}

// Opens door and turns on the green LED for setDelay seconds + 2 short beeps 
void openDoor( int setDelay )
{
  setDelay *= 1000; // Sets delay in seconds

  digitalWrite(doorPin, HIGH);  // Unlock door!
  
  delay(setDelay); // Hold door lock open for 5 seconds
  digitalWrite(beepPin, HIGH); //start beep
  digitalWrite(greenPin, HIGH); //start LED flash
  delay(200);
  digitalWrite(greenPin, LOW);
  digitalWrite(beepPin, LOW);
  delay(200);
  digitalWrite(greenPin, HIGH);
  digitalWrite(beepPin, HIGH); //beep a few times
  delay(200);
  digitalWrite(greenPin, LOW);//stop LED flash
  digitalWrite(beepPin, LOW); //stop beep
  
  digitalWrite(doorPin, LOW); // Relock door
}

// Flashes Red LED + Beep long if failed login
void failed()
{
     
  // Blink red fail LED 3 times to indicate failed key
  digitalWrite(beepPin, HIGH); //start beep
  delay(1000);
  digitalWrite(beepPin, LOW); //stop beep
  digitalWrite(redPin, HIGH);  // Turn on red LED
  delay(500);
  digitalWrite(redPin, LOW);   // Turn off red LED
      
  digitalWrite(redPin, HIGH);  // Turn on red LED
  delay(500);
  digitalWrite(redPin, LOW);   // Turn off red LED
      
  digitalWrite(redPin, HIGH);  // Turn on red LED
  delay(500);
  digitalWrite(redPin, LOW);   // Turn off red LED
}

