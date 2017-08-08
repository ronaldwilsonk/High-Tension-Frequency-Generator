/* Frequency Controller
 * This section of the code maintains a particular RPM on the generator. This in turn produces a fixed frequency on the output signal
 */

#include<math.h>                                            //for isnan()

// Pin Maps
const int directionControlPin=9;                            // Controls direction of rotation for stepper HIGH->Clockwise LOW->Anti-Clockwise
const int stepperStepPin=8;                                 // Step pulses makes the motor turn HIGH->LOW transition for step
const int stopSensePin=7;                                   // HIGH->initializes the stepper to startPoint. LOW->resets stepper motor to zero
/*
 * stopSense needs to be mapped
 */
const int encoderReadInPin=11;                              // Encoder read in for Motor             

void setup() 
{
 pinMode(directionControlPin,OUTPUT);
 pinMode(stepperStepPin,OUTPUT);
 pinMode(stopSensePin,INPUT);
 pinMode(encoderReadInPin,INPUT);
 Serial.begin(9600);
}

class stepperCustom                                           // Only effective for Motor2 (as of Built 1.0)  
{
  private:
  int current_position=0;                                                // Keeps track of current position of servo
  const int startPoint=100;
  public:
  int readCurrentPosition(); 
  void startupM();                                            // Initializes stepper motor to starting position (startPoint)
  void shutdownM();                                           // Resets stepper to zero position 
  void stepForward(int n);                                    // Steps forward for 'n' steps
  void stepBackward(int n);                                   // Steps backward for 'n' steps
  int readFromSerial();                                       // Reads input RPM from serial
  bool controller(int rpm, double readOut);                   // Maintains control of motor RPM w.r.t encoder
};

class encoder                           
{
  private:
  double RPM; 
  public:
  double readEncoder();                       // Reads current value from both encoders
  void writeToSerial();
};

int stepperCustom::readCurrentPosition()
{
  return this->current_position;
}

void stepperCustom::startupM()
{
//    Serial.println("Motor Starting");
    digitalWrite(directionControlPin,HIGH);
    digitalWrite(stepperStepPin,HIGH);
    while(this->current_position<=startPoint)
    {
      digitalWrite(stepperStepPin,HIGH);
      delay(5);
      digitalWrite(stepperStepPin,LOW);
      delay(5);
//      Serial.println(this->current_position);
      this->current_position+=1;
    }
    delay(5);
}

void stepperCustom::shutdownM()
{
//    Serial.println("Motor Shutdown");
    digitalWrite(directionControlPin,LOW);
    digitalWrite(stepperStepPin,HIGH);
    while(this->current_position>0)
    {
      digitalWrite(stepperStepPin,HIGH);
      delay(5);
      digitalWrite(stepperStepPin,LOW);
      delay(5);
//      Serial.println(this->current_position);
      this->current_position-=1;
    }
    delay(5);
    this->current_position=0;
}

void stepperCustom::stepForward(int steps)
{
//    Serial.println("FWD");
    digitalWrite(directionControlPin,HIGH);
    for(int i=0;i<=steps;i++)
    {
      digitalWrite(stepperStepPin,HIGH);
      delay(20);
      digitalWrite(stepperStepPin,LOW);
      delay(20);
//      Serial.println(this->current_position);
      this->current_position+=1;
    }
    delay(20);
}

void stepperCustom::stepBackward(int steps)
{
//  Serial.println("BWD");
  digitalWrite(directionControlPin,LOW);
  for(int i=0;i<=steps;i++)
  {
    digitalWrite(stepperStepPin,HIGH);
    delay(20);
    digitalWrite(stepperStepPin,LOW);
    delay(20);
//    Serial.println(this->current_position);
    this->current_position-=1;
  }
  delay(20);
}

bool stepperCustom::controller(int rpm, double readOut)
{
  int error=rpm-readOut, steps=0;
  int delta=30;                                               //minimum error range. Stops PID controller if error is within delta
  if(abs(error)<=150)                                         //control singal limited to promote slower convergence
    steps=1;
  else
    steps=2;
  
  if(error>delta)
  {
    this->stepForward(steps);
    delay(50);
    return true; 
  }
  else if(error<(-delta))
  {
    this->stepBackward(steps);
    delay(50);
    return true;
  }
  else
  {
    delay(1);
    return false;
  }
}

int stepperCustom::readFromSerial()
{
 byte buf[2];
 while(int(buf[0])==0)
    Serial.readBytesUntil('e',buf,2);
 return 30*(int)buf[0];
}

void encoder::writeToSerial()
{
  double RPM=this->RPM/60;                       //Convert RPM to Hz
  int buf[3];
  buf[2] = RPM - (int)RPM;
  RPM=(RPM-buf[2])/10;
  buf[1] = RPM - (int)RPM;
  buf[0]=RPM-buf[1];
  Serial.write(buf[0]);
  Serial.write(buf[1]);
  Serial.write(buf[2]);
  Serial.write("e");
}

double encoder::readEncoder()
{
  double duration, total_duration;
  long pulse_current_position;
  bool flag=true;
  long start_time=millis();
  
  while(flag==true)
  {
    if((millis()-start_time)>500)
    {
      flag=false;
      this->RPM=0;  
    }
    else if(total_duration>0.0005)
       flag=false;
       
    duration=pulseIn(encoderReadInPin, HIGH);
    if(duration!=0)
    {
      total_duration+=(duration*0.000001);
      pulse_current_position+=1;
    }
  }
  this->RPM=(this->RPM+((pulse_current_position/total_duration)))/2;
//  Serial.println("RPM: ");
//  Serial.println(this->RPM);
//  Serial.println(" ");
  delay(1);
  if(flag==false)
    {
      this->writeToSerial();
      return this->RPM;         
    }
}

stepperCustom stepper;                                                      
encoder motor;                                                         
    
void loop()                                                              //main() starts here
{
  int inputRPM=stepper.readFromSerial();
  
  if((stepper.readCurrentPosition()==0)&&(digitalRead(stopSensePin)==HIGH))
  {
    stepper.startupM();
    delay(20);
  }
   
  if((stepper.readCurrentPosition()>0)&&(digitalRead(stopSensePin)==HIGH))
  {
    stepper.controller(inputRPM, motor.readEncoder());
    delay(50); 
  }
  
  if((stepper.readCurrentPosition()>0)&&(digitalRead(stopSensePin)==LOW))
     stepper.shutdownM();
}
