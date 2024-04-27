#ifndef SKID_STEER_H
#define SKID_STEER_H

#include "motors.h"

#define kp_L 13.752f
#define A1_L -0.568096f
#define B0_L 3.25489f
#define B1_L -2.82299f

#define kp_R 13.179f
#define A1_R -0.57783f
#define B0_R 2.97670f
#define B1_R -2.55453f

#define max_lin_acc 50.0f
#define max_lin_vel 250.0f
#define max_ang_acc 0.0f
#define max_ang_vel 0.0f
#define wheel_base_mm 220.0f
#define update_period_ms 10.0f

enum ControlMode {
    DISABLED = 0,
    VELOCITY = 1,
    DISPLACEMENT = 2,
    ANG_DISP = 3,
    ANG_VEL = 4
};

void Initialize_Skid_Steer(float left_meas, float right_meas);

void Skid_Steer_Set_Velocity(float lin_vel, float ang_vel);

void Skid_Steer_Set_Displacement(float lin_disp, float ang_disp, float left_meas, float right_meas);

void Skid_Steer_Set_Angular_Velocity(float lin_vel, float ang_vel);

void Skid_Steer_Set_Angular_Displacement(float lin_disp, float ang_disp, float left_meas, float right_meas);

void Skid_Steer_Disable();

ControlMode Skid_Steer_Get_Control_Mode();

void Skid_Steer_Update(float left_meas, float right_meas);

#endif