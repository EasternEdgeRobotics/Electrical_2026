//Velocity Map Paramaters
const float maxUp=0.1; // m/s
const float maxDown=-0.1; // m/s
const float upOffset=2; // in m distance between 0 error and max speed up
const float downOffset=-2; // in m distance between 0 error and max speed down 



float DepthError(float setPoint, float currentDepth) 
{
  float error=setPoint-currentDepth;
  return(error);
}

float VelocityMap(float error)
{
  float m=(maxUp-maxDown)/(upOffset-downOffset);
  if ((error<=upOffset)&& (error >=downOffset))
  {
    return (m*error);
  }
  else if(error>upOffset)
  {
    return (maxUp);
  }
  else if(error<downOffset)
  {
    return (maxDown);
  }
}






