/***********************************
 * LCDFunctions
 ***********************************/

void lcd_clearThenPrint (char* str)
{
  lcd.clear();
  lcd.print(str);
}

void lcd_setThenPrint (String str, char line)
{
  lcd.setCursor(0,line);
  lcd.print(str);
}

void lcd_scroll (unsigned int scrollCnt)
{
  for (int positionCounter = 0; positionCounter < scrollCnt; positionCounter++) {
    // scroll one position left:
    lcd.scrollDisplayLeft(); 
    // wait a bit:
    delay(500);
  }

  // scroll 29 positions (string length + display length) to the right
  // to move it offscreen right:
  for (int positionCounter = 0; positionCounter < scrollCnt; positionCounter++) {
    // scroll one position right:
    lcd.scrollDisplayRight(); 
    // wait a bit:
    delay(500);
  }
}

void lcd_displayInfo (char* str)
{
  String s = String(str);
  int count = 0;
  int endstr = s.length()-2;
  
  // find '/' and print address/customerID in 2nd line of LCD
  count = s.indexOf('/');
  
  lcd_setThenPrint(s.substring(0,count), 0);
  lcd_setThenPrint(s.substring(count+1, endstr), 1);
}
