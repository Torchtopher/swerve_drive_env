'''A simple sample environment. Use this as a template for your own envs.'''

import gymnasium
import numpy as np
import math
import matplotlib.pyplot as plt
#from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas

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
        self.fig, self.ax = None, None


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
        print("Rendering")
        arr = self.c_envs.get_render_data()
        print(f"x: {arr}")
        self._base_render(arr)
        plt.pause(1/60)

    def _base_render(self, arr):
        if self.fig is None or self.ax is None:
            self.fig, self.ax = plt.subplots()
            self.ax.set_xlim(-10, 10)
            self.ax.set_ylim(-10, 10)
            self.ax.set_aspect('equal')
            self.ax.grid(True)

        self.ax.clear()
        self.ax.set_xlim(-1, 10)
        self.ax.set_ylim(-1, 10)
        self.ax.set_aspect('equal')
        self.ax.grid(True)

        x, y, theta, vx, vy, omega, goal_x, goal_y, goal_rot = arr

        # Draw the swerve drive robot as a 25x25 square
        square = np.array([
            [-12.5, -12.5],
            [12.5, -12.5],
            [12.5, 12.5],
            [-12.5, 12.5],
            [-12.5, -12.5]
        ]) / 25.0  # Scale to normalized units

        # Rotate and translate square
        rotation_matrix = np.array([
            [np.cos(theta), -np.sin(theta)],
            [np.sin(theta), np.cos(theta)]
        ])
        transformed_square = (rotation_matrix @ square.T).T + np.array([x, y])

        self.ax.plot(transformed_square[:, 0], transformed_square[:, 1], 'b')
        self.ax.scatter(x, y, c='r', label="Position")
        # plot the goal point
        self.ax.scatter(goal_x, goal_y, c='g', label="Goal")
        # plot the target angle at the goal point (target location[2])
        print(f"Goal Rot: {goal_rot}")
        self.ax.quiver(goal_x, goal_y, np.cos(math.radians(goal_rot)), np.sin(math.radians(goal_rot)), color='g', label="Goal Angle", width=0.003)
        # plot the current angle of the robot
        print(f"Robot Rot: {theta}")
        self.ax.quiver(x, y, np.cos(theta), np.sin(theta), color='r', label="Robot Angle", width=0.004, scale=30)
        # plot the velocity vector
        print(f"Vx: {vx}, Vy: {vy}")
        self.ax.quiver(x, y, vx, vy, color='b', label="Velocity", width=0.003)
        # plot ideal velocity vector 
        print(f"Goal X: {goal_x}, Goal Y: {goal_y}")
        self.ax.quiver(x, y, goal_x - x, goal_y - y, color='g', label="Ideal Velocity", width=0.003)
        # put the reward in the title
        #self.ax.set_title(f"Reward: {self.last_reward}")
        # set also the distance to the goal in the title
        self.ax.set_title(f"swerve")
        self.ax.legend()    

    def close(self):
        self.c_envs.close()


if __name__ == "__main__":
    env = Swerve()
    #env.allocate()
    env.reset()
    while True:
        env.render()