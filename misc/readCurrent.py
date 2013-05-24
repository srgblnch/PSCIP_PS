#!/usr/bin/env python

import PyTango
import time
from datetime import datetime

dev = PyTango.DeviceProxy("test/pc/psi0")
current = "cos"

while(True):
    now_1 = datetime.now()
    
    for i in range(1000):
        current = dev.read_attribute("CurrentSetPoint")
	
    now_2 = datetime.now()
    duration = (now_2 - now_1)/1000
    
    print "Read Current peformed,  value: ", current.value, ", time taken to perform: ", duration, " [usec]"
    time.sleep(0.1)

