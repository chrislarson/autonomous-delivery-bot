#ifndef SKID_STEER_H
#define SKID_STEER_H

#include "Arduino.h"

#include "motors.h"

#define kp_L 0.0
#define A1_L 0.0
#define B0_L 0.0
#define B1_L 0.0

#define kp_R 0.0
#define A1_R 0.0
#define B0_R 0.0
#define B1_R 0.0

#define max_lin_acc 0.5
#define max_lin_vel 0.5
#define max_ang_acc 0.0
#define max_ang_vel 0.0
#define wheel_base_mm 0.0
#define update_period_ms 10

void Initialize_Skid_Steer(float left_meas, float right_meas);

void Skid_Steer_Set_Velocity(float lin_vel, float ang_vel);

void Skid_Steer_Set_Displacement(float lin_disp, float ang_disp, float left_meas, float right_meas);

void Skid_Steer_Update(float left_meas, float right_meas);

void Skid_Steer_Set_Enabled(boolean enabled);

#endif