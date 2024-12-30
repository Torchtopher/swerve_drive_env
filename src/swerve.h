#include <stdlib.h>
#include <string.h>

const unsigned char NOOP = 0;
const unsigned char DOWN = 1;
const unsigned char UP = 2;
const unsigned char LEFT = 3;
const unsigned char RIGHT = 4;

const int MAX_X = 10;
const int MAX_Y = 10;
const int MIN_X = 0; 
const int MIN_Y = 0;
const float dt = 1/60;

const int NUM_OBS = 6;
const int NUM_ACTIONS = 3;

typedef struct Swerve Swerve;

struct Swerve {
    float* observations; // 1d array, assuming goal is 0,0 [rel_x_goal, rel_y_goal, rel_theta_goal, x_vel, y_vel, omega]
    float* contActions; // 1d array, [x_accel, y_accel, angular_aceel]
    float* rewards; // just had [rewards]
    unsigned char* terminals;
    
    float x_pos; 
    float y_pos;
    float angle;
    float x_vel;
    float y_vel; 
    float omega;

    int tick;
};

void allocate(Swerve* env) {
    env->observations = (float*)calloc(NUM_OBS, sizeof(float));
    env->contActions = (float*)calloc(NUM_ACTIONS, sizeof(float));
    env->rewards = (float*)calloc(1, sizeof(float));
    env->terminals = (unsigned char*)calloc(1, sizeof(unsigned char));
}

void free_allocated(Swerve* env) {
    free(env->observations);
    free(env->contActions);
    free(env->rewards);
    free(env->terminals);
}

void reset(Swerve* env) {
    memset(env->observations, 0, NUM_OBS*sizeof(float));
    env->tick = 0;
    env->x_pos = MAX_X / 2;
    env->y_pos = MAX_Y / 2;
    // ADD LOGIC FOR RESETING HERE
}

void step(Swerve* env) {
    env->tick += 1;
}

