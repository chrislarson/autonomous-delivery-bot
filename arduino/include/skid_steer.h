#ifndef SKID_STEER_H
#define SKID_STEER_H

#include "motors.h"
#include "math.h"

// #define kp_L 13.752f
// #define A1_L -0.568096f
// #define B0_L 3.25489f
// #define B1_L -2.82299f

// #define kp_R 13.179f
// #define A1_R -0.57783f
// #define B0_R 2.97670f
// #define B1_R -2.55453f

// Poles generated via step response. Tr = 0.01, OS = 0.03, 100Hz (dt -> 0.01)
#define kp_L 13.268041783323165f
#define A1_L -0.247159579006990f
#define B0_L 7.134106198740166f
#define B1_L -6.381265777747156f

#define kp_R 12.399083893268257f
#define A1_R -0.249467723627006f
#define B0_R 6.880745815434894f
#define B1_R -6.130213539061900f

// #define kp_L 5.0606f
// #define A1_L -0.3573f
// #define B0_L 9.0926f
// #define B1_L -8.4499f

// #define kp_R 4.7405f
// #define A1_R -0.3606f
// #define B0_R 8.6222f
// #define B1_R -7.9828f

// // Poles generated via bessel filter.

#define max_lin_acc 150.0f
#define max_lin_vel 250.0f
#define max_ang_acc PI/2.0f
#define max_ang_vel PI
#define wheel_base_mm 220.0f
#define update_period_ms 10.0f

// #define max_lin_acc 50.0f
// #define max_lin_vel 250.0f
// #define max_ang_acc 0.0f
// #define max_ang_vel 0.0f
// #define wheel_base_mm 220.0f
// #define update_period_ms 10.0f

enum ControlMode {
  DISABLED = 0,
  VELOCITY = 1,
  DISPLACEMENT = 2,
  ANG_DISP = 3,
  ANG_VEL = 4,
  WAIT = 5
};

void Initialize_Skid_Steer(float left_meas, float right_meas);

void Skid_Steer_Set_Velocity(float lin_vel, float ang_vel);

void Skid_Steer_Set_Displacement(float lin_disp, float ang_disp,
                                 float left_meas, float right_meas);

void Skid_Steer_Set_Angular_Velocity(float lin_vel, float ang_vel);

void Skid_Steer_Set_Angular_Displacement(float lin_disp, float ang_disp,
                                         float left_meas, float right_meas);

void Skid_Steer_Wait(unsigned long period_ms);

void Skid_Steer_Disable();

ControlMode Skid_Steer_Get_Control_Mode();

void Skid_Steer_Update(float left_meas, float right_meas);

void sendDebug(float num1, float num2, float num3, float num4,
               float debug_period);

#endif