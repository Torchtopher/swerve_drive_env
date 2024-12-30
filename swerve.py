'''A simple sample environment. Use this as a template for your own envs.'''

import gymnasium
import numpy as np

import pufferlib
from cy_swerve import CySwerve

class Swerve(pufferlib.PufferEnv):
    def __init__(self, num_envs=2, render_mode=None, size=11, buf=None):

        self.single_observation_space = gymnasium.spaces.Box(low=0, high=1,
            shape=(6,), dtype=np.float32)
        self.single_action_space = gymnasium.spaces.Box(low=-1, high=1,
            shape=(3,), dtype=np.float32
        )
        self.render_mode = render_mode
        self.num_agents = num_envs

        super().__init__(buf)
        print(self.actions.shape)
        print(self.actions)
        print(self.observations)
        self.c_envs = CySwerve(self.observations, self.actions,
            self.rewards, self.terminals, num_envs)
 
    def reset(self, seed=None):
        self.c_envs.reset()
        return self.observations, []

    def step(self, actions):
        self.actions[:] = actions
        self.c_envs.step()

        episode_returns = self.rewards[self.terminals]

        info = []
        if len(episode_returns) > 0:
            info = [{
                'reward': np.mean(episode_returns),
            }]

        return (self.observations, self.rewards,
            self.terminals, self.truncations, info)

    def render(self):
        x,y,angle,vx,vy = self.c_envs.get_render_data()
        print(f"x: {x}, y: {y}, angle: {angle}, vx: {vx}, vy: {vy}")

    def close(self):
        self.c_envs.close()

if __name__ == "__main__":
    env = Swerve()
    env.render()