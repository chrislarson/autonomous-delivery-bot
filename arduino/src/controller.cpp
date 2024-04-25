#include "controller.h"

/**
 * Function Initialize_Controller sets up the z-transform based controller for the system.
 */
void Initialize_Controller(Controller* cont, float kp, float* A, float* B){
    cont->A[0] = A[0];
    cont->A[1] = A[1];
    cont->B[0] = B[0];
    cont->B[1] = B[1];
    cont->kp = kp;
    //cont->update_period = update_period;
    cont->target_pos = 0;
    cont->target_vel = 0;
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
float Controller_Update(Controller* cont, float measurement, float dt){
    float output_this = cont->B[0]*measurement + cont->B[1]*cont->input_last - cont->A[1]*cont->output_last;
    cont->output_last = output_this;
    cont->target_pos += cont->target_vel*dt;
    float pwm = cont->kp* (cont->target_pos - output_this);
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
    cont->target_pos = measurement;
    cont->input_last = measurement;
    cont->output_last = measurement;
}