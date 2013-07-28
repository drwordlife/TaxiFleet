#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <LiquidCrystal.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h> 

LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);
SoftwareSerial GSM(10, 11);
SoftwareSerial GPS(12, 13);
TinyGPS gpsdata;

#define INIT_TMRCNT_H     0xC2
#define INIT_TMRCNT_L     0xF7
#define TIME_INTERVAL     300

#define ENABLE_LOCFEED    TIMSK1 |= (1<<TOIE1)
#define DISABLE_LOCFEED   TIMSK1 &= ~(1<<TOIE1)
#define DISABLE_PBUTTON1  EIMSK &= ~(1<<INT0)
#define DISABLE_PBUTTON2  EIMSK &= ~(1<<INT1)

#define YES               1
#define NO                2

#define GSM_PWR_STAT      6
#define GSM_PWR_ON        7
#define GPS_PWR_STAT      8
#define GPS_PWR_ON        9

char server_num[] = "09164139553";
char Rx_data[500];
char customerAccepted = 0;
float latitude = TinyGPS::GPS_INVALID_F_ANGLE;
float longitude = TinyGPS::GPS_INVALID_F_ANGLE;
volatile int cnt = 1;
volatile int sendLocFlag = 0;
volatile int acceptCustomerFlag = 0;
volatile int pickedUpFlag = 0;
volatile int droppedOffFlag = 0;

//place timer ISR here to send text msg every 5 mins
ISR(TIMER1_OVF_vect) 
{  
  // Disable timer 1 interrupt
  DISABLE_LOCFEED;
  
  // Check if 5 min interval reached
  if (cnt == TIME_INTERVAL)
  {    
    Serial.println(millis());
    sendLocFlag = 1;
    
    // Reset counter
    cnt = 1;
  }
  else
  {
    cnt++;
  }

  // Reload Timer: Interrupt triggers every 1 second 
  TCNT1H = INIT_TMRCNT_H;
  TCNT1L = INIT_TMRCNT_L;

  // Enable timer 1 interrupt
  ENABLE_LOCFEED;
}

void acceptCustomer (void)
{
  DISABLE_PBUTTON1;
  DISABLE_PBUTTON2;
  acceptCustomerFlag = YES;
}

void rejectCustomer (void)
{
  DISABLE_PBUTTON1;
  DISABLE_PBUTTON2;
  Serial.println("reject customer!");
  acceptCustomerFlag = NO;
}

void pickedUpCustomer (void)
{
  DISABLE_PBUTTON1;
  DISABLE_PBUTTON2;
  pickedUpFlag = YES;
}

void notPickedUpCustomer (void)
{
  DISABLE_PBUTTON1;
  DISABLE_PBUTTON2;
  pickedUpFlag = NO;
}

void droppedOffCustomer (void)
{
  DISABLE_PBUTTON1;
  droppedOffFlag = 1;
}

void setTimer1 (void)
{
  //Timer1 Settings: Internal Timer, Prescaler: 1024
  TCCR1B |= ((1<<CS12) | (1<<CS10));
  TCCR1B &= ~(1<<CS11);
  // Use normal mode
  TCCR1A &= ~((1<<COM1A1) | (1<<COM1A0));
  TCCR1A &= ~((1<<COM1B1) | (1<<COM1B0));
  TCCR1A &= ~((1<<WGM11) | (1<<WGM10));
  TCCR1B &= ~((1<<WGM13) | (1<<WGM12));
  // Set Timer: Interrupt triggers every 1 second 
  TCNT1H = INIT_TMRCNT_H;
  TCNT1L = INIT_TMRCNT_L;  
  // Disable timer 1 interrupt
  DISABLE_LOCFEED;
}

void setup (void)
{ 
  Serial.begin(57600);
  GSM.begin(9600);
  GPS.begin(4800);
  lcd.begin(16,2);
  
  // Initializations
  Serial.println("Initialize...");
  lcd_clearThenPrint("Initialize...");
    
  gsm_init();
  gps_init();
  setTimer1();  
  
  Serial.println("Init Done!");  
  lcd_clearThenPrint("Init Done!");
}

void loop (void)
{
  char *txtmsg = "\0";
  char msg_buf[40], str_lat[15], str_long[15];
    
  // initialize Rx data, msg_buf
  tp_clearBuf(Rx_data, 500);
  tp_clearBuf(msg_buf, 40);
  GSM.listen();
  
  if (acceptCustomerFlag == YES)
  {
    Serial.println("accept customer!");
    sprintf(msg_buf, "CUSTOMERYES");
    gsm_sendMsg(server_num, msg_buf);
    
    acceptCustomerFlag = 0;
    customerAccepted = 1;
  }

  if (pickedUpFlag != 0)
  {
    if (pickedUpFlag == YES)
    {
      Serial.println("customer picked up!");
      sprintf(msg_buf, "PICKEDUP/%s", tp_custIDptr());
      delay(3000);
      attachInterrupt(0, droppedOffCustomer, LOW);
    }
    else if (pickedUpFlag == NO)
    {
      Serial.println("customer not picked up!");
      sprintf(msg_buf, "PICKEDUPNO");
    }
  
    gsm_sendMsg(server_num, msg_buf);
    pickedUpFlag = 0;
  }
  
  if (droppedOffFlag)
  {
    Serial.println("customer dropped off!");
    sprintf(msg_buf, "DROPPED/%s", tp_custIDptr());
    
    // send DROPPED/customerID
    gsm_sendMsg(server_num, msg_buf);
    
    pickedUpFlag = 0;
    droppedOffFlag = 0;
  }
  
  if (sendLocFlag)
  {
    Serial.println("sending location!");
    
    // get location
    GPS.listen();
    gps_getPosition(&latitude, &longitude);
    if (latitude == TinyGPS::GPS_INVALID_F_ANGLE || longitude == TinyGPS::GPS_INVALID_F_ANGLE)
    {
      Serial.println("ERROR: I'm sorry! I can't seem to get any GPS data at this moment.");
      lcd_clearThenPrint("No GPS data!");
    }
    else
    {
      // convert latitude and longitude float values to string
      dtostrf(double(latitude), 7, 4, str_lat);
      dtostrf(double(longitude), 7, 4, str_long);
      sprintf(msg_buf, "LOC/%s/%s", str_lat, str_long);
      
      // send LOC/lat/long
      GSM.listen();
      gsm_sendMsg(server_num, msg_buf);
    }
    sendLocFlag = 0;
  }
  
  if (GSM.available() > 0)
  {
    gsm_receiveFromUart(Rx_data);
    delay(3000); 
    
    if (pickedUpFlag != YES)
    { 
      // get keyword and process SMS
      txtmsg = tp_getTxtMsg(Rx_data);
      tp_processTxtMsg(txtmsg);
    }
    
    // initialize Rx data
    tp_clearBuf(Rx_data, 500);
  }
  
}
