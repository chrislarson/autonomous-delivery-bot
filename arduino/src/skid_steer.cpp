#include "skid_steer.h"

#include "controller.h"

Controller controller_left;
Controller controller_right;
float A_L[2] = {1, A1_L};
float B_L[2] = {B0_L, B1_L};
float A_R[2] = {1, A1_R};
float B_R[2] = {B0_R, B1_R};

static float prev_lin_vel;
static float prev_ang_vel;
static float curr_lin_disp;
static float target_lin_disp;
static float curr_ang_disp;
static float target_ang_disp;
static float enc_left_origin;
static float enc_right_origin;
boolean velocity_mode;
boolean skid_steer_enabled;

static const float mm_to_counts = 1/mm_per_count;
static const float wheel_base_counts = wheel_base_mm * mm_per_count;

// Private Functions

void Skid_Steer_Zero(float left_meas, float right_meas) {
    Controller_SetTo(&controller_left, left_meas);
    Controller_SetTo(&controller_right, right_meas);
    Controller_Set_Target_Position(&controller_left, left_meas);
    Controller_Set_Target_Position(&controller_right, right_meas);
    enc_left_origin = left_meas;
    enc_right_origin = right_meas;
    prev_lin_vel = 0.0;
    prev_ang_vel = 0.0;
    velocity_mode = true;
}

void setControllerVelocities(float lin_vel, float ang_vel) {
    float left_vel = lin_vel - (wheel_base_counts*0.5*ang_vel);
    float right_vel = lin_vel + (wheel_base_counts*0.5*ang_vel);
    Controller_Set_Target_Velocity(&controller_left,left_vel);
    Controller_Set_Target_Velocity(&controller_right,right_vel);
    prev_lin_vel = lin_vel;
    prev_ang_vel = ang_vel;
}

void calcDisplacement(float left_meas, float right_meas) {
    float left_disp = left_meas - enc_left_origin;
    float right_disp = right_meas - enc_right_origin;
    curr_lin_disp = 0.5*(left_disp + right_disp);
    curr_ang_disp = (right_disp - left_disp)/wheel_base_counts;
}

float calcTargetVelocity(float curr_disp, float target_disp, float prev_vel, float max_vel, float max_acc, float dt) {
    float dir = curr_lin_disp <= target_lin_disp ? 1.0 : -1.0;
    float target_vel;
    if ((dir*curr_disp) < (dir*target_disp) - (prev_vel*prev_vel)/(2*max_acc)) {
        target_vel = prev_vel + dir*max_acc*dt;
        target_vel = Saturate(target_vel, max_vel);
    } else if (dir*prev_vel > max_acc*dt) {
        target_vel = prev_vel - dir*max_acc*dt;
    } else {
        target_vel = 0;
    }
    return target_vel;
}

void Saturate_Setpoints(float* left_setpoint, float* right_setpoint, float ABS_MAX){
    if (*left_setpoint < *right_setpoint) {
        float unsat_right_setpoint = *right_setpoint;
        *right_setpoint = Saturate(*right_setpoint, ABS_MAX);
        *left_setpoint *= *right_setpoint/unsat_right_setpoint;
    } else {
        float unsat_left_setpoint = *left_setpoint;
        *left_setpoint = Saturate(*left_setpoint, ABS_MAX);
        *right_setpoint *= *left_setpoint/unsat_left_setpoint;
    }
}

// Public Functions

void Initialize_Skid_Steer(float left_meas, float right_meas){
    Initialize_Controller(&controller_left, kp_L, &A_L[0], &B_L[0]);
    Initialize_Controller(&controller_right, kp_R, &A_R[0], &B_R[0]);
    Skid_Steer_Zero(left_meas, right_meas);
}

void Skid_Steer_Set_Velocity(float lin_vel, float ang_vel){
    lin_vel *= mm_to_counts;
    ang_vel *= mm_to_counts;
    setControllerVelocities(lin_vel, ang_vel);
    velocity_mode = true;
}


void Skid_Steer_Set_Displacement(float lin_disp, float ang_disp, float left_meas, float right_meas){
    left_meas *= mm_to_counts;
    right_meas *= mm_to_counts;
    Controller_Set_Target_Position(&controller_left, left_meas);
    Controller_Set_Target_Position(&controller_right, right_meas);
    enc_left_origin = left_meas;
    enc_right_origin = right_meas;
    target_lin_disp = lin_disp;
    target_ang_disp = ang_disp;
    velocity_mode = false;
}

unsigned long controller_update_prev_time = 0;
void Skid_Steer_Update(float left_meas, float right_meas){
    unsigned long currTime = millis();
    unsigned long deltaTime = currTime - controller_update_prev_time;

    if (deltaTime <= update_period_ms) {
        return;
    }
    else if (!skid_steer_enabled) {
        Skid_Steer_Zero(left_meas, right_meas);
        return;
    }
    
    calcDisplacement(left_meas, right_meas);

    if (!velocity_mode) {
        float target_lin_vel = calcTargetVelocity(curr_lin_disp, target_lin_disp, prev_lin_vel, max_lin_vel, max_lin_acc, deltaTime);
        float target_ang_vel = calcTargetVelocity(curr_ang_disp, target_ang_disp, prev_ang_vel, max_ang_vel, max_ang_acc, deltaTime);
        setControllerVelocities(target_lin_vel, target_ang_vel);
    }

    float left_setpoint = Controller_Update(&controller_left, left_meas, deltaTime);
    float right_setpoint = Controller_Update(&controller_right, right_meas, deltaTime);

    Saturate_Setpoints(&left_setpoint, &right_setpoint, MOTOR_MAX);
    
    tankDrive(left_setpoint, right_setpoint);
}

void Skid_Steer_Set_Enabled(boolean enabled) {
    skid_steer_enabled = enabled;
}