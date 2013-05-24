License: GPL3+

This source code is part of tango-ds:
https://tango-ds.svn.sourceforge.net/svnroot/tango-ds/Servers/PowerSupply/PSCIP_PS

Directories:
    1.0/
    1.1/ first trial
    1.2/ tuning events, messages, interlocks etc
    doc/ documentation used for programming. the authorative source is
         shared folder smb://storage00/AllDivisions/Magnets&PowerSupplies/POWER SUPPLIES/OCEM/


The versioning is a little bit messy, currently the good device server should 1.1/ds and the good kernel driver 0.9-IRQF*/driver.
Both have been compiled on ictlael01 (openSuse 11.1, 2.6.27.29-0.1-default, i686)

Older Notes
=====================================================================================
loging on ctalpha
$ssh -X mlipinski@ctalpha

see log messages on-the-fly
$sudo tail -f /var/log/messages

copy the right sources to the right direction
	- in my case I have the source of the driver and DS in /home/mlipinski/Project/development, so I copy it from there
	- i think I copy the source to another location to work with it for some reason I don't remember... however I don't apply the coping thing to DS, no need
$cd /
$sudo rm -r development/
$sudo cp -r /home/mlipinski/Project/development/ /



compile the driver
$cd /development/PSCIP/driver/
$sudo make


insert the driver  (logged in on mlipinski@ctalpha):
	- it's good to turn on "on-the-fly" log messages viewing before inserting the driver because the driver can tell (how tokative it is depends on the __DEBUG__ defined in pscip.c) quite a lot useful thing when inserting
	- the script is set to show few last lines of the log message file
$sudo ./pscip_load


deinsert the driver:
	- the script is set to show few last lines of the log message file
$sudo ./pscip_unload

run the simple terminal interface:
	- the first input argument determines the IP number
	- the second input argument determines the channel on the IP
$gcc test_prog.c
$./a.out 0 1

compile Tango Device Server
$cd /home/mlipinski/Project/development/PSCIP/device_server/PSI_PC2/
$make

run Tango device server
	- there are two DSes in the jive (tango database) to make my life easier
	- maciej is for link 0 of IP 0
	- zbigniew is for link 1 of IP 0
	- before turning on DS make sure that the Power Supply is turned on for fiber optic control!!!!! - is it a bug???
$./bin/PSI_PC maciej -v4	or	./bin/PSI_PC zbigniew -v4


