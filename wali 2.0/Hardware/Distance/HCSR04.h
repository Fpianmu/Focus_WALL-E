#ifndef __HCSR04_H
#define __HCSR04_H

void HC_SR04_Init(void);
int16_t HC_SR04_GetDistanceMM(void);
float HC_SR04_GetDistanceM(void);

#endif
