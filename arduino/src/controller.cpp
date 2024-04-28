#include "controller.h"

/**
 * Function Initialize_Controller sets up the z-transform based controller for the system.
 */
void Initialize_Controller(Controller* cont, ControlMode controlMode, float kp, float A_1, float B_0, float B_1, float cpmm){
    switch (controlMode)
    {
    case VELOCITY:
    case DISPLACEMENT:
    default:
        cont->gains[0].A_1 = A_1;
        cont->gains[0].B_0 = B_0;
        cont->gains[0].B_1 = B_1;
        cont->gains[0].kp = kp;
        cont->gains[0].counts_per_mm = cpmm;
        break;
    case ANG_VEL:
    case ANG_DISP:
        cont->gains[1].A_1 = A_1;
        cont->gains[1].B_0 = B_0;
        cont->gains[1].B_1 = B_1;
        cont->gains[1].kp = kp;
        cont->gains[1].counts_per_mm = cpmm;
        break;
    }
}

/**
 * Function Controller_Set_Target_Velocity sets the target velocity for the
 * controller.
    float right_vel = lin_vel + (wheel_base*0.5*ang_vel);
 */
void Controller_Set_Target_Velocity(Controller* cont, float vel, ControlMode controlMode){
    switch (controlMode)
    {
    case VELOCITY:
    case DISPLACEMENT:
    default:
        cont->target_vel = vel * cont->gains[0].counts_per_mm;
        break;
    case ANG_VEL:
    case ANG_DISP:
        cont->target_vel = vel * cont->gains[1].counts_per_mm;
        break;
    }
}

/**
 * Function Controller_Set_Target_Position sets the target postion for the
 * controller, this also sets the target velocity to 0.
 */
void Controller_Set_Target_Position_Counts(Controller* cont, float pos ){
    cont->target_pos = pos;
    cont->target_vel = 0;
}

/**
 * Function Controller_Add_Target_Position adds to the target postion for the
 * controller, this also sets the target velocity to 0.
 */
void Controller_Add_Target_Position(Controller* cont, float displacement, ControlMode controlMode ){
    switch (controlMode)
    {
    case VELOCITY:
    case DISPLACEMENT:
    default:
        cont->target_pos = cont->target_pos + (displacement * cont->gains[0].counts_per_mm);
        break;
    case ANG_VEL:
    case ANG_DISP:
        cont->target_pos = cont->target_pos + (displacement * cont->gains[1].counts_per_mm);
        break;
    }
    cont->target_vel = 0;
}

/**
 * Function Controller_Update takes in a new measurement and returns the
 * new control value.
 */
float Controller_Update(Controller* cont, float measurement, float dt_s, ControlMode controlMode){
    float B_0, B_1, A_1, kp;
    switch (controlMode)
    {
    case VELOCITY:
    case DISPLACEMENT:
    default:
        B_0 = cont->gains[0].B_0;
        B_1 = cont->gains[0].B_1;
        A_1 = cont->gains[0].A_1;
        kp = cont->gains[0].kp;
        break;
    case ANG_VEL:
    case ANG_DISP:
        B_0 = cont->gains[1].B_0;
        B_1 = cont->gains[1].B_1;
        A_1 = cont->gains[1].A_1;
        kp = cont->gains[1].kp;
        break;
    }
    
    float output_this = B_0*measurement + B_1*cont->input_last - A_1*cont->output_last;
    cont->output_last = output_this;
    cont->input_last = measurement;

    cont->target_pos += cont->target_vel*dt_s;

    float pwm = kp * (cont->target_pos - output_this);
    return pwm;
}

/**
 * Function Controller_Last returns the last control command
 */
float Controller_Last(Controller* cont){
    return cont->output_last;
}

/**
 * Function Controller_SettTo sets the last input and output
 * to match the measurement so it starts with zero error.
 */
void Controller_SetTo(Controller* cont, float measurement){
    cont->input_last = measurement;
    cont->output_last = measurement;
}