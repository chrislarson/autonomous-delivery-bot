#include "controller.h"

Controller controller_left;
Controller controller_right;
float num_left[2];
float denom_left[2];
float num_right[2];
float denom_right[2];
static float prev_lin_vel;
static float prev_ang_vel;
static float prev_lin_disp;
static float prev_ang_disp;

#define kp_L 0.0
#define kp_R 0.0
#define max_lin_acc 0.5
#define max_ang_acc 0.0
#define wheel_base 0.0
#define mm_to_counts 0.7540

void Initialize_Skid_Steer();

void Skid_Steer_Set_Velocity(float lin_vel, float ang_vel);

void Skid_Steer_Set_Displacement(float lin_disp, float ang_disp);

void Skid_Steer_Update(float left_meas, float right_meas, float dt);