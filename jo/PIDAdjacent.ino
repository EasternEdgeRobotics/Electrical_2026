//Velocity Map Paramaters Up paramaters are for when below setpoint Down are when above
const float maxUp=0.1; // m/s
const float maxDown=-0.1; // m/s
const float upOffset=-2; // in m distance between 0 error and max speed up negative because max up happens at -meters
const float downOffset=2; // in m distance between 0 error and max speed down positive because max down happens at +meters

float velocity=0;
float depth1=0;

const unsigned long velocityInterval=0.5*1000; //target time used to calcualte current speed
unsigned long depth1Time; //actual value used to calculate current speed
bool velocityTimeMeasured=false; //have we already got a dtdt?

float DepthError(float setPoint, float currentDepth) 
{
  float error=setPoint-currentDepth;
  return(error);
}

float VelocityMap(float error)
{
  float m=(maxDown-maxUp)/(downOffset-upOffset);
  if ((error<=downOffset)&& (error >=upOffset)) //again down offset is a distance up where we start going down and up offset is distance up where we go down from
  {
    return (m*error);
  }
  else if(error<upOffset)
  {
    return (maxUp);
  }
  else if(error>downOffset)
  {
    return (maxDown);
  }
}

float CurrentVelocity(float depth2)
{
  float currentVelocity=((depth1-depth2)/(millis()-depth1Time))/1000;
  return(currentVelocity);
}





