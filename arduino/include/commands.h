#include "serial.h"

#define TRAJ_BUFFER_LEN 10

void execCmd(Command cmd);

bool isEnabled();

void addToBuffer(float distance, float theta);