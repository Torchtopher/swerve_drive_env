#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

#define OBS_VX 3
#define OBS_VY 4
#define OBS_OMEGA 5
#define OBS_GOAL_REL_X 0
#define OBS_GOAL_REL_Y 1
#define OBS_GOAL_REL_ANGLE 2

typedef struct Swerve Swerve;

struct Swerve {
    float* observations; // 1d array, assuming goal is 0,0 [rel_x_goal, rel_y_goal, rel_theta_goal, x_vel, y_vel, omega]
    float* contActions; // 1d array, [x_accel, y_accel, angular_aceel]
    float* rewards; // just had [rewards]
    unsigned char* terminals;
    float* render_info; // 1d array has all of obs plus x_pos, y_pos and angle


    float x_pos; 
    float y_pos;
    float angle;
    float x_vel;
    float y_vel; 
    float omega;

    float goal_x_pos;
    float goal_y_pos;
    float goal_angle;

    int tick;
};

void allocate(Swerve* env) {
    printf("Allocating\n");
    //fflush(NULL); 
    env->observations = (float*)calloc(NUM_OBS, sizeof(float));
    env->contActions = (float*)calloc(NUM_ACTIONS, sizeof(float));
    env->rewards = (float*)calloc(1, sizeof(float));
    env->terminals = (unsigned char*)calloc(1, sizeof(unsigned char));
    env->render_info = (float*)calloc(9, sizeof(float));
}

void free_allocated(Swerve* env) {
    free(env->observations);
    free(env->contActions);
    free(env->rewards);
    free(env->terminals);
    free(env->render_info); 
}

void reset(Swerve* env) {
    printf("Reseting env");
    //fflush(NULL); 
    memset(env->observations, 0, NUM_OBS*sizeof(float));
    env->tick = 0;
    env->x_pos = MAX_X / 2;
    env->y_pos = MAX_Y / 2;
    env->angle = 0;

    env->x_vel = 0;
    env->y_vel = 0;
    env->omega = 0;
    
    env->goal_x_pos = 9;
    env->goal_y_pos = 9;
    env->goal_angle = 90;

    // ADD LOGIC FOR RESETING HERE
}

int step(Swerve* env) {
    printf("C: Step\n");
    env->terminals[0] = 0;
    env->rewards[0] = 5;

    env->x_vel += env->contActions[0] * dt;
    env->y_vel += env->contActions[1] * dt;
    env->omega += env->contActions[2] * dt;

    env->x_pos += env->x_vel * dt;
    env->y_pos += env->y_vel * dt;
    env->angle += env->omega * dt;

    env->observations[OBS_VX] = env->x_vel;
    env->observations[OBS_VY] = env->y_vel;
    env->observations[OBS_OMEGA] = env->omega;
    env->observations[OBS_GOAL_REL_X] = env->goal_x_pos - env->x_pos;  
    env->observations[OBS_GOAL_REL_Y] = env->goal_y_pos - env->y_pos;
    env->observations[OBS_GOAL_REL_ANGLE] = env->goal_angle - env->angle;  

    if (env->tick > 300) {
        reset(env);
        return 99;
    }

    env->tick += 1;
    return 99; 
}

float* get_render_data(Swerve* env) {
    printf("Geting render data\n");
    //fflush(NULL); 
    // xyangle
    env->render_info[0] = env->x_pos;
    env->render_info[1] = env->y_pos;
    env->render_info[2] = env->angle;
// 
    // // vxvyomega
    env->render_info[3] = env->observations[OBS_VX];
    env->render_info[4] = env->observations[OBS_VY];
    env->render_info[5] = env->observations[OBS_OMEGA];
// 
    // // goal xyangle
    env->render_info[6] = env->goal_x_pos;
    env->render_info[7] = env->goal_y_pos;
    env->render_info[8] = env->goal_angle;
    // copy? 
    //int i;
    //for (i=0;i < 9;i++) {
    //    printf("%lf\n",env->render_info[i]);
    //}
    return env->render_info;
}
