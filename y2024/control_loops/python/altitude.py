#!/usr/bin/python3

from aos.util.trapezoid_profile import TrapezoidProfile
from frc971.control_loops.python import control_loop
from frc971.control_loops.python import angular_system
from frc971.control_loops.python import controls
import numpy
import sys
from matplotlib import pylab
import gflags
import glog

FLAGS = gflags.FLAGS

try:
    gflags.DEFINE_bool('plot', False, 'If true, plot the loop response.')
except gflags.DuplicateFlagError:
    pass

gflags.DEFINE_bool('hybrid', False, 'If true, make it hybrid.')

kAltitude = angular_system.AngularSystemParams(
    name='Altitude',
    motor=control_loop.KrakenX60FOC(),
    G=(16.0 / 60.0) * (16.0 / 162.0),
    # 4340 in^ lb
    J=1.2,
    q_pos=0.40,
    q_vel=8.0,
    kalman_q_pos=0.12,
    kalman_q_vel=2.0,
    kalman_q_voltage=3.0,
    kalman_r_position=0.05,
    radius=10.5 * 0.0254,
    dt=0.005)


def main(argv):
    if FLAGS.plot:
        R = numpy.matrix([[numpy.pi / 2.0], [0.0]])
        angular_system.PlotKick(kAltitude, R)
        angular_system.PlotMotion(kAltitude, R)
        return

    # Write the generated constants out to a file.
    if len(argv) != 7:
        glog.fatal(
            'Expected .h file name and .cc file name for the intake and integral intake.'
        )
    else:
        namespaces = ['y2024', 'control_loops', 'superstructure', 'altitude']
        angular_system.WriteAngularSystem(kAltitude, argv[1:4], argv[4:7],
                                          namespaces)


if __name__ == '__main__':
    argv = FLAGS(sys.argv)
    glog.init()
    sys.exit(main(argv))
