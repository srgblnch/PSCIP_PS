//=============================================================================
//
// file :        PSI_PC.h
//
// description : Include for the PSI_PC class.
//
// project :	PSI power supplies
//
// $Author$
//
// $Revision$
//
// $Log:  $
// $URL$
// $Rev$
//
// copyleft :     ALBA Synchrotron Facility
//                Edifici Ciencies Nord. Modul C-3 central.
//              Campus Universitari de Bellaterra. Universitat Autonoma de Barcelona
//              08193 Bellaterra, Barcelona
//              Spain
//
//=============================================================================
//
//              This file is generated by POGO
//      (Program Obviously used to Generate tango Object)
//
//         (c) - Software Engineering Group - ESRF
//=============================================================================
#ifndef _PSI_PC_H
#define _PSI_PC_H

#define _SERIAL_

#include <tango.h>
#include <pscip.h>
#include "psc_codes.h"          //taken from Elettra - needs checking
#include "pscip_codes.h"

#include <vector>
#include <time.h>

#define MAX_NUM_OF_DETECTED_ERRMSGS 20
#define FiberConnection                         0
#define SerialConnection                        1

//using namespace Tango;

/**
 * @author      $Author$
 * @version     $Revision$
 */

 //     Add your own constants definitions here.
 //-----------------------------------------------
/*      PSC states.
        Source: "User documentation Power Supply DSP Firmware"
*/

#define OFFSET_VALUE            0.0
#define TIME_TO_NEXT_RECON      5.0 /* seconds */

#define PS_MONITOR            0 /* standby */
#define PS_DEVICE_OFF         1 /* off */
#define PS_DEVICE_ON          2 /* on */
#define PS_ADC_CAL            3 /* init */
#define PS_DEVICE_LOCKED      4
#define PS_TRANSIENT          5 /* init */
#define PS_DEVICE_OFF_LOCKED  6
#define PS_DOWNLOAD_DATA      8
#define PS_SAVE_DATA          9
#define PS_MODIFY_DATA       11

/* Psc status added flag */
#define PSC_PC_ON_STATE     16 /* on=true; off=false */
#define PSC_PC_LOCK_STATE   17 /* error=true; ok=false */
#define PSC_ALARM_SIGNAL    18 /* alarm signal from kempower */
#define PSC_FAULT_SIGNAL    19 /* alarm from PC controller */
#define PSC_CYCLERUN        20
#define PSC_CYCLEERR        21
#define PSC_STANDBYRUN      22
#define PSC_STANDBYERR      23
#define PSC_RUNWAVE         24 /* running waveform */
#define PSC_WAVEBREAK       25 /* waveform stopped on a breakpoint */
#define PSC_NOTINIT         26 /* power supply not initialized */
#define PSC_CURRENTRUN      27
#define PSC_CURRENTERR      28

namespace PSI_PC_ns
{

/**
 * Class Description:
 * <strong><i>Control of SR power supplies.. The Device Server is meant to provide control of  PSI Power Supply Controllers. The DS is basic/universal, it means that it can be used as a base for developing Device Servers for PSI PSC intended for specific usage which requires implementing additional functions (ex. upload/download waveform).</i></strong>
 *	<Br>
 *	<Br>
 *	The Device Server can use two types of hardware connection to communicate with PSI PSC: serial (RS232) or fiber. The connection type is determined by setting the property: ConnectionType (0 for fiber and 1 for serial).<Br>
 *	<Br>
 *	For simplicity, PSC DS connected by serial connection will be called serial DS, PSC DS connected by fiber connection will be called fiber DS.<Br>
 *	<Br>
 *	Serial connection is considered as local and 'service', it is not supposed to be used on normal basis, just in extraordinary cases. <Br>
 *	Fiber connection is considered a default connection meant to be used most of the time.<Br>
 *	<Br>
 *	Fiber connection uses PCI/cPCI Carrier Card. Each card carries two Industrial Pack (IP), each having two channels. Each channel of IP is connected to one PSC. One IP is represented in /dev/ folder as node called pscipX (X  is number).<Br>
 *	If fiber connection is to be used, the property 'FODevice' needs to be set (ex '/dev/pscip0') along with 'Channel' property (0 or 1).<Br>
 *	<Br>
 *	Serial connection uses PySerial Device Server. The property 'TangoDevice' needs to be set inicating the Py Serial name and localization (ex. 'ws/pc-test/ocem00-serial ').<Br>
 *	<Br>
 *	For the PySerial to work properly,  the following attributes need to be set:<Br>
 *	<Br>
 *	<Center>
 *	<table  border = 2 >
 *	<tr bgcolor=#cccccc><td> <strong>Attr name</strong> </td> <td> <strong>value</strong> </td></tr>
 *	<tr><td> Bautrate </td> <td> 115200 </td></tr>
 *	<tr><td> DataBits </td> <td> 8 </td></tr>
 *	<tr><td> Parity </td> <td> odd </td></tr>
 *	<tr><td> Port </td> <td> Correct port to which PSC is connected (ex. /dev/ttyR0) </td></tr>
 *	<tr><td> StopBits </td> <td> 1 </td></tr>
 *	</table>
 *	</Center>
 *	<Br>
 *	<Br>
 *	In theory, instead of PySerial DS, any DS can be used instead of PySerial without any changes to PSI SD as long as the following commands are implemented:
 *	<ul>
 *	<li>'ReadLine' for reading data,
 *	<li>'Write' for writing
 *	<li>'Open'
 *	<li>'Close'
 *	<li>'State'
 *	</ul>
 *	Both connections can be used simultaneously only for reading. When writing, a priority is given to one of the Device Servers  The priority can be only set Serial DS. It means that DS connected by fiber cannot regain writing priority , it has to be granted priority. For safety reasons the priority is granted to serial DS as long as it is connected to PSC. To be precise, the priority for serial DS is granted as soon as it attempts the first write. The priority is set to fiber DS only if serial DS is disconnected from PSC.<Br>
 *	<Br>
 *	<strong>CAUTION:</strong> <i>If the priority is set to serial DS and an attempt to write is made by fiber DS, the fiber DS gives no error message, it looks as if the operation was successful, however, the value is not written.</i> <Br>
 *	<Br>
 *	The following documentation is available, which can be useful  for future developers/users:<Br>
 *	1)      <a href="documentation/Communication_protocol.doc">Communication_protocol.doc</a> is adescription of communication protocol between DS and PSC<Br>
 *	2)      <a href="documentation/DSP_SW_R4_User_Documentation English.doc">DSP_SW_R4_User_Documentation English.doc</a>  is a description of PSI Power Supply Controller (all registers and their meaning, meaning of leds, etc)<Br>
 *	3)      <a href="documentation/Messages_Codes_en.xls">Messages_Codes_en.xls</a> explains meaning of error codes from PSC <Br>
 *	4)      <a href="documentation/pc-architecture.odt">DS Specification</a> Architecture and Design of the TANGO device server for the Power supplies at ALBA<Br>
 *	<p>Detailed documentation describing hardware used is provided
 *	with the PSCIP Linux Device Driver.</p>
 *	<p></p>
 *	<p></p>
 *	<p><b><span>Commands, attributes and
 *	properties detailed description.</b></p>
 *	<p></p>
 *	<p>The following symbols are used:</p>
 *	<p></p>
 *	<div align="center">
 *	<table class="MsoTableGrid" width="583" border="1" cellspacing="0" cellpadding="0">
 *	<tbody>
 *	<tr>
 *	<td valign="top" width="67">
 *	<p><b>Symbol</b></p>
 *	</td>
 *	<td valign="top" width="258">
 *	<p><b>meaning</b></p>
 *	</td>
 *	<td valign="top" width="258">
 *	<p><b>Attribute /property</b></p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="67">
 *	<p>I</p>
 *	</td>
 *	<td valign="top" width="258">
 *	<p>current</p>
 *	</td>
 *	<td valign="top" width="258">
 *	<p>Attribute</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="67">
 *	<p>I_ideal</p>
 *	</td>
 *	<td valign="top" width="258">
 *	<p>current setpoint</p>
 *	</td>
 *	<td valign="top" width="258">
 *	<p>Attribute</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="67">
 *	<p>I_ref</p>
 *	</td>
 *	<td valign="top" width="258">
 *	<p>current reference</p>
 *	</td>
 *	<td valign="top" width="258">
 *	<p>Attribute</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="67">
 *	<p>o</p>
 *	</td>
 *	<td valign="top" width="258">
 *	<p>offset</p>
 *	</td>
 *	<td valign="top" width="258">
 *	<p>Attribute</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="67">
 *	<p>f</p>
 *	</td>
 *	<td valign="top" width="258">
 *	<p>current factor</p>
 *	</td>
 *	<td valign="top" width="258">
 *	<p>Property</p>
 *	</td>
 *	</tr>
 *	</tbody>
 *	</table>
 *	</div>
 *	<p></p>
 *	<p>In most of the cases the default values of 'offset' and 'current factor' should be set:</p>
 *	<p>o = 0</p>
 *	<p>f = 1</p>
 *	<p>Change of the default setting should be done only in the
 *	case User knows what he/she is doing !!!!</p>
 *	<p></p>
 *	<p><b><u><span>Attributes: </span></u></b></p>
 *	<table class="MsoTableGrid" border="1" cellspacing="0" cellpadding="0" align="center">
 *	<tbody>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>Name</b></p>
 *	</td>
 *	<td valign="top" width="396">
 *	<p><b>Description </b></p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>Current</b></p>
 *	</td>
 *	<td valign="top" width="396">
 *	<p>current calculated out of measured ( by PSC ) current
 *	according to the following formula:</p>
 *	<p align="center" style="text-align: center;">I = I_ideal + (
 *	I_ref &#45; I_measured ) / f</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>CurrentSetpoint</b></p>
 *	</td>
 *	<td valign="top" width="396">
 *	<p>the ideal current which is intended to be acquired, it is
 *	not the current set to the hardware, the current set in the hardware is
 *	calculated according to the following formula:</p>
 *	( I_ideal + o )</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>CurrentOffset</b></p>
 *	</td>
 *	<td valign="top" width="396">
 *	<p>a value added to the current written to the 'current
 *	setpoint' attribute</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>CurrentReference</b></p>
 *	</td>
 *	<td valign="top" width="396">
 *	<p>the current value written to the hardware</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>CurrentMeasured</b></p>
 *	</td>
 *	<td valign="top" width="396">
 *	<p>current measurement read from the hardware</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>Errors</b></p>
 *	</td>
 *	<td valign="top" width="396">
 *	<p>returns readable information about all the errors which
 *	have occurred. If error occurs more then one time, it is not duplicated in
 *	the return table</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>ErrorCodes</b></p>
 *	</td>
 *	<td valign="top" width="396">
 *	<p>returns codes of the latest errors detected according to
 *	categories stored in <b>ErrorLabel</b>
 *	attribute</p>
 *	<p></p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>ErrorLabel</b></p>
 *	</td>
 *	<td valign="top" width="396">
 *	<p> returns the categories of the error codes</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>RemoteMode</b></p>
 *	</td>
 *	<td valign="top" width="396">
 *	<p>is glued to 1 (means remote mode) because PSC can only
 *	be operated remotely</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>State</b></p>
 *	</td>
 *	<td valign="top" width="396">
 *	<p>returns device state (short message)</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>Status</b></p>
 *	</td>
 *	<td valign="top" width="396">
 *	<p>returns human readable, textual description</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>Voltage</b></p>
 *	</td>
 *	<td valign="top" width="396">
 *	<p>returns voltage measured on the load</p>
 *	</td>
 *	</tr>
 *	</tbody>
 *	</table>
 *	<div align="center"></div>
 *	<div align="center"></div>
 *	<div align="center"></div>
 *	<div align="center"></div>
 *	<p align="center"><b><u><span><span> </span></span></u></b></p>
 *	<p><b><u><span>Properties:</span></u></b></p>
 *	<table class="MsoTableGrid" border="1" cellspacing="0" cellpadding="0" align="center">
 *	<tbody>
 *	<tr>
 *	<td valign="top" width="189">
 *	<p><b>Name</b></p>
 *	</td>
 *	<td valign="top" width="407">
 *	<p><b>Description</b></p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="189">
 *	<p><b>Channel</b><b><u><span></span></u></b></p>
 *	</td>
 *	<td valign="top" width="407">
 *	<p>(0 or 1) defines IP channel used to connect PSC by fiber</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="189">
 *	<p><b>ConnectionType</b><b><u><span></span></u></b></p>
 *	</td>
 *	<td valign="top" width="407">
 *	<p>(0 &#45; fiber; 1 &#45; serial) determines which hardware medium
 *	is used to connect industrial PC with PSC</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="189">
 *	<p><b>FODevice</b><b><u><span></span></u></b></p>
 *	</td>
 *	<td valign="top" width="407">
 *	<p>defines device used for fiber optic connection (ex.
 *	/dev/pscip0)</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="189">
 *	<p><b>TangoDevice</b><b><u><span></span></u></b></p>
 *	</td>
 *	<td valign="top" width="407">
 *	<p>defines name and location of PySerial DS used for serial
 *	communication</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="189">
 *	<p><b>Current factor</b><b><u><span></span></u></b></p>
 *	</td>
 *	<td valign="top" width="407">
 *	<p>multiplier</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="189">
 *	<p><b>InterlockFlowBit
 *	InterlockTemperatureBit</b><b><u><span></span></u></b></p>
 *	</td>
 *	<td valign="top" width="407">
 *	<p>'flow' interlock and 'temperature'  interlock signals are
 *	represented in 'Digital_Input_Signals' register of PSC as bits. The
 *	properties allow user to determine which bits of the PSC register are
 *	associated with this interlocks. If correctly defined, these properties
 *	enable DS to recognize Flow and Temperature interlocks and provide user with
 *	appropriate error message</p>
 *	<p><b><u><span><span> </span></span></u></b></p>
 *	</td>
 *	</tr>
 *	</tbody>
 *	</table>
 *	<div align="center"></div>
 *	<p align="center"><br /><b><u><span><span></span></span></u></b></p>
 *	<p align="left"><b><u><span><span>Commands:<br /></span></span></u></b></p>
 *	<table class="MsoTableGrid" border="1" cellspacing="0" cellpadding="0" align="center">
 *	<tbody>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>Name</b></p>
 *	</td>
 *	<td valign="top" width="403">
 *	<p>Description</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>ClearError</b></p>
 *	</td>
 *	<td valign="top" width="403">
 *	<p>clear Disorder register in PSC, Error and ErrorCodes
 *	attributes in DS</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>Disable_interlocks</b><b><span></span></b></p>
 *	</td>
 *	<td valign="top" width="403">
 *	<p>write Digital_Input_Signals_Mask with 0xE070 value. I
 *	causes all the interlocks, except one, to be disabled. The not disabled
 *	interlock has input at the rear of PSC and can be disabled hardware-wise. The
 *	change of Mask register value is temporary.</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>Enable_interlocks</b><b><span></span></b></p>
 *	</td>
 *	<td valign="top" width="403">
 *	<p>write Digital_Input_Signals_Mask with 0xF877 value.
 *	Interlocks are enabled<b><span></span></b></p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>Interlocks_state</b><b><span></span></b></p>
 *	</td>
 *	<td valign="top" width="403">
 *	<p>shows the current value of<span>
 *	</span>Digital_Input_Signals_Mask, Three states: 'Interlocks enabled',
 *	'Interlocks disabled' or<span>  </span>'Interlocks
 *	customized'</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>Init</b><b><span></span></b></p>
 *	</td>
 *	<td valign="top" width="403">
 *	<p></p>
 *	<p>default function which re-initialize Device Server not
 *	affecting the state of the hardware</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>Off</b><b><span></span></b></p>
 *	</td>
 *	<td valign="top" width="403">
 *	<p>turns off the device<b><span></span></b></p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>On</b><b><span></span></b></p>
 *	</td>
 *	<td valign="top" width="403">
 *	<p>turns on the device</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>Read_PSC_register</b><b><span></span></b></p>
 *	</td>
 *	<td valign="top" width="403">
 *	<p>enable user to read any PSC register which is
 *	read-enabled. The address and (optionally) format of output data need to be
 *	specified. The default format is hexadecimal. Example input data '0x00' will
 *	read register 0x00 and output it data as hexadecimal, '0x00 f' will read
 *	register 0x00 and output it as flaot</p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>Reset</b><b><span></span></b></p>
 *	</td>
 *	<td valign="top" width="403">
 *	<p>resets PSC by turning it off, an attempt to reestablish
 *	connection with hardware is taken if it has been lost. It also calls
 *	ClearErrors command<b><span></span></b></p>
 *	</td>
 *	</tr>
 *	<tr>
 *	<td valign="top" width="187">
 *	<p><b>State </b></p>
 *	<p><b>Status</b><b><span></span></b></p>
 *	</td>
 *	<td valign="top" width="403">
 *	<p>the same as attributes</p>
 *	</td>
 *	</tr>
 *	</tbody>
 *	</table>
 *	<div align="center"></div>
 *	<p align="center"></p>
 *	<p></p>
 *	<p></p>
 *	<p></p>
 */

/*
 *	Device States Description:
*  Tango::UNKNOWN :  No connection to the PC established yet.
 *                    Note that this applies only when the PC is first initalized,
 *                    if the connection is lost at a later point, a fault is generated.
 *                    Also if it could not be determined whether power is on or off.
*  Tango::INIT :     some form of initialization is done
*  Tango::ON :       Power active
*  Tango::OFF :      PC does not supply power
*  Tango::ALARM :    Voltage or Current supplied significantly deviates from set point
 *                    Including supplying power when it shoul be off and not supplying power
 *                    when in on state.
 *                    Interlock triggered.
*  Tango::FAULT :    PC indicates a broken hardware, programming error detected or connection lost.
*  Tango::RUNNING :  Ramping Mode
*  Tango::MOVING :   Software waveform mode
 */


class PSI_PC: public Tango::Device_4Impl
{
public :
        //      Add your own data members here
        //-----------------------------------------


        //      Here is the Start of the automatic code generation part
        //-------------------------------------------------------------
/**
 *      @name attributes
 *      Attributs member data.
 */
//@{
		Tango::DevDouble	*attr_CurrentSetpoint_read;
		Tango::DevDouble	attr_CurrentSetpoint_write;
		Tango::DevDouble	*attr_Voltage_read;
		Tango::DevBoolean	*attr_RemoteMode_read;
		Tango::DevDouble	*attr_Current_read;
		Tango::DevDouble	*attr_CurrentOffset_read;
		Tango::DevDouble	attr_CurrentOffset_write;
		Tango::DevDouble	*attr_I_read;
		Tango::DevDouble	attr_I_write;
		Tango::DevFloat	*attr_V_read;
		Tango::DevLong	*attr_ErrorCode_read;
		Tango::DevDouble	*attr_SoftwareWaveform_read;
		Tango::DevDouble	attr_SoftwareWaveform_write;
		Tango::DevShort	*attr_ErrorCodes_read;
		Tango::DevString	*attr_ErrorLabel_read;
		Tango::DevString	*attr_Errors_read;
//@}

/**
 *      @name Device properties
 *      Device properties member data.
 */
//@{
/**
 *	Channel used of PC
 */
	Tango::DevShort	channel;
/**
 *	BName of the Linux character / block device.
 */
	string	fODevice;
/**
 *	Defines which bit of digital input register indicates temperature interlock
 */
	string	interlock1;
/**
 *	message to be shown for external interlock 2
 */
	string	interlock2;
/**
 *	name of the python serial device server responisble for serial communication
 */
	string	tangoDevice;
/**
 *	You can choose the kind of connection the DS "talks" with PSC:
 *	0 -> fiber
 *	1 -> serial (RS232)
 *	
 *	To run RS232 you need to create pySerial DS
 */
	Tango::DevShort	connectionType;
/**
 *	scaling factor ( f ):
 *	Iref = f * ( I_idal + o )
 *	
 */
	Tango::DevFloat	currentFactor;
/**
 *	message to be shown for External Interlock 3
 */
	string	interlock3;
/**
 *	message to be shown for external interlock 4
 */
	string	interlock4;
//@}

/**@name Constructors
 * Miscellaneous constructors */
//@{
/**
 * Constructs a newly allocated Command object.
 *
 *      @param cl       Class.
 *      @param s        Device Name
 */
        PSI_PC(Tango::DeviceClass *cl,string &s);
/**
 * Constructs a newly allocated Command object.
 *
 *      @param cl       Class.
 *      @param s        Device Name
 */
        PSI_PC(Tango::DeviceClass *cl,const char *s);
/**
 * Constructs a newly allocated Command object.
 *
 *      @param cl       Class.
 *      @param s        Device name
 *      @param d        Device description.
 */
        PSI_PC(Tango::DeviceClass *cl,const char *s,const char *d);
//@}

/**@name Destructor
 * Only one desctructor is defined for this class */
//@{
/**
 * The object desctructor.
 */
        ~PSI_PC() {delete_device();};
/**
 *      will be called at device destruction or at init command.
 */
        void delete_device();
//@}


/**@name Miscellaneous methods */
//@{
/**
 *      Initialize the device
 *                      - initializes the variables
 *                      - initializes error codes table
 *                      - calls funcion which establishes connectio (fiber or serial) with PSC
 *                      - calls functin which checks state of the device
 *                      - calls function to get properties from database
 *                      - calls funciton which creates error message table
 */
        virtual void init_device();
/**
 *      Always executed method befor execution command method.
 */
        virtual void always_executed_hook();

//@}

/**
 * @name PSI_PC methods prototypes
 */

//@{
/**
 *	Hardware acquisition for attributes.
 */
	virtual void read_attr_hardware(vector<long> &attr_list);
/**
 *	Extract real attribute values for CurrentSetpoint acquisition result.
 */
	virtual void read_CurrentSetpoint(Tango::Attribute &attr);
/**
 *	Write CurrentSetpoint attribute values to hardware.
 */
	virtual void write_CurrentSetpoint(Tango::WAttribute &attr);
/**
 *	Extract real attribute values for Voltage acquisition result.
 */
	virtual void read_Voltage(Tango::Attribute &attr);
/**
 *	Extract real attribute values for RemoteMode acquisition result.
 */
	virtual void read_RemoteMode(Tango::Attribute &attr);
/**
 *	Extract real attribute values for Current acquisition result.
 */
	virtual void read_Current(Tango::Attribute &attr);
/**
 *	Extract real attribute values for CurrentOffset acquisition result.
 */
	virtual void read_CurrentOffset(Tango::Attribute &attr);
/**
 *	Write CurrentOffset attribute values to hardware.
 */
	virtual void write_CurrentOffset(Tango::WAttribute &attr);
/**
 *	Extract real attribute values for I acquisition result.
 */
	virtual void read_I(Tango::Attribute &attr);
/**
 *	Write I attribute values to hardware.
 */
	virtual void write_I(Tango::WAttribute &attr);
/**
 *	Extract real attribute values for V acquisition result.
 */
	virtual void read_V(Tango::Attribute &attr);
/**
 *	Extract real attribute values for ErrorCode acquisition result.
 */
	virtual void read_ErrorCode(Tango::Attribute &attr);
/**
 *	Extract real attribute values for SoftwareWaveform acquisition result.
 */
	virtual void read_SoftwareWaveform(Tango::Attribute &attr);
/**
 *	Write SoftwareWaveform attribute values to hardware.
 */
	virtual void write_SoftwareWaveform(Tango::WAttribute &attr);
/**
 *	Extract real attribute values for ErrorCodes acquisition result.
 */
	virtual void read_ErrorCodes(Tango::Attribute &attr);
/**
 *	Extract real attribute values for ErrorLabel acquisition result.
 */
	virtual void read_ErrorLabel(Tango::Attribute &attr);
/**
 *	Extract real attribute values for Errors acquisition result.
 */
	virtual void read_Errors(Tango::Attribute &attr);
/**
 *	Read/Write allowed for CurrentSetpoint attribute.
 */
	virtual bool is_CurrentSetpoint_allowed(Tango::AttReqType type);
/**
 *	Read/Write allowed for Voltage attribute.
 */
	virtual bool is_Voltage_allowed(Tango::AttReqType type);
/**
 *	Read/Write allowed for RemoteMode attribute.
 */
	virtual bool is_RemoteMode_allowed(Tango::AttReqType type);
/**
 *	Read/Write allowed for Current attribute.
 */
	virtual bool is_Current_allowed(Tango::AttReqType type);
/**
 *	Read/Write allowed for CurrentOffset attribute.
 */
	virtual bool is_CurrentOffset_allowed(Tango::AttReqType type);
/**
 *	Read/Write allowed for I attribute.
 */
	virtual bool is_I_allowed(Tango::AttReqType type);
/**
 *	Read/Write allowed for V attribute.
 */
	virtual bool is_V_allowed(Tango::AttReqType type);
/**
 *	Read/Write allowed for ErrorCode attribute.
 */
	virtual bool is_ErrorCode_allowed(Tango::AttReqType type);
/**
 *	Read/Write allowed for SoftwareWaveform attribute.
 */
	virtual bool is_SoftwareWaveform_allowed(Tango::AttReqType type);
/**
 *	Read/Write allowed for ErrorCodes attribute.
 */
	virtual bool is_ErrorCodes_allowed(Tango::AttReqType type);
/**
 *	Read/Write allowed for ErrorLabel attribute.
 */
	virtual bool is_ErrorLabel_allowed(Tango::AttReqType type);
/**
 *	Read/Write allowed for Errors attribute.
 */
	virtual bool is_Errors_allowed(Tango::AttReqType type);
/**
 *	Execution allowed for On command.
 */
	virtual bool is_On_allowed(const CORBA::Any &any);
/**
 *	Execution allowed for Off command.
 */
	virtual bool is_Off_allowed(const CORBA::Any &any);
/**
 *	Execution allowed for Reset command.
 */
	virtual bool is_Reset_allowed(const CORBA::Any &any);
/**
 *	Execution allowed for ResetInterlocks command.
 */
	virtual bool is_ResetInterlocks_allowed(const CORBA::Any &any);
/**
 *	Execution allowed for DisableInterlocks command.
 */
	virtual bool is_DisableInterlocks_allowed(const CORBA::Any &any);
/**
 *	Execution allowed for EnableInterlocks command.
 */
	virtual bool is_EnableInterlocks_allowed(const CORBA::Any &any);
/**
 *	Execution allowed for InterlockStatus command.
 */
	virtual bool is_InterlockStatus_allowed(const CORBA::Any &any);
/**
 *	Execution allowed for Read_PSC_register command.
 */
	virtual bool is_Read_PSC_register_allowed(const CORBA::Any &any);
/**
 *	Execution allowed for StopSoftwareWaveform command.
 */
	virtual bool is_StopSoftwareWaveform_allowed(const CORBA::Any &any);
/**
 *	Execution allowed for StartSoftwareWaveform command.
 */
	virtual bool is_StartSoftwareWaveform_allowed(const CORBA::Any &any);
/**
 *	Execution allowed for Connect command.
 */
	virtual bool is_Connect_allowed(const CORBA::Any &any);
/**
 *	Execution allowed for Update command.
 */
	virtual bool is_Update_allowed(const CORBA::Any &any);
/**
 * This command gets the device state (stored in its <i>device_state</i> data member) and returns it to the caller.
 *	@return	State Code
 *	@exception DevFailed
 */
	virtual Tango::DevState	dev_state();
/**
 * This command gets the device status (stored in its <i>device_status</i> data member) and returns it to the caller.
 *	@return	Status description
 *	@exception DevFailed
 */
	virtual Tango::ConstDevString	dev_status();
/**
 * 
 *	@exception DevFailed
 */
	void	on();
/**
 * 
 *	@exception DevFailed
 */
	void	off();
/**
 * 
 *	@exception DevFailed
 */
	void	reset();
/**
 * 
 *	@exception DevFailed
 */
	void	reset_interlocks();
/**
 * 
 *	@exception DevFailed
 */
	void	disable_interlocks();
/**
 * 
 *	@exception DevFailed
 */
	void	enable_interlocks();
/**
 * 
 *	@return	Interlocks Status
 *	@exception DevFailed
 */
	Tango::DevString	interlock_status();
/**
 * 
 *	@param	argin	Register address and return format (f-float,i-integer,x-hex, by default x), ex input: 0x00 x - address 0, return value hex
 *	@return	Data, hexadecimal
 *	@exception DevFailed
 */
	Tango::DevString	read__psc_register(Tango::DevString);
/**
 * 
 *	@exception DevFailed
 */
	void	stop_software_waveform();
/**
 * 
 *	@exception DevFailed
 */
	void	start_software_waveform();
/**
 * 
 *	@exception DevFailed
 */
	void	connect();
/**
 * 
 *	@exception DevFailed
 */
	void	update();

/**
 *	Read the device properties from database
 */
	 void get_device_property();
//@}

        //      Here is the end of the automatic code generation part
        //-------------------------------------------------------------


protected :
        //      Add your own data members here
        //-----------------------------------------

        /* Variables*/
        char psc_err_msg_buf[256];
        std::string status_string;      // message returned when status is called
        int fd;                                                                                                                         /**< File descriptor. */
        int numo_ErrorMsgs;                                                                              /** Number of error message in attr_ErrorMsgs_read */
        short ErrorsValue[PSC_DS_ALBA_ERRMSG];                                                          /**< Array holding values of errors. By default 0 means OK*/
        char psc_errmsg[256][PSCIP_SIZE_MSG];                                                           /**< PSCIP_SIZE_MSG defined in pscip.h */
        time_t last_comm_error;
        Tango::DeviceProxy *device;                                                                                     /**<  Proxy Used for serial communication  */

        std::vector<Tango::DevDouble> m_softwareWaveform;
        omni_thread* m_softwareWaveformThread;
        bool m_softwareWaveformThreadRunning;

        static void* _software_waveform_thread(void* ptr);
        void do_software_waveform();

        /* Functions */
        /**  Reads value of the current which should be set by the device  */
        double readCurrentSetpointFromDevice();
        /** reads information from the hardware in order to update State and
          * Status */
        void update_state(void);
        bool psc_write(int channel, char status, char address, int data);       /**< General function for writing to PSC (regardless of the communicatino medium)   */
        bool psc_read(int channel, char status, char address, int *data);       /**<  General funciton for reading from PSC (regardless of the communicatio medium)  */
        void psc_read_serial(pscip_t *pval);                                                                    /**<  Reading using RS232  */
        void psc_write_serial(pscip_t *pval);                                                                   /**<  Writing using RS232  */
        void psc_read_fiber(pscip_t *pval);                                                                     /**<  Reading using fiber optic  */
        void psc_write_fiber(pscip_t *pval);                                                                    /**<  Writing using fiber optic  */
        void close_serial(void);                                                                                        /**<   Closing connectin to PySerial DS */
        void init_serial(void);                                                                                         /**<   Openning/initializing connection to PySerial DS */
        void init_fiber(void);                                                                                          /**<   Openning fiber device (driver) */
        void clear_errors_in_IP();                                                                                      /**<   Clearing communication errors in IP */
        void wrCurrent(Tango::WAttribute &attr, const char *);                                  /**<   Writing desired value of the current to PSC (sets currentWrite and currentSetpoint attributes */
        const char *psc_get_communication_alba_errmsg(int err);                                 /**<   returns message representing error code (from fiber driver and communication using RS232 */
        double readCurrent();
        void dev_init();                                                                                                        /**<   Initialization of the table with PSC errors (coming from power supply controller not communicatioon */
        const char * add_errmsg(const char *msg_to_add);                                                                /**<   Adding error message (regardless the  source) to the error table which is shown to the user */
        Tango::DevState  read_PySerial_state();                                                         /** < reads state of PySerial DS */
        void check_connection_to_PySerial();                                                            /** < checks the connection with PySerial, to be more precise, checks the state of PySerial and tries to reopen it if it is in OFF state, if there is an error while reading state of PySerial, it also tries reopenning PySerial, if reopening fails an exception is thrown */
        /** rethrows exception for communication failure/error changing text
            in the description of caught exception, it is used to notify the
            user that the exception comes from another device server (PySerial)
        */
        void psc_modify_and_re_throw_exception(
            Tango::DevFailed &e,            //exception to be modify
            string modify_s,                // string modifying description message
            string info_s,                  // info string
            string cmd_s,                   // cmd string
            string function_name,           // name of the function in which exception occured
            int err_code
        );

         /** rethrows exception for communication failure/error */
        void psc_re_throw_exception(
            Tango::DevFailed &e,            //exception to be modify
            string info_s,                          //info string
            string cmd_s,                           //describtion string
            string function_name,           // name of the function in which exception occured
            int err_code
        );
        void psc_throw_exception(                                                                                       /** < creates and throws exception for communication failure/error */
            string info_s,                          //info string
            string cmd_s,                           //describtion string
            string function_name,           // name of the function in which exception occured
            int err_code,
            int err_type = ERR_PSI_IP
        );
        /** checks error code of ioctls for F/O communication */
        void handle_comm_error_fiber(
            int err,
            pscip_t *val,
            const char * origin
        );

};

}       // namespace_ns

#endif  // _PSI_PC_H
