#!/usr/bin/env python3

from frc971.control_loops.swerve import dynamics
import pickle
import matplotlib.pyplot as pyplot
import matplotlib
from matplotlib import pylab
import numpy
import time
import scipy
import casadi
import os, sys
from absl import flags
from absl import app

FLAGS = flags.FLAGS
flags.DEFINE_bool('compileonly', False,
                  'If true, load casadi, don\'t compile it')
flags.DEFINE_float('vx', 1.0, 'Goal velocity in m/s in x')
flags.DEFINE_float('vy', 0.0, 'Goal velocity in m/s in y')
flags.DEFINE_float('omega', 0.0, 'Goal velocity in m/s in omega')
flags.DEFINE_float('duration', 0.5, 'Time to simulate in seconds.')
flags.DEFINE_bool('pickle', False, 'Write optimization results.')
flags.DEFINE_string('outputdir', None, 'Directory to write problem results to')

matplotlib.use("GTK3Agg")

# Full print level on ipopt. Piping to a file and using a filter or search method is suggested
# grad_x prints out the gradient at each iteration in the following sequence: U0, X1, U1, etc.
full_debug = False


class MPC(object):

    def __init__(self, solver='fatrop'):
        self.fdot = dynamics.swerve_full_dynamics(
            casadi.SX.sym("X", dynamics.NUM_STATES, 1),
            casadi.SX.sym("U", 8, 1))
        self.velocity_fdot = dynamics.velocity_swerve_physics(
            casadi.SX.sym("X", dynamics.NUM_VELOCITY_STATES, 1),
            casadi.SX.sym("U", 8, 1))

        self.wrapped_swerve_physics = lambda X, U: numpy.array(self.fdot(X, U))

        self.dt = 0.005

        # TODO(austin): Do we need a disturbance torque per module to account for friction?
        # Do it only in the observer/post?

        self.force = [
            dynamics.F(i, casadi.SX.sym("X", 25, 1), casadi.SX.sym("U", 8, 1))
            for i in range(4)
        ]
        self.force_vel = [
            dynamics.F_vel(i,
                           casadi.SX.sym("X", dynamics.NUM_VELOCITY_STATES, 1),
                           casadi.SX.sym("U", 8, 1)) for i in range(4)
        ]
        self.slip_angle = [
            dynamics.slip_angle(i, casadi.SX.sym("X", 25, 1),
                                casadi.SX.sym("U", 8, 1)) for i in range(4)
        ]
        self.mounting_location = [
            dynamics.mounting_location(
                i, casadi.SX.sym("X", dynamics.NUM_VELOCITY_STATES, 1),
                casadi.SX.sym("U", 8, 1)) for i in range(4)
        ]
        self.torque_cross = self.torque_cross_func(casadi.SX.sym("r", 2, 1),
                                                   casadi.SX.sym("F", 2, 1))
        self.force_cross = self.force_cross_func(casadi.SX.sym("Tau", 1, 1),
                                                 casadi.SX.sym("r", 2, 1))
        self.next_X = self.make_physics()
        self.cost = self.make_cost()

        self.N = 200

        # Start with an empty nonlinear program.
        self.w = []
        self.lbw = []
        self.ubw = []
        J = 0
        self.g = []
        self.lbg = []
        self.ubg = []

        self.X0 = casadi.MX.sym('X0', dynamics.NUM_VELOCITY_STATES)

        # We care about the linear and angular velocities only.
        self.R = casadi.MX.sym('R', 3)

        # Make Xn and U for each step.  fatrop wants us to interleave the control variables and
        # states so that it can produce a banded/structured problem which it can solve a lot
        # faster.
        Xn_variables = []
        U_variables = []
        for i in range(self.N):
            U_variables.append(casadi.MX.sym(f'U{i}', 8))

            if i == 0:
                continue

            Xn_variables.append(
                casadi.MX.sym(f'X{i}', dynamics.NUM_VELOCITY_STATES))

        Xn = casadi.horzcat(*Xn_variables)
        U = casadi.horzcat(*U_variables)

        # printme(number) is the debug.
        Xk_begin = casadi.horzcat(self.X0, Xn)
        Xk_end = self.next_X.map(self.N, "thread")(Xk_begin, U)
        J = casadi.sum2(self.cost.map(self.N, "thread")(Xk_end, U, self.R))

        # Put U and Xn interleaved into w to go fast.
        for i in range(self.N):
            self.w += [U_variables[i]]
            self.ubw += [100] * 8
            self.lbw += [-100] * 8

            if i == self.N - 1:
                continue

            self.w += [Xn_variables[i]]
            self.ubw += [casadi.inf] * dynamics.NUM_VELOCITY_STATES
            self.lbw += [-casadi.inf] * dynamics.NUM_VELOCITY_STATES

        self.g += [
            casadi.reshape(Xn - Xk_end[:, 0:(self.N - 1)],
                           dynamics.NUM_VELOCITY_STATES * (self.N - 1), 1)
        ]

        self.lbg += [0] * dynamics.NUM_VELOCITY_STATES * (self.N - 1)
        self.ubg += [0] * dynamics.NUM_VELOCITY_STATES * (self.N - 1)

        prob = {
            'f': J,
            # lbx <= x <= ubx
            'x': casadi.vertcat(*self.w),
            # lbg <= g(x, p) <= ubg
            'g': casadi.vertcat(*self.g),
            # Input parameters (initial position + goal)
            'p': casadi.vertcat(self.X0, self.R),
        }

        compiler = "ccache clang"
        flags = ["-O3"]
        jit_options = {
            "flags": flags,
            "verbose": False,
            "compiler": compiler,
            "temp_suffix": False,
        }

        if solver == 'fatrop':
            equality = [
                True
                for _ in range(dynamics.NUM_VELOCITY_STATES * (self.N - 1))
            ]
            options = {
                "jit": True,
                "jit_cleanup": False,
                "jit_temp_suffix": False,
                "compiler": "shell",
                "jit_options": jit_options,
                "structure_detection": "auto",
                "fatrop": {
                    "tol": 1e-7
                },
                "debug": True,
                "equality": equality,
            }
        else:
            options = {
                "jit": True,
                "jit_cleanup": False,
                "jit_temp_suffix": False,
                "compiler": "shell",
                "jit_options": jit_options,
            }
            if full_debug:
                options["jit"] = False
                options["ipopt"] = {
                    "print_level": 12,
                }

        self.solver = casadi.nlpsol('solver', solver, prob, options)

    # TODO(austin): Vary the number of sub steps to be more short term and fewer long term?
    def make_physics(self):
        X0 = casadi.MX.sym('X0', dynamics.NUM_VELOCITY_STATES)
        U = casadi.MX.sym('U', 8)

        X = X0
        M = 2  # RK4 steps per interval
        DT = self.dt / M

        for j in range(M):
            k1 = self.velocity_fdot(X, U)
            k2 = self.velocity_fdot(X + DT / 2 * k1, U)
            k3 = self.velocity_fdot(X + DT / 2 * k2, U)
            k4 = self.velocity_fdot(X + DT * k3, U)
            X = X + DT / 6 * (k1 + 2 * k2 + 2 * k3 + k4)

        return casadi.Function("next_X", [X0, U], [X])

    def make_cost(self):
        # TODO(austin): tune cost fn?
        # Do we want to penalize slipping tires?
        # Do we want to penalize powers unevenly applied across the motors?
        # Need to do some simulations to see what works well.

        X = casadi.MX.sym('X', dynamics.NUM_VELOCITY_STATES)
        U = casadi.MX.sym('U', 8)
        R = casadi.MX.sym('R', 3)

        J = 0
        vnorm = casadi.sqrt(R[0]**2.0 + R[1]**2.0)

        vnormx = casadi.if_else(vnorm > 0.0001, R[0] / vnorm, 1.0)
        vnormy = casadi.if_else(vnorm > 0.0001, R[1] / vnorm, 0.0)

        vperpx = casadi.if_else(vnorm > 0.0001, -vnormy, 0.0)
        vperpy = casadi.if_else(vnorm > 0.0001, vnormx, 1.0)

        # TODO(austin): Do we want to do something more special for 0?

        # cost velocity a lot more in the perpendicular direction to allow tire to spin up
        # we also only want to get moving in the correct direction as fast as possible
        J += 75 * ((R[0] - X[dynamics.VELOCITY_STATE_VX]) * vnormx +
                   (R[1] - X[dynamics.VELOCITY_STATE_VY]) * vnormy)**2.0

        J += 1500 * ((R[0] - X[dynamics.VELOCITY_STATE_VX]) * vperpx +
                     (R[1] - X[dynamics.VELOCITY_STATE_VY]) * vperpy)**2.0

        J += 1000 * (R[2] - X[dynamics.VELOCITY_STATE_OMEGA])**2.0

        kSteerPositionGain = 0
        kSteerVelocityGain = 0.10
        J += kSteerPositionGain * (X[dynamics.VELOCITY_STATE_THETAS0])**2.0
        J += kSteerVelocityGain * (X[dynamics.VELOCITY_STATE_OMEGAS0])**2.0

        J += kSteerPositionGain * (X[dynamics.VELOCITY_STATE_THETAS1])**2.0
        J += kSteerVelocityGain * (X[dynamics.VELOCITY_STATE_OMEGAS1])**2.0

        J += kSteerPositionGain * (X[dynamics.VELOCITY_STATE_THETAS2])**2.0
        J += kSteerVelocityGain * (X[dynamics.VELOCITY_STATE_OMEGAS2])**2.0

        J += kSteerPositionGain * (X[dynamics.VELOCITY_STATE_THETAS3])**2.0
        J += kSteerVelocityGain * (X[dynamics.VELOCITY_STATE_OMEGAS3])**2.0

        # cost variance of the force by a tire and the expected average force and torque on it
        total_force = self.force_vel[0](X, U)
        total_torque = self.torque_cross(self.mounting_location[0](X, U),
                                         self.force_vel[0](X, U))
        for i in range(3):
            total_force += self.force_vel[i + 1](X, U)
            total_torque += self.torque_cross(
                self.mounting_location[i + 1](X, U), self.force_vel[i + 1](X,
                                                                           U))

        total_force /= 4
        total_torque /= 4
        for i in range(4):
            f_diff = (total_force +
                      self.force_cross(total_torque, self.mounting_location[i]
                                       (X, U))) - self.force_vel[i](X, U)
            J += 0.01 * (f_diff[0, 0]**2.0 + f_diff[1, 0]**2.0)

        # TODO(austin): Don't penalize torque steering current.
        for i in range(4):
            Is = U[2 * i + 0]
            Id = U[2 * i + 1]
            # Steer, cost it a lot less than drive to be more agressive in steering
            J += ((Is + dynamics.STEER_CURRENT_COUPLING_FACTOR * Id)**
                  2.0) / 100000.0
            # Drive
            J += Id * Id / 1000.0

        return casadi.Function("Jn", [X, U, R], [J])

    def torque_cross_func(self, r, F):
        result = casadi.SX.sym('Tau', 1, 1)
        result[0, 0] = r[0, 0] * F[1, 0] - r[1, 0] * F[0, 0]
        return casadi.Function('Tau', [r, F], [result])

    def force_cross_func(self, Tau, r):
        result = casadi.SX.sym('F', 2, 1)
        result[0, 0] = -r[1, 0] * Tau[0, 0] / casadi.norm_2(r)**2.0
        result[1, 0] = r[0, 0] * Tau[0, 0] / casadi.norm_2(r)**2.0
        return casadi.Function('F', [Tau, r], [result])

    def solve(self, p, seed=None):
        if seed is None:
            seed = []

            for i in range(self.N):
                seed += [0, 0] * 4
                if i < self.N - 1:
                    seed += list(p[:dynamics.NUM_VELOCITY_STATES, 0])

        return self.solver(x0=seed,
                           lbx=self.lbw,
                           ubx=self.ubw,
                           lbg=self.lbg,
                           ubg=self.ubg,
                           p=casadi.DM(p))

    def unpack_u(self, sol, i):
        return sol['x'].full().flatten()[
            (8 + dynamics.NUM_VELOCITY_STATES) *
            i:((8 + dynamics.NUM_VELOCITY_STATES) * i + 8)]

    def unpack_x(self, sol, i):
        return sol['x'].full().flatten(
        )[8 + (8 + dynamics.NUM_VELOCITY_STATES) *
          (i - 1):(8 + dynamics.NUM_VELOCITY_STATES) * i]


class Solver(object):

    def __init__(self):
        self.iterations = int(round(FLAGS.duration / 0.005))

        self.X_plot = numpy.zeros((25, self.iterations))
        self.U_plot = numpy.zeros((8, self.iterations))
        self.t = []

    def solve(self, mpc, X_initial, R_goal, debug=False):
        X = X_initial.copy()

        if debug:
            pyplot.ion()
            fig0, axs0 = pylab.subplots(2)
            fig1, axs1 = pylab.subplots(2)

        last_time = time.time()

        seed = ([0, 0] * 4 +
                list(dynamics.to_velocity_state(X))) * (mpc.N - 1) + [0, 0] * 4

        overall_time = 0
        for i in range(self.iterations):
            self.t.append(i * mpc.dt)
            print("Current X at", i * mpc.dt, X.transpose())
            print("Goal R at", i * mpc.dt, R_goal)
            start_time = time.time()
            sol = mpc.solve(
                # TODO(austin): Is this better or worse than constraints on the initial state for convergence?
                p=numpy.vstack((dynamics.to_velocity_state(X), R_goal)),
                seed=seed)
            end_time = time.time()
            print(f"Took {end_time - start_time} seconds to solve.")
            overall_time += end_time - start_time

            self.X_plot[:, i] = X[:, 0]

            U = mpc.unpack_u(sol, 0)
            seed = (list(
                sol['x'].full().flatten()[8 + dynamics.NUM_VELOCITY_STATES:]) +
                    list(sol['x'].full().flatten()
                         [-(8 + dynamics.NUM_VELOCITY_STATES):]))
            self.U_plot[:, i] = U

            print('x(0):', X.transpose())
            for j in range(mpc.N):
                print(f'u({j}): ', mpc.unpack_u(sol, j))
                print(f'x({j+1}): ', mpc.unpack_x(sol, j + 1))

            result = scipy.integrate.solve_ivp(
                lambda t, x: mpc.wrapped_swerve_physics(x, U).flatten(),
                [0, mpc.dt], X.flatten())
            X[:, 0] = result.y[:, -1]

            if time.time() > last_time + 2 or i == self.iterations - 1:
                if debug:
                    axs0[0].clear()
                    axs0[1].clear()

                    axs0[0].plot(self.t,
                                 self.X_plot[dynamics.STATE_VX, 0:i + 1],
                                 label="vx")
                    axs0[0].plot(self.t,
                                 self.X_plot[dynamics.STATE_OMEGA, 0:i + 1],
                                 label="omega")
                    axs0[0].plot(self.t,
                                 self.X_plot[dynamics.STATE_VY, 0:i + 1],
                                 label="vy")
                    axs0[0].legend()

                    axs0[1].plot(self.t, self.U_plot[0, 0:i + 1], label="Is0")
                    axs0[1].plot(self.t, self.U_plot[1, 0:i + 1], label="Id0")
                    axs0[1].legend()

                    axs1[0].clear()
                    axs1[1].clear()

                    axs1[0].plot(self.t,
                                 self.X_plot[dynamics.STATE_THETAS0, 0:i + 1],
                                 label='steer0')
                    axs1[0].plot(self.t,
                                 self.X_plot[dynamics.STATE_THETAS1, 0:i + 1],
                                 label='steer1')
                    axs1[0].plot(self.t,
                                 self.X_plot[dynamics.STATE_THETAS2, 0:i + 1],
                                 label='steer2')
                    axs1[0].plot(self.t,
                                 self.X_plot[dynamics.STATE_THETAS3, 0:i + 1],
                                 label='steer3')
                    axs1[0].legend()
                    axs1[1].plot(self.t,
                                 self.X_plot[dynamics.STATE_OMEGAS0, 0:i + 1],
                                 label='steer_velocity0')
                    axs1[1].plot(self.t,
                                 self.X_plot[dynamics.STATE_OMEGAS1, 0:i + 1],
                                 label='steer_velocity1')
                    axs1[1].plot(self.t,
                                 self.X_plot[dynamics.STATE_OMEGAS2, 0:i + 1],
                                 label='steer_velocity2')
                    axs1[1].plot(self.t,
                                 self.X_plot[dynamics.STATE_OMEGAS3, 0:i + 1],
                                 label='steer_velocity3')
                    axs1[1].legend()
                    pyplot.pause(0.0001)

                last_time = time.time()

        print(f"Tool {overall_time} seconds overall to solve.")


def main(argv):
    if FLAGS.outputdir:
        os.chdir(FLAGS.outputdir)

    module_velocity = 0.0

    X_initial = numpy.zeros((25, 1))
    # All the wheels are spinning at the speed needed to hit 1 m/s
    X_initial[dynamics.STATE_THETAS0, 0] = 0.0
    X_initial[dynamics.STATE_OMEGAS0, 0] = module_velocity

    X_initial[dynamics.STATE_THETAS1, 0] = 0.0
    X_initial[dynamics.STATE_OMEGAS1, 0] = module_velocity

    X_initial[dynamics.STATE_THETAS2, 0] = 0.0
    X_initial[dynamics.STATE_OMEGAS2, 0] = module_velocity

    X_initial[dynamics.STATE_THETAS3, 0] = 0.0
    X_initial[dynamics.STATE_OMEGAS3, 0] = module_velocity

    # Robot is moving at 0 m/s
    X_initial[dynamics.STATE_VX, 0] = 0.0
    X_initial[dynamics.STATE_VY, 0] = 0.0
    # No angular velocity
    X_initial[dynamics.STATE_OMEGA, 0] = 0.0

    R_goal = numpy.zeros((3, 1))
    R_goal[0, 0] = FLAGS.vx
    R_goal[1, 0] = FLAGS.vy
    R_goal[2, 0] = FLAGS.omega

    mpc = MPC(solver='fatrop') if not full_debug else MPC(solver='ipopt')
    solver = Solver()
    if not FLAGS.compileonly:
        results = solver.solve(mpc=mpc,
                               X_initial=X_initial,
                               R_goal=R_goal,
                               debug=(FLAGS.pickle == False))
    else:
        return 0

    if FLAGS.pickle:
        with open('t.pkl', 'wb') as f:
            pickle.dump(solver.t, f)
        with open('X_plot.pkl', 'wb') as f:
            pickle.dump(solver.X_plot, f)
        with open('U_plot.pkl', 'wb') as f:
            pickle.dump(solver.U_plot, f)

        fig0, axs0 = pylab.subplots(2)
        fig1, axs1 = pylab.subplots(2)

        axs0[0].clear()
        axs0[1].clear()

        axs0[0].plot(solver.t, solver.X_plot[dynamics.STATE_VX, :], label="vx")
        axs0[0].plot(solver.t, solver.X_plot[dynamics.STATE_VY, :], label="vy")
        axs0[0].legend()

        axs0[1].plot(solver.t, solver.U_plot[0, :], label="Is0")
        axs0[1].plot(solver.t, solver.U_plot[1, :], label="Id0")
        axs0[1].legend()

        axs1[0].clear()
        axs1[1].clear()

        axs1[0].plot(solver.t,
                     solver.X_plot[dynamics.STATE_THETAS0, :],
                     label='steer0')
        axs1[0].plot(solver.t,
                     solver.X_plot[dynamics.STATE_THETAS1, :],
                     label='steer1')
        axs1[0].plot(solver.t,
                     solver.X_plot[dynamics.STATE_THETAS2, :],
                     label='steer2')
        axs1[0].plot(solver.t,
                     solver.X_plot[dynamics.STATE_THETAS3, :],
                     label='steer3')
        axs1[0].legend()
        axs1[1].plot(solver.t,
                     solver.X_plot[dynamics.STATE_OMEGAS0, :],
                     label='steer_velocity0')
        axs1[1].plot(solver.t,
                     solver.X_plot[dynamics.STATE_OMEGAS1, :],
                     label='steer_velocity1')
        axs1[1].plot(solver.t,
                     solver.X_plot[dynamics.STATE_OMEGAS2, :],
                     label='steer_velocity2')
        axs1[1].plot(solver.t,
                     solver.X_plot[dynamics.STATE_OMEGAS3, :],
                     label='steer_velocity3')
        axs1[1].legend()

        fig0.savefig('state.svg')
        fig1.savefig('steer.svg')


if __name__ == '__main__':
    app.run(main)
