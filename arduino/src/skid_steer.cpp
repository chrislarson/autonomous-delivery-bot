#include "skid_steer.h"

#include "controller.h"
#include "serial.h"

Controller controller_left;
Controller controller_right;

static float curr_lin_vel = 0.0;
static float curr_ang_vel = 0.0;
static float curr_lin_disp = 0.0;
static float target_lin_disp;
static float curr_ang_disp = 0.0;
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
  Controller_Set_Target_Position_Counts(&controller_left, left_meas);
  Controller_Set_Target_Position_Counts(&controller_right, right_meas);
  enc_left_origin = left_meas;
  enc_right_origin = right_meas;
  // curr_lin_vel = 0.0;
  // curr_ang_vel = 0.0;
  curr_lin_disp = 0.0;
  curr_ang_disp = 0.0;
}

void setControllerVelocities(float lin_vel, float ang_vel) {
  float left_vel = lin_vel - (wheel_base_mm * 0.5 * ang_vel);
  float right_vel = lin_vel + (wheel_base_mm * 0.5 * ang_vel);
  Controller_Set_Target_Velocity(&controller_left, left_vel, controlMode);
  Controller_Set_Target_Velocity(&controller_right, right_vel, controlMode);
}

void addControllerDisplacements(float lin_disp, float ang_disp) {
  float left_disp = lin_disp - (wheel_base_mm * 0.5 * ang_disp);
  float right_disp = lin_disp + (wheel_base_mm * 0.5 * ang_disp);
  Controller_Add_Target_Position(&controller_left, left_disp, controlMode);
  Controller_Add_Target_Position(&controller_right, right_disp, controlMode);
}

// void calcDisplacement(float left_meas, float right_meas) {
//     float left_disp;
//     float right_disp;
//     switch (controlMode)
//     {
//     case VELOCITY:
//     case DISPLACEMENT:
//     default:
//         left_disp = (left_meas -
//         enc_left_origin)/controller_left.gains[0].counts_per_mm; right_disp =
//         (right_meas -
//         enc_right_origin)/controller_right.gains[0].counts_per_mm; break;
//     case ANG_VEL:
//     case ANG_DISP:
//         left_disp = (left_meas -
//         enc_left_origin)/controller_left.gains[1].counts_per_mm; right_disp =
//         (right_meas -
//         enc_right_origin)/controller_right.gains[1].counts_per_mm; break;
//     }
//     curr_lin_disp = 0.5*(left_disp + right_disp);
//     curr_ang_disp = (right_disp - left_disp)/wheel_base_mm;
// }

float calcTrapezoidalVelocity(float curr_disp, float target_disp,
                              float prev_vel, float max_vel, float max_acc,
                              float dt_s) {
  float dir = 0 <= target_disp ? 1.0 : -1.0;
  float target_vel;
  float decel_disp = fabs(target_disp) - (prev_vel * prev_vel) / (2 * max_acc);
  if (fabs(curr_disp) < decel_disp) {
    target_vel = prev_vel + dir * max_acc * dt_s;
    target_vel = Saturate(target_vel, max_vel);
  } else if (fabs(prev_vel) > max_acc * dt_s) {
    target_vel = prev_vel - dir * max_acc * dt_s;
  } else {
    target_vel = 0;
  }
  return target_vel;
}

void Saturate_Setpoints(float* left_setpoint, float* right_setpoint,
                        float ABS_MAX) {
  if (*left_setpoint < *right_setpoint) {
    float unsat_right_setpoint = *right_setpoint;
    *right_setpoint = Saturate(*right_setpoint, ABS_MAX);
    *left_setpoint *= *right_setpoint / unsat_right_setpoint;
  } else {
    float unsat_left_setpoint = *left_setpoint;
    *left_setpoint = Saturate(*left_setpoint, ABS_MAX);
    *right_setpoint *= *left_setpoint / unsat_left_setpoint;
  }
}

// Public Functions

void Initialize_Skid_Steer(float left_meas, float right_meas) {
  Initialize_Controller(&controller_left, DISPLACEMENT, kp_L, A1_L, B0_L, B1_L,
                        1.5785 * 1.035 * 1.06);
  Initialize_Controller(&controller_right, DISPLACEMENT, kp_R, A1_R, B0_R, B1_R,
                        1.5785 * 1.035);
  Initialize_Controller(&controller_left, ANG_DISP, kp_L, A1_L, B0_L, B1_L,
                        1.5785 * 1.035);
  Initialize_Controller(&controller_right, ANG_DISP, kp_R, A1_R, B0_R, B1_R,
                        1.5785 * 1.035);
  Skid_Steer_Zero(left_meas, right_meas);
  controlMode = DISABLED;
}

void Skid_Steer_Set_Velocity(float lin_vel, float ang_vel) {
  // convert from mm to counts
  // float lin_vel_counts = lin_vel * counts_per_mm;
  // float ang_vel_counts = ang_vel * counts_per_mm;

  // set velocity targets
  curr_lin_vel = lin_vel;
  curr_ang_vel = ang_vel;

  // switch to velocity mode
  controlMode = VELOCITY;

  setControllerVelocities(curr_lin_vel, curr_ang_vel);
  controller_update_prev_time = millis();
}

void Skid_Steer_Set_Displacement(float lin_disp, float ang_disp,
                                 float left_meas, float right_meas) {
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

void Skid_Steer_Set_Angular_Velocity(float lin_vel, float ang_vel) {
  // convert from mm to counts
  // float lin_vel_counts = lin_vel * counts_per_mm;
  // float ang_vel_counts = ang_vel * counts_per_mm;

  // set velocity targets
  curr_lin_vel = lin_vel;
  curr_ang_vel = ang_vel;

  // switch to angular velocity mode
  controlMode = ANG_VEL;

  setControllerVelocities(curr_lin_vel, curr_ang_vel);
  controller_update_prev_time = millis();
}

void Skid_Steer_Set_Angular_Displacement(float lin_disp, float ang_disp,
                                         float left_meas, float right_meas) {
  // // convert from mm to counts
  // lin_disp *= counts_per_mm;
  // ang_disp *= counts_per_mm;

  // set displacement targets
  target_lin_disp = lin_disp;
  target_ang_disp = ang_disp;

  // zero out controllers so that the velocity starts at 0
  Skid_Steer_Zero(left_meas, right_meas);

  // switch to angular displacement mode
  controlMode = ANG_DISP;
  controller_update_prev_time = millis();
}

void Skid_Steer_Disable() { controlMode = DISABLED; }

ControlMode Skid_Steer_Get_Control_Mode() { return controlMode; }

void Skid_Steer_Update(float left_meas, float right_meas) {
  unsigned long currTime = millis();
  unsigned long deltaTime = currTime - controller_update_prev_time;

  if (deltaTime < update_period_ms || controlMode == DISABLED) {
    return;
  }

  float deltaTimeSeconds = deltaTime * 1e-3;

  // calcDisplacement(left_meas, right_meas);

  float delta_lin_disp = Saturate(curr_lin_vel * deltaTimeSeconds,
                                  fabs(target_lin_disp - curr_lin_disp));
  float delta_ang_disp = Saturate(curr_ang_vel * deltaTimeSeconds,
                                  fabs(target_ang_disp - curr_ang_disp));
  addControllerDisplacements(delta_lin_disp, delta_ang_disp);
  curr_lin_disp += delta_lin_disp;
  curr_ang_disp += delta_ang_disp;

  switch (controlMode) {
    case DISPLACEMENT:
    case ANG_DISP:
      curr_lin_vel =
          calcTrapezoidalVelocity(curr_lin_disp, target_lin_disp, curr_lin_vel,
                                  max_lin_vel, max_lin_acc, deltaTimeSeconds);
      curr_ang_vel =
          calcTrapezoidalVelocity(curr_ang_disp, target_ang_disp, curr_ang_vel,
                                  max_ang_vel, max_ang_acc, deltaTimeSeconds);
      if (fabs(curr_lin_vel) < 1e-6 && fabs(curr_ang_vel) < 1e-6 &&
          fabs(left_meas - controller_left.input_last) < 1e-6 &&
          fabs(right_meas - controller_right.input_last) < 1e-6) {
        controlMode = DISABLED;
      }
      break;
    default:
      break;
  }

  // sendDebug(curr_lin_vel, fabs(curr_ang_vel), fabs(left_meas -
  // controller_left.input_last), fabs(right_meas -
  // controller_right.input_last), 20.0);

  // float delta_lin_disp = curr_lin_vel * deltaTimeSeconds;
  // if (fabs(curr_lin_vel) < 1e-6) {
  //   delta_lin_disp = target_lin_disp - curr_lin_disp;
  // }
  // float delta_ang_disp = curr_ang_vel * deltaTimeSeconds;
  // if (fabs(curr_ang_vel) < 1e-6) {
  //   delta_ang_disp = target_ang_disp - curr_ang_disp;
  // }
  // addControllerDisplacements(delta_lin_disp, delta_ang_disp);
  // curr_lin_disp += delta_lin_disp;
  // curr_ang_disp += delta_ang_disp;

  // setControllerVelocities(curr_lin_vel, curr_ang_vel);

  float left_setpoint = Controller_Update(&controller_left, left_meas,
                                          deltaTimeSeconds, controlMode);
  float right_setpoint = Controller_Update(&controller_right, right_meas,
                                           deltaTimeSeconds, controlMode);
  // Saturate_Setpoints(&left_setpoint, &right_setpoint, MOTOR_MAX);
  left_setpoint = Saturate(left_setpoint, MOTOR_MAX);
  right_setpoint = Saturate(right_setpoint, MOTOR_MAX);

  if (left_setpoint * target_lin_disp < 0) {
    left_setpoint = 0;
  }

  if (right_setpoint * target_lin_disp < 0) {
    right_setpoint = 0;
  }

  if (controlMode != DISABLED) {
    tankDrive(left_setpoint, right_setpoint);
  } else {
    tankDrive(0, 0);
  }

  controller_update_prev_time = currTime;
}

unsigned long debug_prev_time = 0;
void sendDebug(float num1, float num2, float num3, float num4,
               float debug_period) {
  if (debug_prev_time >= 0) {
    unsigned long currTime = millis();
    unsigned long deltaTime = currTime - debug_prev_time;
    if (deltaTime >= (unsigned long)debug_period) {
      debug_prev_time = currTime;
      DebugCmd debugCmd;
      debugCmd.data.cmd = DEBUG;
      debugCmd.data.time_millis = currTime;
      debugCmd.data.left_vel = num1;
      debugCmd.data.right_vel = num2;
      debugCmd.data.left_pos = num3;
      debugCmd.data.right_pos = num4;
      sendCommand(DEBUG, &debugCmd);
    }
  }
}