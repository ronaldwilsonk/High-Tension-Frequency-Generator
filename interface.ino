/* Interface Programming
 * 
 * Pinmap for LCD
 * 12 Register Select (RS)
 * 11 Read/Write (RW)
 * 10 Enable (E)
 * 5 D4
 * 4 D5
 * 3 D6
 * 2 D7
 * 
 * Usage for carraige return: Single press enables system. Two consecutive presses shuts down system.
 * 
 * Functions:
 * void LCD_init(int mode): Mode 0 prints startup screen
 *                          Mode 1 prints system running warning message
 *                          Mode 2 prints ending screen
 *                          Mode 3 invalid inputs given
 * void LCD_setValue(float f, int v): Rewrites new voltage and frequency values to screen 
 * 
 * Class inputSense
 * void freqShiftUp(): Shifts frequency up by 0.5 Hz
 * void freqShiftDown(): Shifts frequency down by 0.5 Hz
 * void volShiftUp(): Shifts voltage up by 250 V
 * void volShiftDown(): Shifts voltage down by 250 V
 * void reset(): Resets voltage and frequency to 0
 * void setSystemStatusCode(int code): Sets system status code
 *                                     Code 0: First Initialization
 *                                     Code 1: Running
 *                                     Code 2: Stopped
 * int getSystemStatusCode(): Returns system status code
 * int dispValues(int mode): Returns current voltage and frequency values
 *                           Mode 0: Frequency
 *                           Mode 1: Voltage
 * void writeToSerial() : Writes encoded frequency and voltage to serial. 
 *                        Encoding: 1 -> 250V   2->500V         
 *                                  1 -> 0.5Hz  2->1Hz
 * void readFromSerial(): Updates real time frequency and voltage parameters on screen
 *                        Encoding: 
 */

const int freqShiftUpPin=A0;
const int freqShiftDownPin=A1;
const int volShiftUpPin=A2;
const int volShiftDownPin=A3;
const int carraigeReturnSense=A4;

#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 10, 5, 4, 3, 2);

void LCD_init(int mode)      
{
  lcd.clear();
  if(mode==0)
  {
    lcd.setCursor(0, 1);
    lcd.print("Initializing System");
    lcd.setCursor(0, 2);
    lcd.print("Please Wait");
  }
  else if(mode==1)
  {
    lcd.setCursor(0, 1);
    lcd.print("ATTENTION");
    lcd.setCursor(0, 2);
    lcd.print("System Live");
  }
  else if(mode==2)
  {
    lcd.setCursor(0, 1);
    lcd.print("Killing Output");
    lcd.setCursor(0, 2);
    lcd.print("Please Wait");
  }
  else if(mode==3)
  {
    lcd.setCursor(0, 1);
    lcd.print("Invalid value set");
    lcd.setCursor(0, 2);
    lcd.print("Please check setting");
  }
  delay(2000);
}

void LCD_setValue(float f, int v)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Frequency [Hz]");
  lcd.setCursor(0, 1);
  lcd.print(f,DEC);
  lcd.setCursor(4, 1);
  lcd.print("           ");
  lcd.setCursor(0, 2);
  lcd.print("Set Voltage [V]");
  lcd.setCursor(0, 3);
  lcd.print(v,DEC);
}

class inputSense
{
  private:
  int voltageLevel=0;
  float freqLevel=0;
  int systemStatusCode=0;
  public:
  void freqShiftUp();
  void freqShiftDown();
  void volShiftUp();
  void volShiftDown();
  void setSystemStatusCode(int code);
  int getSystemStatusCode();
  float dispValues(int mode);
  void writeToSerial();
  void readFromSerial();
  void reset();
};

void inputSense::freqShiftUp() 
{ 
  if(this->freqLevel<100)
    {
      this->freqLevel+=0.50;
      LCD_setValue(this->freqLevel,this->voltageLevel);
    }    
}
void inputSense::freqShiftDown() 
{ 
  if(this->freqLevel>0)
    {
      this->freqLevel-=0.50;
      LCD_setValue(this->freqLevel,this->voltageLevel);
    }
}
void inputSense::volShiftUp() 
{ 
  if(this->voltageLevel<10000)
    {
      this->voltageLevel+=250;
      LCD_setValue(this->freqLevel,this->voltageLevel);
    }
}
void inputSense::volShiftDown() 
{ 
  if(this->voltageLevel>0)
    {
      this->voltageLevel-=250;
      LCD_setValue(this->freqLevel,this->voltageLevel);
    }
}
float inputSense::dispValues(int mode)
{
 if(mode==0)
  return this->freqLevel;
 else if (mode==1)
  return this->voltageLevel;
}

void inputSense::reset()
{
  this->freqLevel=0;
  this->voltageLevel=0;
}

void inputSense::setSystemStatusCode(int code)
{
  this->systemStatusCode = code;
}


int inputSense::getSystemStatusCode() 
{
  return this->systemStatusCode;
}

void inputSense::writeToSerial()
{
  Serial.write(int(this->freqLevel*2));
  Serial.write(int(this->voltageLevel/250));
  Serial.write('e');
}

void inputSense::readFromSerial()
{
  byte buf[2];
  while(int(buf[0])==0)
     Serial.readBytesUntil('e',buf,2);
  this->freqLevel=buf[0]+buf[1]/10;
}

void setup() 
{
  Serial.begin(9600);
  pinMode(freqShiftUpPin,INPUT);
  pinMode(freqShiftDownPin,INPUT);
  pinMode(volShiftUpPin,INPUT);
  pinMode(volShiftDownPin,INPUT);
  pinMode(carraigeReturnSense,INPUT);
  lcd.begin(20, 4);
}

inputSense start;

void loop() 
{
  bool flag=false;
  LCD_init(start.getSystemStatusCode());
  if(start.getSystemStatusCode()==1)
    start.readFromSerial();
  LCD_setValue(start.dispValues(0),start.dispValues(1));
  while(flag==false)
  {
    if(analogRead(freqShiftUpPin)>1000) 
    {
      start.freqShiftUp();
//      Serial.println("freq up");
      delay(500);
    }
    if(analogRead(freqShiftDownPin)>1000)
    {
      start.freqShiftDown();
//      Serial.println("freq dn");
      delay(500);
    }
    if(analogRead(volShiftUpPin)>1000) 
    {
      start.volShiftUp();
//      Serial.println("vol up");
      delay(500);
      }
    if(analogRead(volShiftDownPin)>1000) 
    {
      start.volShiftDown();
//      Serial.println("vol dn");
      delay(500);
      }
    if(analogRead(carraigeReturnSense)>1000) 
    {
       bool timer_trip = false;
       int timer_start = millis();
       delay(250);
       while(timer_trip == false)
       {
          if(analogRead(carraigeReturnSense)>1000)
          {
            flag = true;
//            Serial.println("exit");
            start.setSystemStatusCode(2);
            start.reset();
            timer_trip = true;       
          }
          else if(millis() - timer_start > 2000)
          {
            flag = true;
//            Serial.println("enter");
            if(start.dispValues(0)==0 || start.dispValues(1)==0)
              start.setSystemStatusCode(3);
            else
            {
              start.setSystemStatusCode(1);
              start.writeToSerial();
            }
            timer_trip = true;     
          }
        }  
       delay(500);
     }
  }
}
