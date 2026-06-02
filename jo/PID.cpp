
/**
 * This document is a copy pf the PID controller by Philip Salmony 
 * https://github.com/pms67/PID/blob/master/PID.c
 * https://www.youtube.com/watch?v=zOByx3Izf5U
 * 

 MIT License

Copyright (c) 2020 Philip Salmony

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 * 
 */


#include "pid.h"

void PIDController_Init(PIDController *pid) {

    // clear controller variables
    pid->integrator = 0.0f;
    pid->prevError = 0.0f;

    pid->differentiator = 0.0f;
    pid->prevMeasurement = 0.0f;

    pid->out = 0.0f;
}

float PIDController_Update(PIDController *pid, float setpoint, float measurement) {
    
    //error
    float error = setpoint - measurement;
    
    // proportional
    float proportional = pid->Kp * error;

    //integral
    pid->integrator = pid->integrator + 0.5f * pid->Ki * pid->T * (error + pid->prevError);

    // anti-wind-up via integrator clamping
    if (pid->integrator > pid->limMaxInt) {
        pid->integrator = pid->limMaxInt;
    }
    else if (pid->integrator < pid->limMinInt) {
        pid->integrator = pid->limMinInt;
    }

    // derivative (band-limited differentiator) MIGHT WANT TO CHANGE -Brendan & ozzy
    pid->differentiator = -(2.0f * pid->Kd * (measurement - pid->prevMeasurement) // note: derivative on measurement, therefore minus sign in fromt of equation
                            + (2.0f * pid->tau - pid->T) * pid->differentiator)
                            / (2.0f * pid->tau + pid->T);
    
    // compute output and apply limits
    pid->out = proportional + pid->integrator + pid->differentiator;
//use our own clamping function later maybe
    // if (pid->out > pid->outLimMax) { 
    //     pid->out = pid->outLimMax;
    // }
    // else if (pid->out < pid->outLimMin) {
    //     pid->out = pid->outLimMin;
    // }

    // store error and measurement for later use
    pid->prevError = error;
    pid->prevMeasurement = measurement;

    // return controller output
    return pid->out;
}