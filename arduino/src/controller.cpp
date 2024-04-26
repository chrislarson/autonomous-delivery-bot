#include "controller.h"

/**
 * Function Initialize_Controller sets up the z-transform based controller for the system.
 */
void Initialize_Controller(Controller* cont, float kp, float A_1, float B_0, float B_1, float cpmm){
    cont->A_1 = A_1;
    cont->B_0 = B_0;
    cont->B_1 = B_1;
    cont->kp = kp;
    cont->counts_per_mm = cpmm;
}

/**
 * Function Controller_Set_Target_Velocity sets the target velocity for the
 * controller.
    float right_vel = lin_vel + (wheel_base*0.5*ang_vel);
 */
void Controller_Set_Target_Velocity(Controller* cont, float vel){
    cont->target_vel = vel;
}

/**
 * Function Controller_Set_Target_Position sets the target postion for the
 * controller, this also sets the target velocity to 0.
 */
void Controller_Set_Target_Position(Controller* cont, float pos ){
    cont->target_pos = pos;
    cont->target_vel = 0;
}

/**
 * Function Controller_Update takes in a new measurement and returns the
 * new control value.
 */
float Controller_Update(Controller* cont, float measurement, float dt_s, ControlMode mode){
    float output_this = cont->B_0*measurement + cont->B_1*cont->input_last - cont->A_1*cont->output_last;
    cont->output_last = output_this;
    cont->input_last = measurement;

    cont->target_pos += cont->target_vel*dt_s;

    float pwm = cont->kp * (cont->target_pos - output_this);
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