#!/usr/bin/env python3
# -*- python -*-
# -*- coding: utf-8 -*-

# 1st experiment
# Look at Vz (Settling time, Overshoot, rise time, final value for Steady-State error,
#
# 2nd experiment
# Look at Va (Settling time, Overshoot, rise time, final value for Steady-State error,
#
# 3rd experiment
# Look at h (Overshoot, Steady-State error)

import csv
import argparse
import sys

def CommentStripper (iterator):
    for line in iterator:
        if line [:1] == '#':
            continue
        if not line.strip ():
            continue
        yield line

if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='Check functional properties.')
    parser.add_argument('csv_file',
                        type=argparse.FileType('r'),
                        help='The file containing results in CSV format.')
    parser.add_argument('--check-flight-mode',dest='flight_mode',
                        default='LevelChange',
                        choices=['VerticalSpeed','AirSpeed','LevelChange'],
                        help='Indicate which flight mode to check for. (default: %(default)s)')
    parser.add_argument('--enable-graph',dest='withGraph',action='store_true',default=False,
                        help='Enable graphical output for easier visual check (requires matplotlib)')

    args = parser.parse_args()

    reader = csv.reader(CommentStripper(args.csv_file))

    data = [[float(item) for number, item in enumerate(row)] for row in reader]

    if args.flight_mode.lower() == "verticalspeed":
        print("Checking for Vertical Speed control...")

        # Vz maximum value 
        max_Vz = max(row[4] for row in data)
        
        # Vz steady-state value (if system is stable)
        steady_state_Vz = (data[-1][4])
        
        # 5% of Vz steady-state value 
        settling_time_5_value  = abs(0.05*steady_state_Vz)
        
        # 10% of Vz steady-state value
        rise_time_10_value     = 0.1*steady_state_Vz
        
        # 90% of Vz steady-state value
        rise_time_90_value     = 0.9*steady_state_Vz

        # Finding the time where Vz attains 10% of Vz steady-state value
        indexes = [i for i,x in enumerate(row[4] for row in data) if x < rise_time_10_value]
        rise_time_10 = data[indexes[-1]][0]

        # Finding the time where Vz attains 90% of Vz steady-state value
        indexes = [i for i,x in enumerate(row[4] for row in data) if x < rise_time_90_value]
        rise_time_90 = data[indexes[-1]][0]

        # Rise-time computation
        rise_time = rise_time_90 - rise_time_10

        # Finding the settling-time where Vz remains within 5% of Vz steady-state value
        indexes = [i for i,x in enumerate(row[4] for row in data) if abs(x-steady_state_Vz) > settling_time_5_value]
        settling_time = data[indexes[-1]][0]

        # Overshoot in %
        overshoot = (max_Vz - steady_state_Vz)/(steady_state_Vz*100)

        # Steady-state error in %
        steady_state_error = (2.5 - steady_state_Vz)*100/(2.5)

        print('Properties for Vz output')
        print('P1 (settling time      ) =',settling_time,'s')
        print('P2 (overshoot          ) =',overshoot,'%')
        print('P3 (rise time          ) =',rise_time,'s')
        print('P4 (steady state error ) =',abs(steady_state_error),'%')

        # Begin Figure
        if args.withGraph:
            import numpy as np
            import matplotlib.pyplot as plt

            time_value = [row[0] for row in data]
            Va_value   = [row[1] for row in data]
            Vz_value   = [row[4] for row in data]

            plt.figure(1)

            plt.subplot(121)
            plt.plot(time_value,Va_value,'b',linewidth=2.0)
            plt.axis([time_value[0],time_value[-1],229,231])
            plt.grid()
            plt.xlabel('Time (s)')
            plt.ylabel('Airspeed (m/s)')

            plt.subplot(122)
            plt.plot((time_value[0],time_value[-1]),(0.95*steady_state_Vz,0.95*steady_state_Vz),'b--')
            plt.plot((time_value[0],time_value[-1]),(1.05*steady_state_Vz,1.05*steady_state_Vz),'b--')
            plt.plot(time_value,Vz_value,'b',linewidth=2.0)
            plt.grid()
            plt.xlabel('Time (s)')
            plt.ylabel('Vertical Speed (m/s)')

            plt.show()
        # End Figure

    elif args.flight_mode.lower() == "airspeed":
        print("Checking for Airspeed control...")

        # We are only interested in the deviation from the initial condition Va = 230 m/s
        # Va maximum deviation = (maximum value - initial value)
        max_Va = max(row[1] for row in data)-230

        # Va steady-state deviation = (steady-state value - initial value) (if system is stable)
        steady_state_Va = (data[-1][1])-230

        # 5% of Va steady-state deviation
        settling_time_5_value  = abs(0.05*steady_state_Va)

        # 10% of Va steady-state deviation + initial value
        rise_time_10_value     = 0.1*steady_state_Va + 230

        # 90% of Va steady-state deviation + initial value
        rise_time_90_value     = 0.9*steady_state_Va + 230

        # Finding the time where Va attains 10% of Va steady-state deviation
        indexes = [i for i,x in enumerate(row[1] for row in data) if x < rise_time_10_value]
        rise_time_10 = data[indexes[-1]][0]

        # Finding the time where Va attains 90% of Va steady-state deviation
        indexes = [i for i,x in enumerate(row[1] for row in data) if x < rise_time_90_value]
        rise_time_90 = data[indexes[-1]][0]

        # Rise-time
        rise_time = rise_time_90 - rise_time_10

        # Finding the settling-time where Va remains within 5% of Va steady-state deviation
        indexes = [i for i,x in enumerate(row[1] for row in data) if abs(x-(steady_state_Va+230)) > settling_time_5_value]
        settling_time = data[indexes[-1]][0]

        # Overshoot in %
        overshoot = (max_Va - steady_state_Va)/(steady_state_Va*100)

        # Steady-state error in %
        steady_state_error = (5 - steady_state_Va)*100/(5)

        print('Properties for Va output')
        print('P1 (settling time      ) =',settling_time,'s')
        print('P2 (overshoot          ) =',overshoot,'%')
        print('P3 (rise time          ) =',rise_time,'s')
        print('P4 (steady state error ) =',abs(steady_state_error),'%')

        # Begin Figure
        if args.withGraph:
                import numpy as np
                import matplotlib.pyplot as plt
                time_value = [row[0] for row in data]
                Va_value   = [row[1] for row in data]
                Vz_value   = [row[4] for row in data]

                plt.figure(1)

                plt.subplot(121)
                plt.plot((time_value[0],time_value[-1]),(0.95*steady_state_Va+230,0.95*steady_state_Va+230),'b--')
                plt.plot((time_value[0],time_value[-1]),(1.05*steady_state_Va+230,1.05*steady_state_Va+230),'b--')
                plt.plot(time_value,Va_value,'b',linewidth=2.0)
                plt.grid()
                plt.xlabel('Time (s)')
                plt.ylabel('Airspeed (m/s)')
                plt.subplot(122)
                plt.plot(time_value,Vz_value,'b',linewidth=2.0)
                plt.axis([time_value[0],time_value[-1],-0.25,0.25])
                plt.grid()
                plt.xlabel('Time (s)')
                plt.ylabel('Vertical Speed (m/s)')

                plt.show()
        # End Figure

    elif args.flight_mode.lower() == "levelchange":
        print("Checking for Flight Level Change control...")

        # We are only interested in the deviation from the initial condition h = 10000 m
        # h maximum deviation = (maximum value - initial value)
        max_h = max(row[5] for row in data)

        # h steady-state deviation = (steady-state value - initial value) (if system is stable)
        steady_state_h = (data[-1][5])

        # Overshoot in %
        overshoot = (max_h - steady_state_h)*100/(1000)

        # Steady-state error in %
        steady_state_error = (11000 - steady_state_h)*100/1000

        print('Properties for h output')
        print('P2 (overshoot          ) =',overshoot,'%')
        print('P4 (steady state error ) =',abs(steady_state_error),'%')

        # Begin Figure
        if args.withGraph:
                import numpy as np
                import matplotlib.pyplot as plt

                time_value = [row[0] for row in data]
                Va_value   = [row[1] for row in data]
                Vz_value   = [-row[4] for row in data]
                h_value   =  [row[5]/1000 for row in data]

                plt.figure(1)

                plt.subplot(221)
                plt.plot(time_value,Va_value,'b',linewidth=2.0)
                plt.axis([time_value[0],time_value[-1],229,231])
                plt.grid()
                plt.xlabel('Time (s)')
                plt.ylabel('Airspeed (m/s)')

                plt.subplot(222)
                plt.plot(time_value,Vz_value,'b',linewidth=2.0)
                plt.grid()
                plt.xlabel('Time (s)')
                plt.ylabel('Vertical Speed (m/s)')

                plt.subplot(212)
                plt.plot(time_value,h_value,'b',linewidth=2.0)
                plt.grid()
                plt.xlabel('Time (s)')
                plt.ylabel('Altitude (km)')

                plt.show()
        # End Figure

    else:
        print('Invalid Flight Mode check %s'%args.flight_mode)
