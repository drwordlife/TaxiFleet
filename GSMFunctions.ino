/***********************************
 * GSMFunctions
 ***********************************/

void gsm_receiveFromUart (char* str)
{
  unsigned int i = 0;

  while(GSM.available() > 0)
  {
    str[i] = GSM.read();
    delay(50);
    i++;   
  }
}

void gsm_sendToUart (char* string)
{
  GSM.println(string);
  delay(90);
}

void gsm_powerOn (void)
{
  pinMode(GSM_PWR_STAT, INPUT);
  pinMode(GSM_PWR_ON, OUTPUT);
  
  if (digitalRead(GSM_PWR_STAT) == LOW)
  {
    digitalWrite(GSM_PWR_ON, HIGH);  
    delay(1000);
    digitalWrite(GSM_PWR_ON, LOW);  
    delay(2000);                  
  } 
}

void gsm_init (void)
{
  GSM.listen();
  gsm_powerOn();
  gsm_sendCmd("AT");
  gsm_sendCmd("ATE0");
  gsm_sendCmd("AT+CSQ");
  gsm_sendCmd("AT+CMGF=1");
  gsm_sendCmd("AT+CNMI=2,2,0,0,0");
  delay(500);
}

void gsm_sendCmd (char *at_cmd)
{
  char *stat = '\0';

  while(!stat){
    gsm_sendToUart(at_cmd);
    Serial.print(at_cmd);
    
    // wait until data is received from GSM
    while(GSM.available() <= 0);    
    
    gsm_receiveFromUart(Rx_data); 
    Serial.println(Rx_data);
    stat = strstr(Rx_data, "OK");
  }
  
  tp_clearBuf(Rx_data, 500);
}

void gsm_sendMsg (char *number, char *msg)
{
  char at_cmgs_cmd[30] = {'\0'};
  char msg1[160] = {'\0'};
  char ctl_z = 0x1A;

  sprintf(msg1, "%s%c", msg, ctl_z);
  sprintf(at_cmgs_cmd, "AT+CMGS=\"%s\"\r\n", number);

  // send "AT+CMGS = <number>" and receive data from GSM
  Serial.println(at_cmgs_cmd);
  gsm_sendToUart(at_cmgs_cmd);
  
  // wait until data is received from GSM and ignore received data
  while(GSM.available() <= 0);    
  gsm_receiveFromUart(Rx_data);
  tp_clearBuf(Rx_data, 500);
  delay(300);

  // send txtmsg and receive data from GSM  
  Serial.println(msg1);
  gsm_sendToUart(msg1);
  
  // wait until data is received from GSM and ignore received data
  while(GSM.available() <= 0);    
  gsm_receiveFromUart(Rx_data);
  tp_clearBuf(Rx_data, 500);
  delay(500);
}

void gsm_readMsg(int msg_no)
{
  char at_cmgr_cmd[15] = {'\0'};
  sprintf(at_cmgr_cmd, "AT+CMGR=%d,0\r\n", msg_no);
  gsm_sendToUart(at_cmgr_cmd);
  delay(100);
  gsm_receiveFromUart(Rx_data);  
  Serial.print(Rx_data);
}

