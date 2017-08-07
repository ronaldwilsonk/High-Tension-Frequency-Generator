/* Frequency Controller
 * This section of the code maintains a particular RPM on the generator. This in turn produces a fixed frequency on the output signal
 */

float readTargetFrequency();                                //reads target frequency from serial bus 

#include<math.h>                                            //for isnan()

// Pin Maps
const int directionControlPin=8;                            // Controls direction of rotation for stepper HIGH->Clockwise LOW->Anti-Clockwise
const int stepperStepPin=9;                                 // Step pulses makes the motor turn HIGH->LOW transition for step
const int stopSensePin=7;                                   // HIGH->initializes the stepper to startPoint. LOW->resets stepper motor to zero 
const int encoderReadInPin=10;                              // Encoder read in for Motor             

void setup() 
{
 pinMode(directionControlPin,OUTPUT);
 pinMode(stepperStepPin,OUTPUT);
 pinMode(stopSensePin,INPUT);
 pinMode(encoderReadInPin,INPUT);
 Serial.begin(9600);
}

struct controllerGains
{
  float pGain=0,iGain=0,dGain=0;                              //Differential gain is not used
}PID1;

class stepperCustom                                           // Only effective for Motor2 (as of Built 1.0)  
{
  private:
  int current_position=0;                                                // Keeps track of current position of servo
  const int startPoint=144;
  public:
  int readCurrentPosition(); 
  void startupM();                                            // Initializes stepper motor to starting position (startPoint)
  void shutdownM();                                           // Resets stepper to zero position
  void pid(int rpm, double readOut);                          // Maintains control using PID w.r.t encoder 
  void stepForward(int n);                                    // Steps forward for 'n' steps
  void stepBackward(int n);                                   // Steps backward for 'n' steps
};

int stepperCustom::readCurrentPosition()
{
  return this->current_position;
}
void stepperCustom::startupM()
{
    Serial.println("Motor Starting");
    digitalWrite(directionControlPin,HIGH);
    digitalWrite(stepperStepPin,HIGH);
    while(this->current_position<=startPoint)
    {
      digitalWrite(stepperStepPin,LOW);
      delay(5);
      digitalWrite(stepperStepPin,HIGH);
      delay(5);
      Serial.println(this->current_position);
      this->current_position+=1;
    }
    delay(5);
}

void stepperCustom::shutdownM()
{
    Serial.println("Motor Shutdown");
    digitalWrite(directionControlPin,LOW);
    digitalWrite(stepperStepPin,HIGH);
    while(this->current_position>0)
    {
      digitalWrite(stepperStepPin,LOW);
      delay(5);
      digitalWrite(stepperStepPin,HIGH);
      delay(5);
      Serial.println(this->current_position);
      this->current_position-=1;
    }
    delay(5);
    this->current_position=0;
}

void stepperCustom::stepForward(int steps)
{
    Serial.println("FWD");
    digitalWrite(directionControlPin,HIGH);
    for(int i=0;i<=steps;i++)
    {
      digitalWrite(stepperStepPin,HIGH);
      delay(20);
      digitalWrite(stepperStepPin,LOW);
      delay(20);
      Serial.println(this->current_position);
      this->current_position+=1;
    }
    delay(20);
}

void stepperCustom::stepBackward(int steps)
{
  Serial.println("BWD");
  digitalWrite(directionControlPin,LOW);
  for(int i=0;i<=steps;i++)
  {
    digitalWrite(stepperStepPin,HIGH);
    delay(20);
    digitalWrite(stepperStepPin,LOW);
    delay(20);
    Serial.println(this->current_position);
    this->current_position-=1;
  }
  delay(20);
}

void stepperCustom::pid(int rpm, double readOut)
{
  float error=rpm-readOut;
  PID1.iGain+=error/500;                
  PID1.pGain=error/200;
  int input=PID1.pGain+PID1.iGain;
  
  if(error>0)
    this->stepForward(input);
  else if(error<0)
    this->stepBackward(input);
}

class encoder                           
{
  public:
  long pulse_current_position;
  double RPM=0, duration, total_duration;
  double denoiser(double rpm);              // Clears random noise fluctuations introduced by device. Linear and deterministic in nature
  void readEncoder(encoder& rpmM1);         // Reads current value from both encoders
};

double encoder::denoiser(double rpm) 
{
  return 81.7*(0.009144*rpm+26.08)-2078;
}

void encoder::readEncoder(encoder& M1)
{
  bool flag=true;
  while(flag==true)
  {
    if(M1.total_duration>0.0005)
       flag=false;
    M1.duration = pulseIn(encoderReadInPin, HIGH);
    if(M1.duration!=0)
    {
      M1.total_duration+=(M1.duration*0.000001);
      M1.pulse_current_position+=1;
    }
  }
  
  M1.RPM=(M1.RPM+((M1.pulse_current_position/M1.total_duration)))/2;

  Serial.println("M1 RPM: ");
  Serial.println(M1.RPM);
  Serial.println(M1.denoiser(M1.RPM));
  Serial.println(" ");
  delay(1);         
}

stepperCustom M3;                                                      //M3->Motor 3: refers to stepper motor
encoder M1;                                                         //M1->Motor 1, M2->Motor 2, high speed motor. Refer build 1.0 documentation
    
void loop()                                                              //main() starts here
{
  if((M3.readCurrentPosition()==0)&&(digitalRead(stopSensePin)==HIGH))
  {
    M3.startupM();
    delay(250);
  }
   
  if((M3.readCurrentPosition()>0)&&(digitalRead(stopSensePin)==HIGH))
  {
    M3.stepForward(50);
    delay(20);
    M3.stepBackward(50);
  }
  
  if((M3.readCurrentPosition()>0)&&(digitalRead(stopSensePin)==LOW))
     M3.shutdownM();
}

float readTargetFrequency()
{
  int n;
}
