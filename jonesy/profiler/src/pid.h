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


#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

typedef struct {

    // controller gains
    float Kp;
    float Ki;
    float Kd;

    // derivative low-pass filter time constant
    float tau;

    // output limits
    float outLimMin;
    float outLimMax;

    // integrator limits
    float limMinInt;
    float limMaxInt;

    // sample time (in seconds)
    float T;

    // controller memory
    float integrator;
    float prevError;        // required for integrator
    float differentiator;
    float prevMeasurement;  // required for differentiator

    // controller output
    float out;

} PIDController;

void PIDController_Init(PIDController *pid);
float PIDController_Update(PIDController *pid, float setpoint, float measurement);

#endif