/***********************************
 * SMSProcessing
 ***********************************/

char* customerID;

char* tp_getTxtMsg (char* s)
{
  char* txtMsg;
  
  txtMsg = strstr(s, "\"\r\n");
  return (txtMsg + 3);
}

char* tp_getCustID (char *s)
{
  String str = String(s);
  int count = 0;
  
  count = str.lastIndexOf('/');
  return s+(count+1);
}

char* tp_custIDptr (void)
{
  return customerID;
}

void tp_processTxtMsg (char *s)
{
  float latitude = TinyGPS::GPS_INVALID_F_ANGLE;
  float longitude = TinyGPS::GPS_INVALID_F_ANGLE;
  char msg_buf[20];
  String txtMsg = "";
  long start;
  
  if (strstr(s,"LOCFEEDSTART"))      // Determine if sending auto text msgs is enabled
  {
    Serial.println("Start sending!"); 
    ENABLE_LOCFEED;
  }
  else if (strstr(s,"LOCFEEDSTOP"))  // Determine if to disable constant send location
  {
    Serial.println("Stop sending!");
    DISABLE_LOCFEED;
  }
  else if (strstr(s,"CUSTOMER"))     // Determine if location request from server
  {
    Serial.println("Customer!");
    txtMsg = String(s+9);
    Serial.println(txtMsg);
        
    attachInterrupt(0, acceptCustomer, LOW);
    attachInterrupt(1, rejectCustomer, LOW);
        
    // clear previous customer ID    
    tp_clearBuf(customerID, 2);
    
    // get customer ID
    customerID = tp_getCustID(s+9);

    // print customer info on LCD
    lcd_clearThenPrint("");
    lcd_displayInfo(s+9);    

    // scroll for 60 secs
    start = millis();
    while (millis() - start < 60000)
    {
      lcd_scroll(txtMsg.length());
    }
    
    lcd_clearThenPrint("");    
    lcd_displayInfo(s+9);    
  }
  else if (strstr(s,"CODE"))     // Determine if location request from server
  {
    Serial.println("Code!");
    Serial.println(s+5);
    
    lcd_clearThenPrint(s+5);
    if (customerAccepted)
    {
      attachInterrupt(0, pickedUpCustomer, LOW);
      attachInterrupt(1, notPickedUpCustomer, LOW);
      customerAccepted = 0;
    }
    
    sprintf(msg_buf, "CODE: %s", s+5);
    lcd_clearThenPrint("");
    
  }
  else				     // Unknown message
  {
    // ignore message
    Serial.println("Invalid msg!");
  }
}

void tp_clearBuf (char *str, int len)
{
  int i;
  
  for (i=0; i<len; i++)
    str[i] = '\0';
}



