#include <TimeLib.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <DS1307RTC.h>  // a basic RTC library that returns time as a time_t

// NTP Servers:
static const char ntpServerName[] = "pool.ntp.org";

EthernetUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);

void setup() {
  // Initialize Ethernet
  byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
  Ethernet.begin(mac);

  // Set the time zone to UTC (Zulu Time)
  setTimeZone(0);

  // Initialize serial communication
  Serial.begin(9600);
  while (!Serial) ; // wait for serial port to connect.
}

void loop() {
  time_t now = getNtpTime();

  if (now != 0) {
    digitalClockDisplay();
    // Update the RTC clock with the current time
    RTC.set(now);
  }

  delay(1000);
}

void digitalClockDisplay() {
  // Get the current time
  time_t now = time(nullptr);

  // Print the current time in Zulu (UTC) time
  Serial.print(hour(now));
  printDigits(minute(now));
  printDigits(second(now));
  Serial.print(" ");
  Serial.print(day(now));
  Serial.print(" ");
  Serial.print(month(now));
  Serial.print(" ");
  Serial.print(year(now));
  Serial.println();
}

void printDigits(int digits) {
  // Print a leading zero for single-digit numbers
  if (digits < 10) {
    Serial.print('0');
  }
  Serial.print(digits);
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime() {
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // Get a random server from the pool
  Ethernet.hostByName(ntpServerName, ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form the NTP request
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  // 8 bytes of zero for Reference ID
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a time stamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
