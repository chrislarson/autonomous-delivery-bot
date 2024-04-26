#include "skid_steer.h"

#include "controller.h"

Controller controller_left;
Controller controller_right;

static float target_lin_vel;
static float target_ang_vel;
static float curr_lin_disp;
static float target_lin_disp;
static float curr_ang_disp;
static float target_ang_disp;
static float enc_left_origin;
static float enc_right_origin;
ControlMode controlMode;

unsigned long controller_update_prev_time = 0;

// static const float wheel_base_counts = wheel_base_mm * counts_per_mm;

// Private Functions

void Skid_Steer_Zero(float left_meas, float right_meas) {
    Controller_SetTo(&controller_left, left_meas);
    Controller_SetTo(&controller_right, right_meas);
    Controller_Set_Target_Position(&controller_left, left_meas);
    Controller_Set_Target_Position(&controller_right, right_meas);
    enc_left_origin = left_meas;
    enc_right_origin = right_meas;
    target_lin_vel = 0.0;
    target_ang_vel = 0.0;
}

void setControllerVelocities(float lin_vel, float ang_vel) {
    float left_vel = lin_vel - (wheel_base_mm*0.5*ang_vel);
    float right_vel = lin_vel + (wheel_base_mm*0.5*ang_vel);
    Controller_Set_Target_Velocity(&controller_left, left_vel, controlMode);
    Controller_Set_Target_Velocity(&controller_right, right_vel, controlMode);
}

void calcDisplacement(float left_meas, float right_meas) {
    float left_disp;
    float right_disp;
    switch (controlMode)
    {
    case VELOCITY:
    case DISPLACEMENT:
    default:
        left_disp = (left_meas - enc_left_origin)/controller_left.gains[0].counts_per_mm;
        right_disp = (right_meas - enc_right_origin)/controller_right.gains[0].counts_per_mm;
        break;
    case ANG_VEL:
    case ANG_DISP:
        left_disp = (left_meas - enc_left_origin)/controller_left.gains[1].counts_per_mm;
        right_disp = (right_meas - enc_right_origin)/controller_right.gains[1].counts_per_mm;
        break;
    }
    curr_lin_disp = 0.5*(left_disp + right_disp);
    curr_ang_disp = (right_disp - left_disp)/wheel_base_mm;
}

float calcTrapezoidalVelocity(float curr_disp, float target_disp, float prev_vel, float max_vel, float max_acc, float dt_s) {
    //float dir = curr_disp <= target_disp ? 1.0 : -1.0;
    float target_vel;
    float decel_disp = fabs(target_disp) - (prev_vel*prev_vel)/(2*max_acc);
    if (fabs(curr_disp) < decel_disp) {
        target_vel = prev_vel + max_acc*dt_s;
        target_vel = Saturate(target_vel, max_vel);
    } else if (fabs(prev_vel) > max_acc*dt_s) {
        target_vel = prev_vel - max_acc*dt_s;
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
    Initialize_Controller(&controller_left, DISPLACEMENT, kp_L, A1_L, B0_L, B1_L, 1.5785/0.94);
    Initialize_Controller(&controller_right, DISPLACEMENT, kp_R, A1_R, B0_R, B1_R, 1.5215/0.94);
    Skid_Steer_Zero(left_meas, right_meas);
    controlMode = DISABLED;
}

void Skid_Steer_Set_Velocity(float lin_vel, float ang_vel){
    // convert from mm to counts
    // float lin_vel_counts = lin_vel * counts_per_mm;
    // float ang_vel_counts = ang_vel * counts_per_mm;

    // set velocity targets
    target_lin_vel = lin_vel;
    target_ang_vel = ang_vel;

    // switch to velocity mode
    controlMode = VELOCITY;

    setControllerVelocities(target_lin_vel, target_ang_vel);
    controller_update_prev_time = millis();
}


void Skid_Steer_Set_Displacement(float lin_disp, float ang_disp, float left_meas, float right_meas){
    // // convert from mm to counts
    // lin_disp *= counts_per_mm;
    // ang_disp *= counts_per_mm;
    
    // set displacement targets
    target_lin_disp = lin_disp;
    target_ang_disp = ang_disp;

    // zero out controllers so that the velocity starts at 0
    Skid_Steer_Zero(left_meas, right_meas);
    
    // switch to displacement mode
    controlMode = DISPLACEMENT;
    controller_update_prev_time = millis();
}

void Skid_Steer_Disable() {
    controlMode = DISABLED;
}

void Skid_Steer_Update(float left_meas, float right_meas){
    unsigned long currTime = millis();
    unsigned long deltaTime = currTime - controller_update_prev_time;

    if (deltaTime < update_period_ms || controlMode == DISABLED) {
        return;
    }
    
    float deltaTimeSeconds = deltaTime * 1e-3;

    calcDisplacement(left_meas, right_meas);
    switch (controlMode)
    {
    case ANG_DISP:
        target_lin_vel = 0;
        target_ang_vel = calcTrapezoidalVelocity(curr_ang_disp, target_ang_disp, target_ang_vel, max_ang_vel, max_ang_acc, deltaTimeSeconds);
        break;
    case DISPLACEMENT:
        target_lin_vel = calcTrapezoidalVelocity(curr_lin_disp, target_lin_disp, target_lin_vel, max_lin_vel, max_lin_acc, deltaTimeSeconds);
        target_ang_vel = calcTrapezoidalVelocity(curr_ang_disp, target_ang_disp, target_ang_vel, max_ang_vel, max_ang_acc, deltaTimeSeconds);
        break;
    default:
        break;
    }
    setControllerVelocities(target_lin_vel, target_ang_vel);

    float left_setpoint = Controller_Update(&controller_left, left_meas, deltaTimeSeconds);
    float right_setpoint = Controller_Update(&controller_right, right_meas, deltaTimeSeconds);
    //Saturate_Setpoints(&left_setpoint, &right_setpoint, MOTOR_MAX);
    left_setpoint = Saturate(left_setpoint, MOTOR_MAX);
    right_setpoint = Saturate(right_setpoint, MOTOR_MAX);
    
    tankDrive(left_setpoint, right_setpoint);

    controller_update_prev_time = currTime;
}