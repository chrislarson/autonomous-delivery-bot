#include "skid_steer.h"
#include "motors.h"

void Initialize_Skid_Steer(){
    Initialize_Controller(&controller_left, kp_L, &num_left[0],&denom_left[0]);
    Initialize_Controller(&controller_right, kp_R, &num_right[0],&denom_right[0]);
    prev_lin_vel = 0.0;
    prev_ang_vel = 0.0;
    prev_lin_disp = 0.0;
    prev_ang_disp = 0.0;
}

void Skid_Steer_Set_Velocity(float lin_vel, float ang_vel){
    float left_vel = lin_vel - (wheel_base*0.5*ang_vel);
    float right_vel = lin_vel + (wheel_base*0.5*ang_vel);
    Controller_Set_Target_Velocity(&controller_left,left_vel * mm_to_counts);
    Controller_Set_Target_Velocity(&controller_right,right_vel * mm_to_counts);
}

void Skid_Steer_Set_Displacement(float lin_disp, float ang_disp, float left_meas, float right_meas){
    
}

void Skid_Steer_Update(float left_meas, float right_meas, float dt){

}