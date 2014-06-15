# RRD file creation
#
# A COUNTER value is stored as a derivative with physical dimension "per second"
# according to RRDtool documentation
#
# The step size is dt = 60s. (This is the wanted resolution of the measurement.)
#
# A power distribution box can have a maximum power throughput of
#   Pmax = 3 x Imax x 400V
#
# We have Imax = 63A for normal distribution boxes and Imax = 250A for the main
# distribution
#
# The impulse constant is imp = 1000/kWh for most distribution boxes,
# imp = 800/kWh for some old ones and imp = 10000 for the main distribution
# board.
#
# The maximum number of impulses between two measurements is
#   Nmax = imp * Pmax * dt
#
# The maximum derivate is
#   dNmax = Nmax/dt = imp * Pmax
#
# This results into
#
# (a) for most distribution boxes
#     dNmax = 1000/kWh * 75600W = 756/36 1/s = 21/s
#
# (b) for some old distribution boxes
#     dNmax = 800/kWh * 75600W = 84/5 1/s = 16.8/s
#
# (c) for the main distribution box
#     dNmax = 10000/kWh * 300kW = 10000/12 1/s = 833.3/s
#
#
# The primary step size is 1min. We keep 525600 records. This is 1 year.
# The next step size is 15min. We keep 70080 records. This are 2 years.
# The coarsest step size is 60min. We keep 43800 records. This are 5 years.
#
# The command to create the database is: 

/usr/bin/rrdtool create \
/srv/cacti/rra/power.rrd \
--step 60  \
DS:01:COUNTER:120:0:18 \
DS:02:COUNTER:120:0:18 \
DS:06:COUNTER:120:0:18 \
DS:07:COUNTER:120:0:22 \
DS:10:COUNTER:120:0:850 \
DS:11:COUNTER:120:0:18 \
DS:12:COUNTER:120:0:22 \
DS:16:COUNTER:120:0:22 \
DS:17:COUNTER:120:0:22 \
DS:21:COUNTER:120:0:22 \
DS:22:COUNTER:120:0:22 \
DS:26:COUNTER:120:0:22 \
DS:27:COUNTER:120:0:22 \
DS:31:COUNTER:120:0:22 \
DS:32:COUNTER:120:0:22 \
DS:36:COUNTER:120:0:22 \
DS:37:COUNTER:120:0:22 \
DS:41:COUNTER:120:0:22 \
DS:46:COUNTER:120:0:22 \
DS:47:COUNTER:120:0:22 \
RRA:AVERAGE:0.5:1:525600 \
RRA:AVERAGE:0.5:15:70080 \
RRA:AVERAGE:0.5:60:43800 \
RRA:MIN:0.5:1:525600 \
RRA:MIN:0.5:15:70080 \
RRA:MIN:0.5:60:43800 \
RRA:MAX:0.5:1:525600 \
RRA:MAX:0.5:15:70080 \
RRA:MAX:0.5:60:43800 \
RRA:LAST:0.5:1:525600 \
RRA:LAST:0.5:15:70080 \
RRA:LAST:0.5:60:43800 \
