#define NUM_STEPPERS 2

#define UP true
#define DOWN false

#define Z_STEPPER 0
#define CAM_STEPPER 1

char commandBuffer[50];
int commandBufferPos = 0;

class Stepper
{
  int stepPin, dirPin;
 
  public:
  
  float stepSize;
  int pos, target;

  void setPins(int stepp, int dir);
  void setParams(float ssize, int ppos, int ttarget);
  void step(bool dir);
};

void Stepper::setPins(int stepp, int dir)
{
  stepPin = stepp;
  dirPin = dir;
}

void Stepper::setParams(float ssize, int ppos, int ttarget)
{
  stepSize = ssize;
  pos = ppos;
  target = ttarget;  
}

void Stepper::step(bool dir)
{
  if(dir == UP)
    digitalWrite(dirPin, LOW);
  else
    digitalWrite(dirPin, HIGH);
    
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(400);
  digitalWrite(stepPin, LOW);
}

Stepper stepper[NUM_STEPPERS];

void setup() 
{
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps

  memset(commandBuffer, 0, 50);
  
  stepper[Z_STEPPER].setPins(9 ,10);
  stepper[Z_STEPPER].setParams(1.0 / (200.0 * 18.0 * 4.0), 0, 0);
  
  stepper[CAM_STEPPER].setPins(11, 12);
  stepper[CAM_STEPPER].setParams(1.0 / (200.0 * 4.0), 0, 0);
}

void loop()
{
  communicationHandler();
}

void processCommand(char *buff)
{  
  switch(buff[0])
  {
    case 'R':
    {
      stepper[Z_STEPPER].target = 0.0;
      moveSteppers();
      
      broadcastR(0);
      broadcastZ();
      broadcastS();
    }
    break;
    
    case 'I':
    {
      broadcastI();
    }
    break;
    
    case 'N':
    {
      char substr[50] = {0};
      strncpy(substr, buff + 2, strlen(buff) - 2);
      
      stepper[CAM_STEPPER].target = 800;
      stepper[Z_STEPPER].target += atoi(substr);
      
      moveSteppers();
      
      broadcastF();
    }
    break;
    
    case 'B':
    {
      char substr[50] = {0};
      strncpy(substr, buff + 2, strlen(buff) - 2);
      
      stepper[Z_STEPPER].target = atoi(substr);
      moveSteppers();
      
      broadcastF();
    }
    break;
  }  
}

void moveSteppers()
{
  while(stepper[Z_STEPPER].pos != stepper[Z_STEPPER].target || stepper[CAM_STEPPER].pos != stepper[CAM_STEPPER].target)
  {
    for(int i=0; i < NUM_STEPPERS; i++)
    {
      if(stepper[i].pos < stepper[i].target)
      {
        stepper[i].step(true);
        stepper[i].pos++;
      }
      if(stepper[i].pos > stepper[i].target)
      {
        stepper[i].step(false);
        stepper[i].pos--;
      }
    }
  }
}

void broadcastF()
{
  Serial.println("F");
}

void broadcastI()
{
  Serial.print("I ");
  Serial.println(stepper[Z_STEPPER].stepSize, 10);
}

void broadcastS()
{
  Serial.println("S 0");
}

void broadcastR(int val)
{
  Serial.print("R ");
  Serial.println(val);
}

void broadcastZ()
{
  Serial.print("Z ");
  Serial.println(stepper[Z_STEPPER].pos);
}

void communicationHandler()
{
  
  while(Serial.available() > 0)
  {
    int incoming = Serial.read();
    
    // command has been completely received, process it
    if(incoming == '\n')
    
    
    {
      processCommand(commandBuffer);
      memset(commandBuffer, 0, 50);
      commandBufferPos = 0;
      // break out of loop
      return; 
    }
    // add incoming to commandBuffer if not end of line
    
    else
    {
      commandBuffer[commandBufferPos] = incoming;
      commandBufferPos++;
    }
  }
}
