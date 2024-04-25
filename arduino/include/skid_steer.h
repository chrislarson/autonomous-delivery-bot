#ifndef SKID_STEER_H
#define SKID_STEER_H

#include "motors.h"

#define kp_L 1.3752
#define A1_L -0.568096
#define B0_L 3.25489
#define B1_L -2.82299

#define kp_R 1.3179
#define A1_R -0.57783
#define B0_R 2.97670
#define B1_R -2.55453

#define max_lin_acc 500
#define max_lin_vel 500
#define max_ang_acc 0.0
#define max_ang_vel 0.0
#define wheel_base_mm 220
#define update_period_ms 10

enum ControlMode {
    DISABLED = 0,
    VELOCITY = 1,
    DISPLACEMENT = 2
};

void Initialize_Skid_Steer(float left_meas, float right_meas);

void Skid_Steer_Set_Velocity(float lin_vel, float ang_vel);

void Skid_Steer_Set_Displacement(float lin_disp, float ang_disp, float left_meas, float right_meas);

void Skid_Steer_Update(float left_meas, float right_meas);

void Skid_Steer_Disable();

#endif