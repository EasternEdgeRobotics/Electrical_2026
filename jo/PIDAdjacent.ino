//Velocity Map Paramaters Up paramaters are for when below setpoint Down are when above
const float maxUp=0.05; // m/s
const float maxDown=-0.05; // m/s
const float upOffset=-2; // in m distance between 0 error and max speed up negative because max up happens at -meters
const float downOffset=2; // in m distance between 0 error and max speed down positive because max down happens at +meters

float velocity=0;
float depth1=0;
// float velocityForRead=0; //used for printing to sd card

const unsigned long velocityInterval=0.2*1000; //target time used to calcualte current speed
unsigned long depth1Time; //actual value used to calculate current speed
bool velocityTimeMeasured=false; //have we already got a dtdt?

float DepthError(float setPoint, float currentDepth) 
{
  float error=currentDepth-setPoint;
  Serial.print("DepthError:");
  Serial.println(error);
  return(error);
}

float VelocityMap(float error)
{
  float m=(maxDown-maxUp)/(downOffset-upOffset);
  if ((error<=downOffset)&& (error >=upOffset)) //again down offset is a distance up where we start going down and up offset is distance up where we go down from
  {
    Serial.print("VelocityMap in range:");
    Serial.println(m*error);
    return (m*error);
  }
  else if(error<upOffset)
  {
    Serial.print("VelocityMap below:");
    Serial.println(maxUp);
    return (maxUp);
  }
  else if(error>downOffset)
  {
    Serial.print("VelocityMap up:");
    Serial.println(maxDown);
    return (maxDown);
  }
}

float CurrentVelocity(float depth2)
{
  float currentVelocity=((depth1-depth2)/((millis()-depth1Time)/1000f));
  Serial.print("currentVelocity:");
  Serial.println(currentVelocity);
  Serial.print("time interval");
  Serial.println((millis()-depth1Time));
  velocityTimeMeasured=false;
  return(currentVelocity);
}





