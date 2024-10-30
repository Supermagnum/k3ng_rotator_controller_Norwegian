#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
//define some pins
SoftwareSerial mySerial(3, 2); // RX, TX
Adafruit_GPS GPS(&mySerial);

// init the gps and set baud rate

void setup() {
  Serial.begin(115200);
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1000); // 1 Hz update rate
}

// update the time if its out of sync by more than 5 seconds:
void loop() {
  char c = GPS.read();
  if (GPS.newNMEAreceived()) {
    if (GPS.parse(GPS.lastNMEA())) {
      // Check if the GPS has a valid lock
      if (GPS.fix) {
        // Get the current time from the GPS
        uint8_t hour = GPS.hour, minute = GPS.minute, second = GPS.seconds;
        uint8_t day = GPS.day, month = GPS.month, year = GPS.year;

        // Calculate the time difference between the GPS and RTC
        time_t gpsTime = GPS.date + GPS.time;
        time_t rtcTime = RTC.get();
        long timeDifference = gpsTime - rtcTime;

        // If the time difference is greater than 5 seconds, update the RTC
        if (abs(timeDifference) > 5) {
          RTC.set(GPS.date, GPS.time);
        }

        // Print the current time to the serial monitor
        Serial.print("Current Time: ");
        Serial.print(hour, DEC); Serial.print(':');
        Serial.print(minute, DEC); Serial.print(':');
        Serial.print(second, DEC); Serial.print(" ");
        Serial.print(day, DEC); Serial.print('/');
        Serial.print(month, DEC); Serial.print('/');
        Serial.println(year, DEC);
      }
    }
  }
}
