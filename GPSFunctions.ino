/***********************************
 * GPSFunctions
 ***********************************/

void gps_init (void)
{
  pinMode(GPS_PWR_STAT, INPUT);
  pinMode(GPS_PWR_ON, OUTPUT);
  
  if (digitalRead(GPS_PWR_STAT) == LOW)
  {
    digitalWrite(GPS_PWR_ON, HIGH);  
    delay(1000);
    digitalWrite(GPS_PWR_ON, LOW);  
    delay(2000);                  
  } 
}

boolean gps_feed(void)
{
  while (GPS.available())
  {
    if (gpsdata.encode(GPS.read()))
      return true;
  }
  return false;
}

void gps_getPosition (float *flat, float *flong)
{
  unsigned long age;
  unsigned long start = millis();
  boolean gotGPSfeed = false;

  // try to get GPS data in 60 seconds
  while ( gotGPSfeed == false  && ((millis() - start) < 60000) )
  {
    gotGPSfeed = gps_feed();
    if(gotGPSfeed == true)
    {
      gpsdata.f_get_position(flat, flong, &age);
      Serial.print("LAT: ");
      Serial.print(*flat, 10);
      Serial.print(" ");
      Serial.print("LONG: ");
      Serial.println(*flong, 10);
    }
  }
}
