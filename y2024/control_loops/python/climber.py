#!/usr/bin/python3

from frc971.control_loops.python import control_loop
from frc971.control_loops.python import linear_system
import numpy
import sys
import gflags
import glog

FLAGS = gflags.FLAGS

try:
    gflags.DEFINE_bool('plot', False, 'If true, plot the loop response.')
except gflags.DuplicateFlagError:
    pass

kClimber = linear_system.LinearSystemParams(
    name='Climber',
    motor=control_loop.KrakenX60FOC(),
    G=(8. / 60.) * (16. / 60.),
    radius=16 * 0.25 / numpy.pi / 2.0 * 0.0254,
    mass=2.0,
    q_pos=0.03,
    q_vel=2.,
    kalman_q_pos=0.12,
    kalman_q_vel=2.00,
    kalman_q_voltage=35.0,
    kalman_r_position=0.05,
    dt=0.005,
)


def main(argv):
    if FLAGS.plot:
        R = numpy.matrix([[0.4], [0.0]])
        linear_system.PlotMotion(kClimber,
                                 R,
                                 max_velocity=5.0,
                                 plant_params=kClimber)
        linear_system.PlotKick(kClimber, R, plant_params=kClimber)
        return

    # Write the generated constants out to a file.
    if len(argv) != 7:
        glog.fatal(
            'Expected .h file name and .cc file name for the climber and integral climber.'
        )
    else:
        namespaces = ['y2024', 'control_loops', 'superstructure', 'climber']
        linear_system.WriteLinearSystem(kClimber, argv[1:4], argv[4:7],
                                        namespaces)


if __name__ == '__main__':
    argv = FLAGS(sys.argv)
    glog.init()
    sys.exit(main(argv))
