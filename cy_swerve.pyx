from libc.stdlib cimport calloc, free

cdef extern from "swerve.h":
    ctypedef struct Swerve:
        float* observations
        float* contActions
        float* rewards
        unsigned char* terminals
        int tick


    void reset(Swerve* env)
    void step(Swerve* env)

cdef class CySwerve:
    cdef:
        Swerve* envs
        int num_envs

    def __init__(self, float[:] observations, float[:] actions,
            float[:] rewards, unsigned char[:] terminals, int num_envs):

        self.envs = <Swerve*> calloc(num_envs, sizeof(Swerve))
        self.num_envs = num_envs

        cdef int i
        for i in range(num_envs):
            self.envs[i] = Swerve(
                observations = &observations[i],
                contActions = &actions[i],
                rewards = &rewards[i],
                terminals = &terminals[i],
            )

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

    def close(self):
        free(self.envs)
