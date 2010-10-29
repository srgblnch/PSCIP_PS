

at the moment to run the serial Tango DS:
- insert Rocket driver:
	$cd /etc/init.d/
	$sudo ./rocket	start
	$cd /dev
	$sudo chmod 777 ttyR*
	
- start pySerial DS
	$cd /home/mlipinski/Project/development/PSCIP/device_server/serial/pySerialDS/pySerialLine/src
	$python PySerial.py serpwr
	
- start PSI_PC DS
	$cd /home/mlipinski/Project/development/PSCIP/device_server/serial/PSI_PC2
	$./bin/PSI_PC2 maciej -v4
	
- run jive and hope everything works fine...