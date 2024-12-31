from libc.stdlib cimport calloc, free
from cython.operator import dereference 

cdef extern from "swerve.h":
    ctypedef struct Swerve:
        float* observations
        float* contActions
        float* rewards
        unsigned char* terminals
        int tick


    void reset(Swerve* env)
    void step(Swerve* env)
    void allocate(Swerve* env)
    float* get_render_data(Swerve* env)

cdef class CySwerve:
    cdef:
        Swerve* envs
        int num_envs

    def __init__(self, float[:, :] observations, float[:, :] actions,
            float[:] rewards, unsigned char[:] terminals, int num_envs):

        self.envs = <Swerve*> calloc(num_envs, sizeof(Swerve))
        self.num_envs = num_envs

        cdef int i
        for i in range(num_envs):
            self.envs[i] = Swerve(
                observations = &observations[i, 0],
                contActions = &actions[i, 0],
                rewards = &rewards[i],
                terminals = &terminals[i],
            )

    def allocate(self): 
        cdef int i
        for i in range(self.num_envs):
            allocate(&self.envs[i])

    def reset(self):
        cdef int i
        for i in range(self.num_envs):
            reset(&self.envs[i])

    def step(self):
        cdef int i
        for i in range(self.num_envs):
            step(&self.envs[i])

    # def render(self):
    #     cdef Swerve* env = &self.envs[0]
    #     if self.client == NULL:
    #         self.client = make_client(env)

    #     render(self.client, env)

    def get_render_data(self):
        cdef float* ptr = get_render_data(&self.envs[0])
        cdef int i
        lst=[]
        for i in range(9):
            lst.append(ptr[i])
        return lst
        

    def close(self):
        free(self.envs)
