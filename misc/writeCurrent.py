#!/usr/bin/env python

import PyTango
import time
from datetime import datetime

dev = PyTango.DeviceProxy("test/pc/psi0")
current = 0.1
change = 0.1


while(True):
    now_1 = datetime.now()
    dev.write_attribute("CurrentSetPoint", current)
    now_2 = datetime.now()
    duration = now_2.microsecond - now_1.microsecond
    print "Write CurrentSetPoint peformed, set value: %s, ime taken to perform: %d [usec]" %(current,duration)
    time.sleep(0.1)
    current = current + change
    if current > 2 :
        change = -change
    elif current < 0.1 :
	change = -change 
