#include "Arduino.h"
#include "skid_steer.h"
typedef struct
{
    float A_1;
    float B_0;
    float B_1;
    float kp;
    float counts_per_mm;
} ControlerGains;

typedef struct
{
    ControlerGains gains[2];
    float target_pos;
    float target_vel;
    float input_last;
    float output_last;
} Controller;


/**
 * Function Saturate saturates a value to be within the range.
 */
inline float Saturate( float value, float ABS_MAX )
{
    return ( value > ABS_MAX ) ? ABS_MAX : ( value < -ABS_MAX ) ? -ABS_MAX : value;
}

/**
 * Function Initialize_Controller sets up the z-transform based controller for the system.
 */
void Initialize_Controller(Controller* cont, ControlMode mode, float kp, float A_1, float B_0, float B_1, float cpmm);

/**
 * Function Controller_Set_Target_Velocity sets the target velocity for the
 * controller.
 */
void Controller_Set_Target_Velocity(Controller* cont, float vel, ControlMode controlMode);

/**
 * Function Controller_Set_Target_Position sets the target postion for the
 * controller, this also sets the target velocity to 0.
 */
void Controller_Set_Target_Position_Counts(Controller* cont, float pos );

/**
 * Function Controller_Add_Target_Position adds to the target postion for the
 * controller, this also sets the target velocity to 0.
 */
void Controller_Add_Target_Position(Controller* cont, float displacement, ControlMode controlMode );

/**
 * Function Controller_Update takes in a new measurement and returns the
 * new control value.
 */
float Controller_Update(Controller* cont, float measurement, float dt_s, ControlMode controlMode);

/**
 * Function Controller_Last returns the last control command
 */
float Controller_Last(Controller* cont);

/**
 * Function Controller_SettTo sets the last input and output
 * to match the measurement so it starts with zero error.
 */
void Controller_SetTo(Controller* cont, float measurement);