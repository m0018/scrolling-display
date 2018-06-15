
/************************* Necessary Libraries *********************************/
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <EEPROM.h>
#define SCROLL_DELAY 75

/************************* Variables *********************************/
char* str;
String inputString;
String payload;
uint32_t present;
bool first_time;
uint16_t  scrollDelay;  // in milliseconds
#define  CHAR_SPACING  1 // pixels between characters
int addr = 50;
int check = 0;
int stringComplete=1;

// Global message buffers shared by Serial and Scrolling functions
#define BUF_SIZE  75
char curMessage[BUF_SIZE];
char newMessage[BUF_SIZE];
bool newMessageAvailable = false;


// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define MAX_DEVICES 8

/************************* Matrix Display Pins *********************************/
#define CLK_PIN   13 // or SCK
#define DATA_PIN  11 // or MOSI
#define CS_PIN    10 // or SS


// SPI hardware interface
MD_MAX72XX mx = MD_MAX72XX(CS_PIN, MAX_DEVICES);
// Arbitrary pins
//MD_MAX72XX mx = MD_MAX72XX(DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);


uint8_t scrollDataSource(uint8_t dev, MD_MAX72XX::transformType_t t)
// Callback function for data that is required for scrolling into the display
{
  static char   *p = curMessage;
  static uint8_t  state = 0;
  static uint8_t  curLen, showLen;
  static uint8_t  cBuf[8];
  uint8_t colData;

  // finite state machine to control what we do on the callback
  switch (state)
  {
    case 0: // Load the next character from the font table
      showLen = mx.getChar(*p++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);
      curLen = 0;
      state++;

      // if we reached end of message, reset the message pointer
      if (*p == '\0')
      {
        p = curMessage;     // reset the pointer to start of message
        if (newMessageAvailable)  // there is a new message waiting
        {
          strcpy(curMessage, str);  // copy it in
          newMessageAvailable = false;
        }
      }
    // !! deliberately fall through to next state to start displaying

    case 1: // display the next part of the character
      colData = cBuf[curLen++];
      if (curLen == showLen)
      {
        showLen = CHAR_SPACING;
        curLen = 0;
        state = 2;
      }
      break;

    case 2: // display inter-character spacing (blank column)
      colData = 0;
      curLen++;
      if (curLen == showLen)
        state = 0;
      break;

    default:
      state = 0;
  }

  return (colData);
}

void scrollText(void)
{
  static uint32_t prevTime = 0;
  if (millis() - prevTime >= scrollDelay)
  {
    mx.transform(MD_MAX72XX::TSL);  // scroll along - the callback will load all the data
    prevTime = millis();      // starting point for next time
  }
}

void setup()
{
  Serial.begin(9600);
//  EEPROM.begin();      
  mx.begin();
  delay(5000);
  strcpy(curMessage, "Hello! ");
  mx.setShiftDataInCallback(scrollDataSource);

  scrollDelay = SCROLL_DELAY;
  newMessage[0] = '\0';
  delay(3000);
  Serial.print("\n[MD_MAX72XX Message Display]\nType a message for the scrolling display\nEnd message line with a newline");


 // check = EEPROM.read(addr);
  Serial.println(check);

 /* if(check==0)
  {
      EEPROM.write(addr,10);
     // EEPROM.commit();
     //ESP.restart();
  }
  
  else
  {
      EEPROM.write(addr,0);
     // EEPROM.commit();
  }   */




  

  newMessageAvailable = 1;
  present = millis();
  first_time = 1;
  inputString = "   MATT ENGG EQUIPMENTS   ";


}

void loop()
{
  
  if(stringComplete == 1)
  {
      payload ="";
      payload = inputString;
      payload += "       ";
      str = &payload[0];      
      newMessageAvailable = 1; 
      stringComplete = 0;
      inputString = "";
  }
  serialEvent();
  scrollText();
}
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
   
    if (inChar == '\n') 
    {
      stringComplete = 1;
      //Serial.print(inputString);
    }
    else
    {
       inputString += inChar;
    }
  }
}
