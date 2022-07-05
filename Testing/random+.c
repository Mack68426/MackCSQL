#include "random+.h"

int randomInt(int lower, int upper) { return rand() % (upper - lower + 1) + lower;}

unsigned int randomUInt(unsigned int lower, unsigned int upper) { return rand() % (upper - lower + 1) + lower;}

float randomFloat(float lower, float upper) { return (float)(rand() * (upper - lower) / (RAND_MAX + 1.0F) + lower); }

double randomDouble(double lower, double upper) { return (double)(rand() * (upper - lower) / (RAND_MAX +1.0) + lower); }

char randomChar(int lower, int upper) { return (char)(rand() % (upper - lower + 1) + lower); }