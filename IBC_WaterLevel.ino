#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

#define CLIENT_ADDRESS 2
#define SERVER_ADDRESS 1

#define RFM95_INT 3
#define RFM95_CS 10
#define RFM95_RST 2

#define trigPin 5
#define echoPin 6

#define TXPWR 23
#define FREQ 868.0

char Pstr[10];
char buffer[50];

RH_RF95 driver(RFM95_CS, RFM95_INT);

RHReliableDatagram manager(driver, CLIENT_ADDRESS);

void setup() 
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(115200);
  while (!Serial) {
    delay(1);
  }
  Serial.println("LoRa TX Test!");
  if (manager.init())
  {
    if (!driver.setFrequency(FREQ))
      Serial.println("Unable to set RF95 frequency");
//    if (!rf95.setModemConfig(RH_RF95::Bw500Cr45Sf128))
//      Serial.println("Invalid setModemConfig() option");
    driver.setTxPower(TXPWR, false);
    Serial.println("RF95 radio initialized.");
  }
  else
  {
    Serial.println("RF95 radio initialization failed.");
  }
  Serial.print("RF95 max message length = ");
  Serial.println(driver.maxMessageLength());
}

// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void loop()
{
  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;
  if (distance >= 200 || distance <= 0){
    Serial.println("Distance Out of range");
  }
  sprintf(buffer, "%d", distance);
  Serial.println(distance);
  if (manager.sendtoWait((uint8_t *) buffer, sizeof(buffer), SERVER_ADDRESS))
  {
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;   
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
    {
      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);
    }
    else
    {
      Serial.println("No reply, is rf95_reliable_datagram_server running?");
    }
  }
  else
  {
    Serial.println("sendtoWait failed");
  }
  delay(500);
}
