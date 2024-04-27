#include "serial.h"

#define TRAJ_BUFFER_LEN 10

void execCmd(Command cmd);

bool isEnabled();

bool getNextCmd(float* dist, float* rads);