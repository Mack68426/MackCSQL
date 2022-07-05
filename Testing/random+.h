#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/random.h>

// 設定亂數種子
#define set_random_seed() srand((unsigned int)time(NULL));

int randomInt(int lower, int upper);
unsigned int randomUInt(unsigned int lower, unsigned int upper);
float randomFloat(float lower, float upper);
double randomDouble(double lower, double upper);
char randomChar(int lower, int upper);