static const char *RcsId = "$Header:  $";
//+=============================================================================
//
// file :         PSI_PC.cpp
//
// description :  C++ source for the PSI_PC and its commands.
//                The class is derived from Device. It represents the
//                CORBA servant object which will be accessed from the
//                network. All commands which can be executed on the
//                PSI_PC are implemented in this file.
//
// project :      TANGO Device Server
//
// $Author:  $ Maciej Lipinski
//
// $Revision: 1.0 $
//
// $Log:  $
//
// copyleft :     ALBA Synchrotron Facility
//                Edifici Ciï¿½ncies Nord. Mï¿½dul C-3 central.
//              Campus Universitari de Bellaterra. Universitat Autï¿½noma de Barcelona
//              08193 Bellaterra, Barcelona
//              Spain




//===================================================================
//
//	The following table gives the correspondence
//	between commands and method name.
//
//  Command name           |  Method name
//	----------------------------------------
//  State                  |  dev_state()
//  Status                 |  dev_status()
//  On                     |  on()
//  Off                    |  off()
//  Reset                  |  reset()
//  ResetInterlocks        |  reset_interlocks()
//  DisableInterlocks      |  disable_interlocks()
//  EnableInterlocks       |  enable_interlocks()
//  InterlockStatus        |  interlock_status()
//  Read_PSC_register      |  read__psc_register()
//  StopSoftwareWaveform   |  stop_software_waveform()
//  StartSoftwareWaveform  |  start_software_waveform()
//  Connect                |  connect()
//  Update                 |  update()
//
//===================================================================


#include <tango.h>
#include <PSI_PC.h>
#include <PSI_PCClass.h>
#include <sys/time.h>

#include <iomanip>

#include <iostream>


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>

#include "NanoSeconds.h"
#import <cmath>
#include <sstream>

namespace PSI_PC_ns
{

static const char * psc_ds_alba_errmsg[] = {
        "psi: communication error (IP/RS)",
        "psi: psc error",
        "temperature interlock",
        "water flow interlock",
        "interlock"
};

//+----------------------------------------------------------------------------
//
// method :             PSI_PC::PSI_PC(string &s)
//
// description :        constructor for simulated PSI_PC
//
// in : - cl : Pointer to the DeviceClass object
//      - s : Device name
//
//-----------------------------------------------------------------------------
PSI_PC::PSI_PC(Tango::DeviceClass *cl,string &s)
:Tango::Device_4Impl(cl,s.c_str())
{
        init_device();
}

PSI_PC::PSI_PC(Tango::DeviceClass *cl,const char *s)
:Tango::Device_4Impl(cl,s)
{
        init_device();
}

PSI_PC::PSI_PC(Tango::DeviceClass *cl,const char *s,const char *d)
:Tango::Device_4Impl(cl,s,d)
{
        init_device();
}
//+----------------------------------------------------------------------------
//
// method :             PSI_PC::delete_device()
//
// description :        will be called at device destruction or at init command.
//
//-----------------------------------------------------------------------------
void PSI_PC::delete_device()
{
        DEBUG_STREAM << "deleting device..." << device <<endl;
        this->stop_software_waveform();

        //      Deallocate resources used
        delete attr_I_read;
        attr_I_read = 0;

        delete attr_CurrentSetpoint_read;
        attr_CurrentSetpoint_read = 0;

        delete attr_Current_read;
        attr_Current_read = 0;

        delete attr_Voltage_read;
        attr_Voltage_read = 0;

        delete attr_RemoteMode_read;
        attr_RemoteMode_read = 0;

        attr_ErrorCodes_read = 0;

        if (attr_Errors_read) {
                for(int i = 0; i < MAX_NUM_OF_DETECTED_ERRMSGS; i++)
                        CORBA::string_free(attr_Errors_read[i]);
                delete[] attr_Errors_read;
        }
        attr_Errors_read = 0;
        if (fd > 0)
        {
                DEBUG_STREAM << "closing file descriptor " << fd << endl;
                close(fd);
                fd = 0;
        }
        if (device)
        {
            DEBUG_STREAM << "closing device " << device << endl;
            close_serial();
            device = 0;
        }
        DEBUG_STREAM << "delete device"<<endl;
}

//+----------------------------------------------------------------------------
//
// method :             PSI_PC::init_device()
//
// description :        Is called at device server initialization, it:
//                      - initializes the variables
//                      - initializes error codes table
//                      - calls funcion which establishes connectio (fiber or serial) with PSC
//                      - calls functin which checks state of the device
//                      - calls function to get properties from database
//                      - calls funciton which creates error message table
//
//-----------------------------------------------------------------------------
void PSI_PC::init_device()
{
        DEBUG_STREAM << "PSI_PC::PSI_PC() init_device " << endl;

        // Empty initialize most things...

        DEBUG_STREAM << "PSI_PC::PSI_PC() preparing soft waveform... " << endl;

#if 1
        m_softwareWaveformThread = 0;
        m_softwareWaveformThreadRunning = false;
        m_softwareWaveform.resize(10000);
        // default SF waveform I put a sin between 0.0-1.0 A
        static const double pi = 3.1415926535897931;
        for (size_t n=0; n<m_softwareWaveform.size(); ++n)
                m_softwareWaveform[n] = 0.5+0.5*sin(2.0*pi*n/m_softwareWaveform.size());
#endif
        connectionType = FiberConnection;
        device = 0;
        fd = 0;
        numo_ErrorMsgs = 0;

        attr_Errors_read = 0;
        attr_I_read = 0;
        attr_CurrentSetpoint_read = 0;
        attr_Current_read = 0;
        attr_Voltage_read = 0;
        attr_RemoteMode_read = 0;

        attr_CurrentOffset_read = new Tango::DevDouble;
        *attr_CurrentOffset_read = OFFSET_VALUE;

        set_change_event("State", true);
        set_change_event("Status", true);
        set_state(Tango::UNKNOWN);
        push_change_event("State");
        set_status("initializing device server");
        push_change_event("Status");
        DEBUG_STREAM << "PSI_PC::PSI_PC() initializing variables " << endl;
        // Initialise variables to default values
        //--------------------------------------------

        try {
                attr_I_read = new Tango::DevDouble;
                *attr_I_read = 0.0;

                attr_CurrentSetpoint_read = new Tango::DevDouble;
                *attr_CurrentSetpoint_read = 0;

                attr_Current_read = new Tango::DevDouble;
                *attr_Current_read = 0;

                attr_Voltage_read = new Tango::DevDouble;
                *attr_Voltage_read = 0;

                attr_RemoteMode_read = new Tango::DevBoolean;
                *attr_RemoteMode_read = true;

                attr_Errors_read = new Tango::DevString[MAX_NUM_OF_DETECTED_ERRMSGS];
                numo_ErrorMsgs = 0;

                for(int i = 0; i < MAX_NUM_OF_DETECTED_ERRMSGS; i++) {
                        attr_Errors_read[i] = CORBA::string_dup("");
                        if (attr_Errors_read[i] == 0)
                                throw std::bad_alloc();
                }
        } catch (std::bad_alloc &ba) {
                Tango::Except::throw_exception (
                                        "OUT OF MEMORY",
                                        "out of memory error",
                                        "PSI_PC::init_device()");
        }


        for(int i = 0; i < PSC_DS_ALBA_ERRMSG; i++)
                ErrorsValue[i] = 0;

        attr_ErrorCodes_read = ErrorsValue;

        attr_ErrorLabel_read = const_cast<char**>(psc_ds_alba_errmsg);

        get_device_property();  //gets properties form databse
        DEBUG_STREAM << "init device device before connect is  "<< device << endl;
        dev_init();                             // creates table with error messages


        /***    fiber /serial initialization    ***/
        if( connectionType == FiberConnection)
        {
                init_fiber();
        }
        else if(connectionType == SerialConnection)
        {
                init_serial();
        }
        else
        {
            int err = CONNECTIONTYPE_PROPERTY_ERROR;        //for fiber connection, communication error values come from the driver, for serial connection we need to provide a code here
            INFO_STREAM<< "Unknown value of property ConnectionType, choose \n0 for fiber \n1 for serial connection "<< endl;
            TangoSys_MemStream out_stream, cmd_stream;
            ErrorsValue[ERR_PSI_IP] =  err;
            out_stream << psc_get_communication_alba_errmsg(err ) <<endl;
            out_stream <<  "choose \n0 for fiber \n1 for serial connection \n, inputed parameter value: "<< connectionType << ends;
            ERROR_STREAM << cmd_stream.str() << endl;

            add_errmsg("Wrong ConnectionType parameter");

            Tango::Except::throw_exception(
                    cmd_stream.str(),
                    out_stream.str(),
                    (const char *) "Psc::psc_init", Tango::ERR);
        }
        // get ModelID
/************** to be added latter
        int model_id;

        int data;
        try
        {
                psc_read(channel,0x0, PSC_MODEL_ID, &data);
        }
        catch(Tango::DevFailed &e)
        {
                Tango::Except::re_throw_exception(e,
                        (const char *)"Command failed, error reading error",
                        (const char *)"Error from psc",
                        (const char *) "PSI_PC::init_fiber()", Tango::ERR);
                        exit(1);
        }
        model_id = data;
*/
        DEBUG_STREAM << "PSI_PC::PSI_PC() init_device connecting... " << endl;
        connect();
        update_state();
# if 0
        try
        {
            update_state();
        }
        catch(Tango::DevFailed &e)
        {
            Tango::Except::print_exception(e);
            INFO_STREAM << "error during initialization" << endl;
            exit(1);
        }
#endif
        DEBUG_STREAM << "init device device is "<< device << endl;

} // end of init_device()


//+----------------------------------------------------------------------------
//
// method :             PSI_PC::get_device_property()
//
// description :        Read the device properties from database.
//
//-----------------------------------------------------------------------------
void PSI_PC::get_device_property()
{
        //      Initialize your default values here (if not done with  POGO).
        //------------------------------------------------------------------val->stat

        //      Read device properties from database.(Automatic code generation)
        //------------------------------------------------------------------
	Tango::DbData	dev_prop;
	dev_prop.push_back(Tango::DbDatum("Channel"));
	dev_prop.push_back(Tango::DbDatum("FODevice"));
	dev_prop.push_back(Tango::DbDatum("Interlock1"));
	dev_prop.push_back(Tango::DbDatum("Interlock2"));
	dev_prop.push_back(Tango::DbDatum("TangoDevice"));
	dev_prop.push_back(Tango::DbDatum("ConnectionType"));
	dev_prop.push_back(Tango::DbDatum("CurrentFactor"));

	//	Call database and extract values
	//--------------------------------------------
	if (Tango::Util::instance()->_UseDb==true)
		get_db_device()->get_property(dev_prop);
	Tango::DbDatum	def_prop, cl_prop;
	PSI_PCClass	*ds_class =
		(static_cast<PSI_PCClass *>(get_device_class()));
	int	i = -1;

	//	Try to initialize Channel from class property
	cl_prop = ds_class->get_class_property(dev_prop[++i].name);
	if (cl_prop.is_empty()==false)	cl_prop  >>  channel;
	else {
		//	Try to initialize Channel from default device value
		def_prop = ds_class->get_default_device_property(dev_prop[i].name);
		if (def_prop.is_empty()==false)	def_prop  >>  channel;
	}
	//	And try to extract Channel value from database
	if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  channel;

	//	Try to initialize FODevice from class property
	cl_prop = ds_class->get_class_property(dev_prop[++i].name);
	if (cl_prop.is_empty()==false)	cl_prop  >>  fODevice;
	else {
		//	Try to initialize FODevice from default device value
		def_prop = ds_class->get_default_device_property(dev_prop[i].name);
		if (def_prop.is_empty()==false)	def_prop  >>  fODevice;
	}
	//	And try to extract FODevice value from database
	if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  fODevice;

	//	Try to initialize Interlock1 from class property
	cl_prop = ds_class->get_class_property(dev_prop[++i].name);
	if (cl_prop.is_empty()==false)	cl_prop  >>  interlock1;
	else {
		//	Try to initialize Interlock1 from default device value
		def_prop = ds_class->get_default_device_property(dev_prop[i].name);
		if (def_prop.is_empty()==false)	def_prop  >>  interlock1;
	}
	//	And try to extract Interlock1 value from database
	if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  interlock1;

	//	Try to initialize Interlock2 from class property
	cl_prop = ds_class->get_class_property(dev_prop[++i].name);
	if (cl_prop.is_empty()==false)	cl_prop  >>  interlock2;
	else {
		//	Try to initialize Interlock2 from default device value
		def_prop = ds_class->get_default_device_property(dev_prop[i].name);
		if (def_prop.is_empty()==false)	def_prop  >>  interlock2;
	}
	//	And try to extract Interlock2 value from database
	if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  interlock2;

	//	Try to initialize TangoDevice from class property
	cl_prop = ds_class->get_class_property(dev_prop[++i].name);
	if (cl_prop.is_empty()==false)	cl_prop  >>  tangoDevice;
	else {
		//	Try to initialize TangoDevice from default device value
		def_prop = ds_class->get_default_device_property(dev_prop[i].name);
		if (def_prop.is_empty()==false)	def_prop  >>  tangoDevice;
	}
	//	And try to extract TangoDevice value from database
	if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  tangoDevice;

	//	Try to initialize ConnectionType from class property
	cl_prop = ds_class->get_class_property(dev_prop[++i].name);
	if (cl_prop.is_empty()==false)	cl_prop  >>  connectionType;
	else {
		//	Try to initialize ConnectionType from default device value
		def_prop = ds_class->get_default_device_property(dev_prop[i].name);
		if (def_prop.is_empty()==false)	def_prop  >>  connectionType;
	}
	//	And try to extract ConnectionType value from database
	if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  connectionType;

	//	Try to initialize CurrentFactor from class property
	cl_prop = ds_class->get_class_property(dev_prop[++i].name);
	if (cl_prop.is_empty()==false)	cl_prop  >>  currentFactor;
	else {
		//	Try to initialize CurrentFactor from default device value
		def_prop = ds_class->get_default_device_property(dev_prop[i].name);
		if (def_prop.is_empty()==false)	def_prop  >>  currentFactor;
	}
	//	And try to extract CurrentFactor value from database
	if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  currentFactor;



        //      End of Automatic code generation
        //------------------------------------------------------------------

}
//+----------------------------------------------------------------------------
//
// method :             PSI_PC::always_executed_hook()
//
// description :        method always executed before any command is executed
//
//-----------------------------------------------------------------------------
void PSI_PC::always_executed_hook()
{

}
//+----------------------------------------------------------------------------
//
// method :             PSI_PC::read_attr_hardware
//
// description :        Hardware acquisition for attributes.
//
//-----------------------------------------------------------------------------
void PSI_PC::read_attr_hardware(vector<long> &attr_list)
{
}
//+----------------------------------------------------------------------------
//
// method :             PSI_PC::read_SoftwareWaveform
//
// description :        Extract real attribute values for SoftwareWaveform acquisition result.
//
//-----------------------------------------------------------------------------
void PSI_PC::read_SoftwareWaveform(Tango::Attribute &attr)
{
        DEBUG_STREAM << "PSI_PC::read_SoftwareWaveform(Tango::Attribute &attr) entering... "<< endl;

        long dim_x;
        Tango::DevDouble * data;

        dim_x = m_softwareWaveform.size();
        if (dim_x != 0) {
                data = &m_softwareWaveform[0];
        } else {
                data = 0;
        }

        attr.set_value(data, dim_x);
}

//+----------------------------------------------------------------------------
//
// method :             PSI_PC::write_SoftwareWaveform
//
// description :        Write SoftwareWaveform attribute values to hardware.
//
//-----------------------------------------------------------------------------
void PSI_PC::write_SoftwareWaveform(Tango::WAttribute &attr)
{
        DEBUG_STREAM << "PSI_PC::write_SoftwareWaveform(Tango::WAttribute &attr) entering... "<< endl;

        const Tango::DevDouble* res;
        long szRead = attr.get_write_value_length();
        attr.get_write_value(res);

        m_softwareWaveform.resize(szRead);
        for (long n=0; n < szRead; ++n)
                m_softwareWaveform[n] = res[n];
}

//+----------------------------------------------------------------------------
//
// method :             PSI_PC::write_I
//
// description :        write I attribute values directly to hardware.
//
//-----------------------------------------------------------------------------
void PSI_PC::write_I(Tango::WAttribute &attr)
{
    DEBUG_STREAM << "PSI_PC::write_I(Tango::WAttribute &attr) entering... "<< endl;
    wrCurrent(attr,"I"); //writes current to the device and set "CurrentSetpint" attriburte
}



//+----------------------------------------------------------------------------
//
// method :             PSI_PC::read_CurrentOffset
//
// description :        Extract real attribute values for CurrentOffset acquisition result.
//
//-----------------------------------------------------------------------------
void PSI_PC::read_CurrentOffset(Tango::Attribute &attr)
{
        DEBUG_STREAM << "PSI_PC::read_CurrentOffset(Tango::Attribute &attr) entering... "<< endl;
        attr.set_value(attr_CurrentOffset_read);
}

//+----------------------------------------------------------------------------
//
// method :             PSI_PC::write_CurrentOffset
//
// description :        Write CurrentOffset attribute values to hardware.
//
//-----------------------------------------------------------------------------
void PSI_PC::write_CurrentOffset(Tango::WAttribute &attr)
{
        DEBUG_STREAM << "PSI_PC::write_CurrentOffset(Tango::WAttribute &attr) entering... "<< endl;
        attr.get_write_value(*attr_CurrentOffset_read);
        wrCurrent(attr, "CurrentOffset");
        DEBUG_STREAM << "value " << *attr_CurrentOffset_read << endl;
}

//+----------------------------------------------------------------------------
//
// method :             PSI_PC::read_Current
//
// description :        Extract real attribute values for Current acquisition result.
//                      returns value calculated according to the formula in specification:
//                       I = I_ideal + (I_read + I_write) / f
//
//-----------------------------------------------------------------------------
void PSI_PC::read_Current(Tango::Attribute &attr)
{
        double const I = readCurrent();
        *attr_I_read = I;
        double long const Il = I;
        double long const fl = currentFactor;
        double long const ol = *attr_CurrentOffset_read;
        *attr_Current_read = Il*fl + ol;
        attr.set_value(attr_Current_read);
}



//+----------------------------------------------------------------------------
//
// method :             PSI_PC::read_RemoteMode
//
// description :        Extract real attribute values for RemoteMode acquisition result.
//      this attrubute is glued to 1 in PSC DS becasue there is only remote control of PSC possible
//
//-----------------------------------------------------------------------------
void PSI_PC::read_RemoteMode(Tango::Attribute &attr)
{
    attr.set_value(attr_RemoteMode_read);
}

//+----------------------------------------------------------------------------
//
// method :             PSI_PC::read_Errors
//
// description :        Extract real attribute values for ErrorMsgs acquisition result.
//      It returns a table of detected error messages, the same messages are not repeated in the table even if the error repeates
//       the table is cleared when the "clear errors" command is executed
//
//-----------------------------------------------------------------------------
void PSI_PC::read_Errors(Tango::Attribute &attr)
{
    attr.set_value(attr_Errors_read, numo_ErrorMsgs, 0, false);
}

//+----------------------------------------------------------------------------
//
// method :             PSI_PC::read_I
//
// description :        Extracts real attribute values for Current acquisition result.
//
//-----------------------------------------------------------------------------
void PSI_PC::read_I(Tango::Attribute &attr)
{
        DEBUG_STREAM << "PSI_PC::read_Current(Tango::Attribute &attr) entering... "<< endl;
        *attr_I_read = readCurrent();
        attr.set_value(attr_I_read);
        double Iref = readCurrentSetpointFromDevice();
        get_device_attr()->get_w_attr_by_name("I").set_write_value(Iref);
}

//+----------------------------------------------------------------------------
//
// method :             PSI_PC::read_CurrentSetpoint
//
// description :        Extract real attribute values for CurrentSetpoint acquisition result.
//      I_ideal - current seting desired by the user - it is currentWrite value altered using provided formula:
//      I_write = f(I_I_deal - b)/f
//
//-----------------------------------------------------------------------------
void PSI_PC::read_CurrentSetpoint(Tango::Attribute &attr)
{
        double Iref = readCurrentSetpointFromDevice();
        *attr_CurrentSetpoint_read = (Iref-*attr_CurrentOffset_read)/currentFactor;
        attr.set_value(attr_CurrentSetpoint_read);
}

//+----------------------------------------------------------------------------
//
// method :             PSI_PC::write_CurrentSetpoint
//
// description :        Write CurrentSetpoint attribute values to hardware.
//      I_ideal - the input value of the command is inputed to the formula specified in doc. the output value (I_write) of the formula is
//      the value which is writen to the device as setpoint current value
//      I_write = f*(I_ideal+ offset)
//
//-----------------------------------------------------------------------------
void PSI_PC::write_CurrentSetpoint(Tango::WAttribute &attr)
{
        DEBUG_STREAM << "PSI_PC::write_CurrentSetpoint(Tango::WAttribute &attr) entering... "<< endl;
        wrCurrent(attr,"CurrentSetpoint");      //writes current to the device and set "Current" attriburte

}

//+----------------------------------------------------------------------------
//
// method :             PSI_PC::read_Voltage
//
// description :        Extract real attribute values for Voltage acquisition result.
// reads voltage from the appropriate register
//
//-----------------------------------------------------------------------------
void PSI_PC::read_Voltage(Tango::Attribute &attr)
{

        int data;
        try
        {
                psc_read(channel,0x0, PSC_VOLTAGE, &data);
        }
        catch(Tango::DevFailed &e)
        {
                Tango::Except::re_throw_exception(e,
                        (const char *)"Command failed",
                        (const char *)"Error from psc",
                        (const char *) "PSI_PC::read_Voltage()", Tango::ERR);
        }
/**
COMMAND : now idea why it works and how - taken from electra DS, if you can offer something better, you are welcome.....:)
        *attr_ScalingFactor_read = (double) *(float*)(&val.data);
*/
        *attr_Voltage_read = (double) *(float*)(&data);
        attr.set_value(attr_Voltage_read);

}


//+----------------------------------------------------------------------------
//
// method :             PSI_PC::read_Error
//
// description :        Extract real attribute values for Error acquisition result.
//      updates table with error codes, the codes are associated with different categories (ex. communication, PSC)
//      the categoris can be read from ErrorLabels attribute
//
//-----------------------------------------------------------------------------
void PSI_PC::read_ErrorCodes(Tango::Attribute &attr)
{
        DEBUG_STREAM << "PSI_PC::read_Error(Tango::Attribute &attr) entering... "<< endl;

        attr.set_value(attr_ErrorCodes_read,PSC_DS_ALBA_ERRMSG);

}

//+----------------------------------------------------------------------------
//
// method :             PSI_PC::read_ErrorLabel
//
// description :        Extract real attribute values for ErrorLabel acquisition result.
//      returns table with error codes categories
//
//-----------------------------------------------------------------------------
void PSI_PC::read_ErrorLabel(Tango::Attribute &attr)
{
        DEBUG_STREAM << "PSI_PC::read_ErrorLabel(Tango::Attribute &attr) entering... "<< endl;

        attr.set_value(attr_ErrorLabel_read, PSC_DS_ALBA_ERRMSG, 0, false);
}

//+------------------------------------------------------------------
/**
 *      method: PSI_PC::dev_state
 *
 *      description:    method to execute "State"
 *      This command gets the device state (stored in its <i>device_state</i> data member) and returns it to the caller.
 *
 * @return      State Code
 *
 */
//+------------------------------------------------------------------
Tango::DevState PSI_PC::dev_state()
{
        Tango::DevState argout = DeviceImpl::dev_state();
        return argout;
}

//+------------------------------------------------------------------
/**
 *	method:	PSI_PC::on
 *
 *	description:	method to execute "On"
 *
 *
 */
//+------------------------------------------------------------------
void PSI_PC::on()
{
        DEBUG_STREAM << "entering On()... !" << endl;

        int data;

        data = 1;
        try
        {
                psc_write(channel, 0x80, PSC_PC_ONOFF, data);
        }
        catch(Tango::DevFailed &e)
        {
                Tango::Except::re_throw_exception(e,
                        (const char *)"Command failed",
                        (const char *)"Error from psc",
                        (const char *) "PSI_PC::on()", Tango::ERR);
        }
}

//+------------------------------------------------------------------
/**
 *	method:	PSI_PC::off
 *
 *	description:	method to execute "Off"
 *
 *
 */
//+------------------------------------------------------------------
void PSI_PC::off()
{
        DEBUG_STREAM << "PSI_PC::off(): entering... !" << endl;

        int data = 0;


        try
        {
                psc_write(channel, 0x80, PSC_PC_ONOFF, data);
        }
        catch(Tango::DevFailed &e)
        {
                Tango::Except::re_throw_exception(e,
                        (const char *)"Command failed",
                        (const char *)"Error from psc",
                        (const char *) "PSI_PC::off()", Tango::ERR);
        }
}



//+------------------------------------------------------------------
/**
 *      method: PSI_PC::command
 *
 *      description:    method to execute "Command"
 *      Sends the specified command to the device and reads its respnse.
 *      This is "cooked" and device dependent in the sense
 *      the certain characters such as newlines, are appended automatically.
 *
 * @param       argin
 * @return
 *
 */
//+------------------------------------------------------------------
/*
Tango::DevString PSI_PC::command(Tango::DevString argin)
{
        //      POGO has generated a method core with argout allocation.
        //      If you would like to use a static reference without copying,
        //      See "TANGO Device Server Programmer's Manual"
        //              (chapter : Writing a TANGO DS / Exchanging data)
        //------------------------------------------------------------

        Tango::DevString argout = new char[100];
        strcpy(argout , "Command not implemented in this version of DS");



        return argout;
}
*/
//+------------------------------------------------------------------
/**
 *	method:	PSI_PC::reset
 *
 *	description:	method to execute "Reset"
 *
 *
 */
//+------------------------------------------------------------------
void PSI_PC::reset()
{
        DEBUG_STREAM << "PSI_PC::reset(): entering... !" << endl;
        set_state(Tango::UNKNOWN);
        push_change_event("State");
        set_status("reset...");
        push_change_event("Status");

//reastablish connection
        if(connectionType == FiberConnection)
        {
                init_fiber();
        }
        else if(connectionType == SerialConnection)
        {
                init_serial();
        }
        else
        {

                TangoSys_MemStream cmd_stream;
                cmd_stream <<  "choose \n0 for fiber \n1 for serial connection \n, inputed parameter value: "<< connectionType << ends;

                psc_throw_exception(
                                                        "Unknown value of property ConnectionType, choose \n0 for fiber \n1 for serial connection ",                            //info string
                                                        cmd_stream.str(),                               //describtion string
                                                        "Psc::psc_init",                // name of the function in which exception occured
                                                        CONNECTIONTYPE_PROPERTY_ERROR
                                                        );
        }


//reset PSC
        off();
        //no problem in clearing error registers in OFF state!!!!!!!!!!
/*
        if(connectionType == FiberConnection)
        {
                reset_interlockss_in_IP();
                usleep(communication_delay);
        }
*/
        reset_interlocks();



}

//+------------------------------------------------------------------
/**
 *      method: PSI_PC::clear_error_in_IP
 *
 * method clears the error counter of channel in IP on the
 * PCI IP Carrier responsible for communication with the
  * Power Supply we are controlling
 */
//+------------------------------------------------------------------
void PSI_PC::clear_errors_in_IP()
{
        int err;
        pscip_t val;


        val.chan = channel;
        if ((err = ioctl(fd, PSCIP_CLRCOUNTER, &val)))
        {
                TangoSys_MemStream  cmd_stream;
                cmd_stream << "Error when communication with psc:";
                cmd_stream << " chan=0x" << setbase (16) << ((long) (val.chan));

                psc_throw_exception(
                                                        "An attempt to access IP and clear its errors failed",                          //info string
                                                        cmd_stream.str(),                               //describtion string
                                                        "Psc::reset_interlockss_in_IP",         // name of the function in which exception occured
                                                        err  - PSCIP_IOCTL_MAGIC
                                                        );
        }



}
//+------------------------------------------------------------------
/**
 *	method:	PSI_PC::reset_interlocks
 *
 *	description:	method to execute "ResetInterlocks"
 *
 *
 */
//+------------------------------------------------------------------
void PSI_PC::reset_interlocks()
{
        DEBUG_STREAM << "PSI_PC::reset_interlocks(): entering... !" << endl;
        set_state(Tango::UNKNOWN);
        push_change_event("State");
        set_status("resetting interlocks...");
        push_change_event("Status");
        int data;
        data = 0xffffffff;      //any writing clears disorder register

        for (int i = 0; i < PSC_DS_ALBA_ERRMSG; i++)
        {
                ErrorsValue[i] = 0;
        }
        for(int i = 0; i < MAX_NUM_OF_DETECTED_ERRMSGS; i++) {
                CORBA::string_free(attr_Errors_read[i]);
                attr_Errors_read[i] = CORBA::string_dup("");
        }
        numo_ErrorMsgs = 0;

        if(connectionType == FiberConnection)
        {
                clear_errors_in_IP();
                usleep(communication_delay);
        }

        try
        {
                psc_write(channel, 0x80, PSC_ERR_MSG, data);
        }
        catch(Tango::DevFailed &e)
        {
                Tango::Except::re_throw_exception(e,
                        (const char *)"Command failed",
                        (const char *)"Error from psc",
                        (const char *) "PSI_PC::reset_interlocks()", Tango::ERR);
        }

}

//+------------------------------------------------------------------
/**
 *      method: PSI_PC::psc_read
 *
 *      description:    method to execute "Psc_read"
 *      Read values from PS using serial or fiber connection
 *
 *
 *
 */
//+------------------------------------------------------------------

void PSI_PC::psc_read(int channel, char status, char address, int *data)
{
        pscip_t val;
        val.chan = channel;
        val.stat = status;
        val.address = address;
        val.data = *data;

        if(connectionType == FiberConnection)
        {
                psc_read_fiber(&val);
        }
        else if(connectionType == SerialConnection)
        {
//              check_connection_to_PySerial();
                psc_read_serial(&val);
        }
        else
        {
                TangoSys_MemStream cmd_stream;
                cmd_stream <<  "choose \n0 for fiber \n1 for serial connection \n, inputed parameter value: "<< connectionType << ends;

                psc_throw_exception(
                                                        "Property connectionType set to wrong value",                           //info string
                                                        cmd_stream.str(),                               //describtion string
                                                        "Psc::psc_read",                // name of the function in which exception occured
                                                        CONNECTIONTYPE_PROPERTY_ERROR
                                                        );
        }
        *data = val.data ;

}

//+------------------------------------------------------------------
/**
 *      method: PSI_PC::psc_write
 *
 *      description:    method to execute "Psc_write"
 *      writes value to PSC using serial or fiber connection
 *
 */
//+------------------------------------------------------------------


void PSI_PC::psc_write(int channel, char status, char address, int data)
{
        //DEBUG_STREAM << "PSI_PC::psc_write(): entering... !" << endl;
        pscip_t val;
        val.chan = channel;
        val.stat = status;
        val.address = address;
        val.data = data;

        if(connectionType == FiberConnection)
        {

                psc_write_fiber(&val);

        }
        else if(connectionType == SerialConnection)
        {
//              check_connection_to_PySerial();

         /***priority to RS232***/
                pscip_t priority_val;
                priority_val.stat = 0x80;
                priority_val.address = 0x02;
                priority_val.data = 0x01000000;
                psc_write_serial(&priority_val);

                usleep(communication_delay);

        /*** serial write***/

                psc_write_serial(&val);

        }
        else
        {
                TangoSys_MemStream cmd_stream;
                cmd_stream <<  "choose \n0 for fiber \n1 for serial connection \n, inputed parameter value: "<< connectionType << ends;

                psc_throw_exception(
                                                        "Property connectionType set to wrong value",                           //info string
                                                        cmd_stream.str(),                               //describtion string
                                                        "Psc::psc_write",               // name of the function in which exception occured
                                                        CONNECTIONTYPE_PROPERTY_ERROR
                                                        );
        }

}


//+------------------------------------------------------------------
/**
 *      method: Psc::update_state
 *      checks PSC state and fills in appropriate tables (with error codes and with error messages)
 */
//+------------------------------------------------------------------
void PSI_PC::update_state(void)
{
    std::stringstream s_info;       //information about the error (eg. from errror codes table)
    std::stringstream s_dev_state;  //message with the state read from the device register
    std::stringstream s_status;     //message with the state read from the device register

    // first set state

    string err_msg;
    Tango::DevState PSC_state;
    /******** PC control status ********/

    int data = 0;
    try
    {
        psc_read(channel, 0x0, PSC_DEVSTATE, &data);
    }
    catch(Tango::DevFailed &e)
    {
        INFO_STREAM<< "reading device state failed" << endl;
        set_state(Tango::FAULT);
        push_change_event("State");
        s_status << "communication fault " << endl;
        set_status(s_status.str());
        push_change_event("Status");
        return;
        // Tango::Except::re_throw_exception(e,
                // (const char *)"Reading PSC failed",
                // (const char *)"Error from psc",
                // (const char *) "Psc::update_state()", Tango::ERR);
    }



    switch(data) {
        case PS_MONITOR:
            s_dev_state << "monitor mode" ;
            PSC_state = Tango::INIT;
            break;
        case PS_DEVICE_OFF:
            s_dev_state << "off" ;
            PSC_state = Tango::OFF;
            break;
        case PS_DEVICE_ON:
            s_dev_state << "power on" ;
            if (m_softwareWaveformThreadRunning)
                    PSC_state = Tango::MOVING;
            else
                    PSC_state = Tango::ON;
            break;
        case PS_ADC_CAL:
            s_dev_state << "init";
            s_info << "calibrating adc..." ;
            PSC_state = Tango::INIT;
            break;
        case PS_TRANSIENT:
            s_dev_state << "init";
            s_info << "transient" ;
            PSC_state = Tango::INIT;
            break;
        case PS_DOWNLOAD_DATA:
            s_dev_state << "init";
            s_info << "downloading data...";
            PSC_state = Tango::INIT;
            break;
        case PS_SAVE_DATA:
            s_dev_state << "init";
            s_info << "saving data..." ;
            PSC_state = Tango::INIT;
            break;
        case PS_DEVICE_LOCKED:
            s_dev_state << "fault";
            s_info << "not configured or other serious problem" ;
            PSC_state = Tango::FAULT;
            break;
        case PS_DEVICE_OFF_LOCKED:
            s_dev_state << "alarm";
            PSC_state = Tango::ALARM;
            break;
        default:
            s_dev_state << "failure to analyze state" << data;
            PSC_state = Tango::FAULT;
    }


    /******** PC error messages ********/
    // checks for errors and other interlocks
#if 0
    int psc_digin = 0;
    psc_read(channel,0x0, PSC_DIG_IN, &psc_digin);
    if (psc_digin & PSC_INTERLOCK_BITS == PSC_INTERLOCK_BITS)
    {
        PSC_state = Tango::ALARM;
        add_errmsg(interlock1.c_str());
        add_errmsg(interlock2.c_str());
        s_info << interlock1 << ", " << interlock2;
    }
    else if (psc_digin & PSC_INTERLOCK2_BIT)
    {
        PSC_state = Tango::ALARM;
        add_errmsg(interlock2.c_str());
        s_info << interlock2;
    }
    else if (psc_digin & PSC_INTERLOCK1_BIT)
    {
        PSC_state = Tango::ALARM;
        add_errmsg(interlock1.c_str());
        s_info << interlock1;
    }
#endif

    // checks other error codes
    int psc_err = 0;
    try
    {
        psc_read(channel, 0x0, PSC_ERR_MSG, &psc_err);
        /* takes into account only last byte */
        psc_err &= 0xff;
        ErrorsValue[ERR_PSI_PSC] = psc_err;
    }
    catch(Tango::DevFailed &e)
    {

        Tango::Except::re_throw_exception(e,
            (const char *)"Command failed, error reading error",
            (const char *)"Error from psc",
            (const char *) "Psc::update_state()", Tango::ERR);
    }

    // external interlock 1,2 are ignored because they are checked above
    if (psc_err) {
        PSC_state = Tango::ALARM;
        if (s_info.str().length())
        {
            s_info << ", ";
        }
        const char * msg = 0;
        if (psc_err == 0x85)
        {
            msg = interlock1.c_str();
        }
        else if (psc_err != 0x86)
        {
            msg = interlock2.c_str();
        }
        else
        {
            msg = psc_errmsg[psc_err];
        }
        add_errmsg(msg);
        s_info << msg;
    }

    if (s_info.str().length())
    {
        s_status << s_dev_state.str() << ": " << s_info.str();
    }
    else
    {
        s_status << s_dev_state.str();
    }
    set_state(PSC_state);
    set_status(s_status.str());
    push_change_event("State");
    push_change_event("Status");
} // ends update_state methods

//+------------------------------------------------------------------
/**
 *      method: PSI_PC::wrCurrent
 *
 *      writes the current
 *      input:
 *      - attr - value of the attribute - the current to be set
*       - name - name of the attribute, that is to be written
 */
//+------------------------------------------------------------------
void PSI_PC::wrCurrent(Tango::WAttribute &attr, const char * name)
{
        DEBUG_STREAM << "Psc::wrCurrent() " << endl;

        double value;

        string s_name = name;
        attr.get_write_value(value);

        if (s_name == "CurrentSetpoint")
        {
            attr_I_write = value*currentFactor + *attr_CurrentOffset_read;
            attr_CurrentSetpoint_write = value;
            get_device_attr()->get_w_attr_by_name("I").set_write_value(attr_I_write);
        }
        else if (s_name == "CurrentOffset")
        {
            attr_I_write = value*currentFactor + *attr_CurrentOffset_read;
            get_device_attr()->get_w_attr_by_name("I").set_write_value(attr_I_write);
        }
        else if (s_name == "I")
        {
            attr_I_write = value;
            attr_CurrentSetpoint_write = (value-*attr_CurrentOffset_read) / currentFactor;
            get_device_attr()->get_w_attr_by_name("CurrentSetpoint").set_write_value(attr_CurrentSetpoint_write);
        }
        else {
                ERROR_STREAM << "Psc::wrCurrent() - coding mistake " << endl;
        }

/**
COMMAND : now idea why it works and how - taken from electra DS, if you can offer something better, you are welcome.....:)
        ftmp = (float) attr_DCLinkVoltage_write;
        val.data = *(int *) (&ftmp);
*/
        const float ftmp = (float)attr_I_write;
        const int data = reinterpret_cast<const int&>(ftmp);

        try
        {
                psc_write(channel, 0x80, PSC_CURSET, data);
        }
        catch(Tango::DevFailed &e)
        {
                Tango::Except::re_throw_exception(e,
                        (const char *)"Command failed",
                        (const char *)"Error from psc",
                        (const char *) "PSI_PC::wrCurrent(()", Tango::ERR);
        }

}


//+------------------------------------------------------------------
/**
 *      method: PSI_PC::psc_get_communication_alba_errmsg
 *
 *      Decodes codes connected with communication (serial/fiber)
 *      decodes the error code comming from PSC_IP (fiber communication) driver
 *      also decodes the error codes set for serial communication
 *
 *
 */
//+------------------------------------------------------------------
const char *PSI_PC::psc_get_communication_alba_errmsg(int err)
{

        int offset = err ;
        int psc_alba_errmsg_number = sizeof(psc_communication_alba_errmsg)/PSCIP_SIZE_MSG;

        if ((offset >= 0) && (offset < psc_alba_errmsg_number))
        {
                return psc_communication_alba_errmsg[offset];
        }
        sprintf(psc_err_msg_buf, "%d", err);
        return psc_err_msg_buf;
}

//+------------------------------------------------------------------
/**
 *      method: PSI_PC::init_fiber
 *
 *
 *      decodes the error code comming from PSC_IP (fiber communication) driver
 *      also decodes the error codes set for serial communication
 *
 *
 */
//+------------------------------------------------------------------
void PSI_PC::init_fiber(void)
{
        if(!fd)
        {
                if ((fd = open(fODevice.c_str(),O_RDWR)) < 0)
                {


                        INFO_STREAM<< "Could not open device: "<< fODevice.c_str() << endl;

                        TangoSys_MemStream info_str, cmd_stream;
                        info_str << "Could not open device: "<< fODevice.c_str() << endl;
                        cmd_stream << "Could not open device: "<< fODevice.c_str() << endl;

                        psc_throw_exception(
                                                                info_str.str(),                                 //info string
                                                                cmd_stream.str(),                               //describtion string
                                                                "PSI_PC::init_fiber",           // name of the function in which exception occured
                                                                COMMUNICATION_DEVICE_ERROR
                                                                );
                }
        }
// get ModelID
/*
        int model_id;

        int data;
        try
        {
                psc_read(channel,0x0, PSC_MODEL_ID, &data);
        }
        catch(Tango::DevFailed &e)
        {
                Tango::Except::re_throw_exception(e,
                        (const char *)"Command failed, error reading error",
                        (const char *)"Error from psc",
                        (const char *) "PSI_PC::init_fiber()", Tango::ERR);
        }
        model_id = data;
*/

}



void PSI_PC::init_serial(void)
{

// create a connection to a TANGO device
        if(!device)
        {
                device = new Tango::DeviceProxy(tangoDevice.c_str());
        }
        DEBUG_STREAM << device << " " << tangoDevice.c_str() << endl;
        DEBUG_STREAM << "ping " << device->ping() << endl;

        string db_info;
        Tango::DeviceData cmd_reply;
        try
        {
                cmd_reply = device->command_inout("Open");
        }
        catch(Tango::DevFailed &e)
        {
                INFO_STREAM<< "dev already openned"<<endl;
                //change here -> check whether the device is open, if open OK, otherwise throw excetion
        }

        INFO_STREAM<<"dev successfuly openned"<< endl;
}

void PSI_PC::close_serial(void)
{
        Tango::DeviceData cmd_reply;
        try
        {
                cmd_reply = device->command_inout("Close");
        }
        catch(Tango::DevFailed &e)
        {
                INFO_STREAM<< "dev already closed"<<endl;
                //change here -> check whether the device is closed, if closed OK, otherwise throw excetion
        }
        delete device;
        device = 0;
        INFO_STREAM<< "device successfuly closed"<<endl;

}

void PSI_PC::psc_write_serial(pscip_t *pval)
{
        int err;
        pscip_t *val;
        Tango::DeviceData inDeviceData,  outputData;
        Tango::DevVarCharArray in;
        in.length(6);
        val = (pscip_t *) pval;
                                                                                        //printf("we write this data and don't care more:\n");
        in[0] = val->stat;
                                                                                        //printf("we want to write state: 0x%x \n", val->stat);
        in[1] = val->address;
                                                                                        //printf("we want to write  address: 0x%x \n", val->address);
        in[2] = (0xFF & (val->data) );
                                                                                        //printf("we want to write  data: 0x%x \n", val->data);
        in[3] = ( 0xFF & (val->data) >> 8 );
        in[4] = ( 0xFF & (val->data) >> 16 );
        in[5] = ( 0xFF & (val->data) >> 24 );

        inDeviceData << in;

        const Tango::DevVarCharArray *devBuf;

        try{
                device->command_inout("Write", inDeviceData);
                usleep(communication_delay);
                outputData = device->command_inout("ReadLine");
                outputData >> devBuf;
        }
        catch(Tango::DevFailed &e)      // catching   DevFailed and it's derivatives: ConnectionFailed, CommunicationFailed, from PySerial
        {

                TangoSys_MemStream cmd_stream;
                cmd_stream << "Writing to serial port failed:";
                cmd_stream << " chan=0x" << setbase (16) << ((long) (val->chan));
                cmd_stream << " addr=0x" << setbase (16) << ((long) (val->address));
                cmd_stream << " stat=0x" << setbase (16) << ((long) (val->stat));
                cmd_stream << " data=0x" << setbase (16) << ((long) (val->data)) << ends;
                psc_modify_and_re_throw_exception(
                                                                                e,                                                                                                                                      //exception to be modify
                                                                                "Serial communication error(RS232),try reseting PySerial msg : ",       //string modifying description message
                                                                                "PySerial failed ",                                                                                             //info string
                                                                                 cmd_stream.str(),                                                                                                      //cmd_string, describtion string
                                                                                "PSI_PC::psc_write_serial",                                                                                     // name of the function in which exception occured
                                                                                SERIAL_COMMUNICATION_ERROR                                                                                      //communication error code
                                                                                );
        }

        try{
                val->stat = (*devBuf)[0];
                val->address = (*devBuf)[1];
                val->data = 0;
                val->data = (val->data & ~0x000000FF) | ( 0xFF & (*devBuf)[2]);
                val->data = (val->data & ~0x0000FF00) | (( 0xFF & (*devBuf)[3]) << 8);
                val->data = (val->data & ~0x00FF0000) | (( 0xFF & (*devBuf)[4]) << 16);
                val->data = (val->data & ~0xFF000000) | (( 0xFF & (*devBuf)[5]) << 24);

        }
        catch(CORBA::Exception &e)              //here we have a case when the PySerial DS is on by there is no connection by RS232, however this case is ver strange
        {
                DEBUG_STREAM << "PySerial DS is ON state but there is no hardware connection by RS232, most probably the cable was disconnected during DS operation" << endl;
                set_state(Tango::FAULT);
                err = SERIAL_COMMUNICATION_ERROR;
                ErrorsValue[ERR_PSI_IP] =  err;
                TangoSys_MemStream out_stream, cmd_stream;
                out_stream << psc_get_communication_alba_errmsg(err ) << ends;
                cmd_stream << "PySerial DS is ON state but there is no hardware connection by RS232, most probably the cable was disconnected during DS operation" << endl;
                add_errmsg(psc_get_communication_alba_errmsg(err ));
                Tango::Except::throw_exception(
                        cmd_stream.str(),
                        out_stream.str(),
                        (const char *) "PSI_PC::psc_write_serial", Tango::ERR);
        }

}

void PSI_PC::psc_write_fiber(pscip_t *pval)
{
        DEBUG_STREAM << "PSI_PC::psc_write_fiber(): entering... !" << endl;
        int err;
        pscip_t *val;
        val = pval;

        if ((err = ioctl(fd, PSCIP_WRITE, val)))
        {


                TangoSys_MemStream out_stream, cmd_stream;
                ErrorsValue[ERR_PSI_IP] =  err - PSCIP_IOCTL_MAGIC;
                out_stream << psc_get_communication_alba_errmsg(err - PSCIP_IOCTL_MAGIC) << ends;
                cmd_stream << "Error when setting psc:";
                cmd_stream << " chan=0x" << setbase (16) << ((long) (val->chan));
                cmd_stream << " addr=0x" << setbase (16) << ((long) (val->address));
                cmd_stream << " stat=0x" << setbase (16) << ((long) (val->stat));
                cmd_stream << " data=0x" << setbase (16) << ((long) (val->data)) << ends;
                ERROR_STREAM << cmd_stream.str() << endl;

                add_errmsg(psc_get_communication_alba_errmsg(err - PSCIP_IOCTL_MAGIC));

                Tango::Except::throw_exception(
                        cmd_stream.str(),
                        out_stream.str(),
                        (const char *) "PSI_PC::psc_write_fiber", Tango::ERR);
        }


}

void PSI_PC::psc_read_serial(pscip_t *pval)
{
        int err;
        pscip_t *val;
        Tango::DeviceData  inputData, outputData;
        val = (pscip_t *) pval;

        const Tango::DevVarCharArray *devBuf;
        //Tango::DevVarCharArray *in = new Tango::DevVarCharArray();
        Tango::DevVarCharArray in;
        in.length(6);
//                                                      printf("w say from which address we want to read and we write this data: \n");
        in[0] = val->stat;
//                                                      printf("we want to read:state(val->state): 0x%x -->>> state((*in)[0]): 0x%x  \n", val->stat, (*in)[0] );
        in[1] = val->address;
//                                                      printf("we want to read: address(val->address): 0x%x -->>> address((*in)[1]): 0x%x \n", val->address, (*in)[1]);
        in[2] = ( 0xFF & (val->data) );
//                                                      printf("we want to read: data(val->data): 0x%x -->>> data((*in)[2]): 0x%x \n", val->data,(*in)[2]);
        in[3] = ( 0xFF & (val->data) >> 8 );
        in[4] = ( 0xFF & (val->data) >> 16 );
        in[5] = ( 0xFF & (val->data) >> 24 );

        inputData << in;

        try
        {
                device->command_inout("Write", inputData);
                usleep(communication_delay);
                outputData = device->command_inout("ReadLine");
                outputData >> devBuf;
        }
        catch(Tango::DevFailed &e) // catching   DevFailed and it's derivatives: ConnectionFailed, CommunicationFailed, from PySerial
        {

                TangoSys_MemStream cmd_stream;
                cmd_stream << "Error when reading serial port:";
                cmd_stream << " chan=0x" << setbase (16) << ((long) (val->chan));
                cmd_stream << " addr=0x" << setbase (16) << ((long) (val->address));
                cmd_stream << " stat=0x" << setbase (16) << ((long) (val->stat));
                psc_modify_and_re_throw_exception(
                                                                                e,                                                                                                                                      //exception to be modify
                                                                                "Serial communication error(RS232),try reseting PySerial msg : ",       //string modifying description message
                                                                                "PySerial failed ",                                                                                             //info string
                                                                                 cmd_stream.str(),                                                                                                      //cmd_string, describtion string
                                                                                "PSI_PC::psc_read_serial",                                                                                      // name of the function in which exception occured
                                                                                SERIAL_COMMUNICATION_ERROR                                                                                      //communication error code
                                                                                );
        }

        try{
                val->stat = (*devBuf)[0];
                val->address = (*devBuf)[1];
                val->data = 0;
                val->data = (val->data & ~0x000000FF) | ( 0xFF & (*devBuf)[2]);
                val->data = (val->data & ~0x0000FF00) | (( 0xFF & (*devBuf)[3]) << 8);
                val->data = (val->data & ~0x00FF0000) | (( 0xFF & (*devBuf)[4]) << 16);
                val->data = (val->data & ~0xFF000000) | (( 0xFF & (*devBuf)[5]) << 24);
        }
        catch(CORBA::Exception &e)              //here we have a case when the PySerial DS is on by there is no connection by RS232, however this case is ver strange
        {
                DEBUG_STREAM << "PySerial DS is ON state but there is no hardware connection by RS232, most probably the cable was disconnected during DS operation" << endl;
                set_state(Tango::FAULT);
                err = SERIAL_COMMUNICATION_ERROR;
                ErrorsValue[ERR_PSI_IP] =  err;
                TangoSys_MemStream out_stream, cmd_stream;
                out_stream << psc_get_communication_alba_errmsg(err) << ends;
                cmd_stream << "PySerial DS is ON state but there is no hardware connection by RS232, most probably the cable was disconnected during DS operation" << endl;
                add_errmsg(psc_get_communication_alba_errmsg(err));
                Tango::Except::throw_exception(
                        cmd_stream.str(),
                        out_stream.str(),
                        (const char *) "PSI_PC::psc_read_serial", Tango::ERR);
        }
}

void PSI_PC::psc_read_fiber(pscip_t *pval)
{
        int err;
        pscip_t *val;
        val = (pscip_t *) pval;
        if ((err = ioctl(fd, PSCIP_READ, val)))
        {
                TangoSys_MemStream cmd_stream;
                cmd_stream << "Error when reading psc:";
                cmd_stream << " chan=0x" << setbase (16) << ((long) (val->chan));
                cmd_stream << " addr=0x" << setbase (16) << ((long) (val->address));
                cmd_stream << " stat=0x" << setbase (16) << ((long) (val->stat));

                psc_throw_exception(
                                                        "Failed to read from PSC though fiber",                                 //info string
                                                        cmd_stream.str(),                               //describtion string
                                                        "PSI_PC::psc_read_fiber",               // name of the function in which exception occured
                                                        err - PSCIP_IOCTL_MAGIC
                                                        );


        }
}



//+------------------------------------------------------------------
/**
 *      method: Psc::dev_init
 *
 *      description:    initialize device parameters
 *      Initialize device
 *
 *
 */
//+------------------------------------------------------------------
void PSI_PC::dev_init()
{

        string tmp_string;
        /* sets error messages table */
        DEBUG_STREAM << "dev_init! " << PSC_ALBA_ERRMSG_NUM << endl;
        for (int errcode = 0; errcode < 0xFF; errcode++) {
                sprintf(psc_errmsg[errcode], "unknown error code %02x" , errcode);
        }
        for (int i = 0; i < PSC_ALBA_ERRMSG_NUM; i++) {
                int errcode;
                sscanf(psc_alba_errmsg[i*2+1],"0x%x",&errcode);
                char const * msg = psc_alba_errmsg[i*2];
                strcpy(psc_errmsg[errcode], msg);
        }


}
//+------------------------------------------------------------------
/**
 *      method: Psc::add_errmsg
 *
 *      description: adds error message to the table with detected errors
 *
 *
 */
//+------------------------------------------------------------------
const char * PSI_PC::add_errmsg(const char *msg_to_add)
{
        int i ;

        if(0==strcmp(msg_to_add,""))
        {
                msg_to_add = "ERROR, PSC code unidentified :";
        }
        for(i = 0; i < MAX_NUM_OF_DETECTED_ERRMSGS; i++)
        {
                if(!strcmp(msg_to_add, attr_Errors_read[i] ))
                        return msg_to_add;
        }
        if(numo_ErrorMsgs == MAX_NUM_OF_DETECTED_ERRMSGS-2) {
                CORBA::string_free(attr_Errors_read[numo_ErrorMsgs]);
                attr_Errors_read[numo_ErrorMsgs] = CORBA::string_dup("error buffer full");
        } else {
                CORBA::string_free(attr_Errors_read[numo_ErrorMsgs]);
                attr_Errors_read[numo_ErrorMsgs] = CORBA::string_dup(msg_to_add);
                numo_ErrorMsgs++;
        }
        return msg_to_add;
}



//+------------------------------------------------------------------
/**
 *      method: PSI_PC::dev_status
 *
 *      description:    method to execute "Status"
 *      This command gets the device status (stored in its <i>device_status</i> data member) and returns it to the caller.
 *
 * @return      Status description
 *
 */
//+------------------------------------------------------------------
Tango::ConstDevString PSI_PC::dev_status()
{

        update_state();
        return get_status().c_str();
//        Tango::ConstDevString argout = DeviceImpl::dev_status();
//        return argout;
}
//+------------------------------------------------------------------
/**
 *      method:  PSI_PC::readCurrentSetpointFromDevice()
 *
 *      description:    read PSI register holding set point value
 *
 *
 */
//+------------------------------------------------------------------
double PSI_PC::readCurrentSetpointFromDevice()
{
        int data;
        double return_value;

        try
        {
                psc_read(channel,0x0, PSC_CURREF, &data);
        }
        catch(Tango::DevFailed &e)
        {
                Tango::Except::re_throw_exception(e,
                        (const char *)"Command failed",
                        (const char *)"Error from psc",
                        (const char *) "PSI_PC::readCurrentSetpointFromDevice()", Tango::ERR);
        }

/**
COMMAND : no idea why it works and how - taken from electra DS, if you can offer something better, you are welcome.....:)
        *attr_ScalingFactor_read = (double) *(float*)(&val.data);
*/
        return_value = *(float*)(&data);

        return return_value;
}
//+------------------------------------------------------------------
/**
 *      method: PSI_PC::check_connection_to_PySerial()
 *
 *      description:    checks the connection with PySerial, to be more precise, checks the state of PySerial and tries to reopen it if it is in OFF state, if there is an error while reading state of PySerial, it also tries reopenning PySerial, if reopening fails an exception is thrown
 *
 *
 */
//+------------------------------------------------------------------
void PSI_PC::check_connection_to_PySerial()
{
        DEBUG_STREAM << "PSI_PC::check_connection_to_PySerial(): entering... !" << endl;

        Tango::DeviceData   outputData;
        Tango::DevState         c_state;
        Tango::DevVarCharArray in;
        int reconnet_flat = 0;
        try
        {
                //checking the PySerial DS state and reseting if needed
                outputData = device->command_inout("State");
                outputData >> c_state;
        }
        catch(Tango::DevFailed &e) // catching   DevFailed and it's derivatives: ConnectionFailed, CommunicationFailed, from PySerial
        {
                DEBUG_STREAM << "Error when reading PySerial state !" << endl;
                reconnet_flat = 1;
        }

        if (reconnet_flat == 1)
        {
                init_serial();
                c_state = read_PySerial_state();
        }

        if(c_state==Tango::OFF)         //if PySerial DS is off, try to reopen the connection
        {
                string db_info;
                Tango::DeviceData cmd_reply;
                try
                {
                        cmd_reply = device->command_inout("Open");
                        DEBUG_STREAM << "Reopening the PySerial DS" << endl;
                }
                catch(Tango::DevFailed &e)
                {
                        INFO_STREAM<< "dev already openned"<<endl;
                        //change here -> check whether the device is open, if open OK, otherwise throw excetion
                        //!!!!!!!!!!!!!!!!!!!

                }
        }
}
//+------------------------------------------------------------------
/**
 *      method: PSI_PC::read_PySerial_state()
 *
 *      description:    reads state of PySerial DS
 *
 *
 */
//+------------------------------------------------------------------
Tango::DevState  PSI_PC::read_PySerial_state()
{
        Tango::DeviceData   outputData;
        Tango::DevState         c_state;
        Tango::DevVarCharArray in;

        try
        {
                //checking the PySerial DS state and reseting if needed
                outputData = device->command_inout("State");
                outputData >> c_state;
        }
        catch(Tango::DevFailed &e) // catching   DevFailed and it's derivatives: ConnectionFailed, CommunicationFailed, from PySerial
        {
                psc_modify_and_re_throw_exception(
                                                                                e,                                                                                                                                      //exception to be modify
                                                                                "PySerial error when reading its state, PySerial msg: ",                        //string modifying description message
                                                                                "PySerial failed ",                                                                                             //info string
                                                                                "Error when executing PySerial command State",                                          //describtion string
                                                                                "PSI_PC::read_PySerial_state",                                                                          // name of the function in which exception occured
                                                                                SERIAL_PYSERIAL_ERROR                                                                                           //communication error code
                                                                                );
        }
        return c_state;

}
//+------------------------------------------------------------------
/**
 *      method: PSI_PC::psc_modify_and_re_throw_exception
 *
 *      description:    rethrows exception for communication failure/error changing text in the description of caught exception, it is used to notify the user that the exception comes from another device server (PySerial)
 *
 *
 */
//+------------------------------------------------------------------
void PSI_PC::psc_modify_and_re_throw_exception(
                                                                                Tango::DevFailed &e,            //exception to be modify
                                                                                string modify_s,                        //string modifying description message
                                                                                string info_s,                          //info string
                                                                                string cmd_s,                           //describtion string
                                                                                string function_name,           // name of the function in which exception occured
                                                                                int err_code
                                                                                )
{
                /***
                Modification of exectption message -adding information to the bottom erro message so that it is clear for the user that the error comes from the PySerial DS
                */
                Tango::DevError &error = e.errors[0];
                string s1;
                s1 = (char *)error.desc;
                string s2;
                s2 = modify_s + s1;
                const char * str_desc = s2.c_str();
                error.desc = CORBA::string_dup(str_desc);
                /***
                Modification of exectption message - done
                */

                INFO_STREAM << cmd_s << endl;
                ErrorsValue[ERR_PSI_IP] =  err_code ;
                TangoSys_MemStream out_stream, cmd_stream;
                out_stream << psc_get_communication_alba_errmsg(err_code ) << ends;
                cmd_stream << cmd_s;

                add_errmsg(psc_get_communication_alba_errmsg(err_code ));
                Tango::Except::print_exception(e);

                Tango::Except::re_throw_exception(e,
                        cmd_stream.str(),
                        out_stream.str(),
                        (const char *) function_name.c_str(), Tango::ERR);



}

//+------------------------------------------------------------------
/**
 *      method: PSI_PC::psc_re_throw_exception
 *
 *      description:    rethrows exception for communication failure/error
 *
 *
 */
//+------------------------------------------------------------------
void PSI_PC::psc_re_throw_exception(
                                                                                Tango::DevFailed &e,            //exception to be modify
                                                                                string info_s,                          //info string
                                                                                string cmd_s,                           //describtion string
                                                                                string function_name,           // name of the function in which exception occured
                                                                                int err_code
                                                                                )
{

                INFO_STREAM << cmd_s << endl;
                ErrorsValue[ERR_PSI_IP] =  err_code ;
                TangoSys_MemStream out_stream, cmd_stream;
                out_stream << psc_get_communication_alba_errmsg(err_code ) << ends;
                cmd_stream << cmd_s;

                add_errmsg(psc_get_communication_alba_errmsg(err_code ));
                Tango::Except::print_exception(e);

                Tango::Except::re_throw_exception(e,
                        cmd_stream.str(),
                        out_stream.str(),
                        (const char *) function_name.c_str(), Tango::ERR);

}
//+------------------------------------------------------------------
/**
 *      method: PSI_PC::psc_throw_exception
 *
 *      description:    creates and throws exception for communication failure/error
 *
 *
 */
//+------------------------------------------------------------------
void PSI_PC::psc_throw_exception(
                                                                                string info_s,                          //info string
                                                                                string cmd_s,                           //describtion string
                                                                                string function_name,           // name of the function in which exception occured
                                                                                int err_code
                                                                                )
{

                INFO_STREAM << cmd_s << endl;
                ErrorsValue[ERR_PSI_IP] =  err_code ;
                TangoSys_MemStream out_stream, cmd_stream;
                out_stream << psc_get_communication_alba_errmsg(err_code ) << ends;
                cmd_stream << cmd_s;

                add_errmsg(psc_get_communication_alba_errmsg(err_code ));

                Tango::Except::throw_exception(
                        cmd_stream.str(),
                        out_stream.str(),
                        (const char *) function_name.c_str(), Tango::ERR);

}
//+------------------------------------------------------------------
/**
 *      method: PSI_PC::readCurrent()
 *
 *      description:    method reads the "measured current",
 *
 *
 */
//+------------------------------------------------------------------
double PSI_PC::readCurrent()
{
        int data;
        double value;

        try
        {
                psc_read(channel,0x0, PSC_CURRENT, &data);
        }
        catch(Tango::DevFailed &e)
        {
                Tango::Except::re_throw_exception(e,
                        (const char *)"Command failed",
                        (const char *)"Error from psc",
                        (const char *) "PSI_PC::read_Current", Tango::ERR);
        }
/**
COMMAND : now idea why it works and how - taken from electra DS, if you can offer something better, you are welcome.....:)
        *attr_ScalingFactor_read = (double) *(float*)(&val.data);
*/
        value = reinterpret_cast<float&>(data);

        return value;
}





//+------------------------------------------------------------------
/**
 *	method:	PSI_PC::disable_interlocks
 *
 *	description:	method to execute "DisableInterlocks"
 *
 *
 */
//+------------------------------------------------------------------
void PSI_PC::disable_interlocks()
{
        DEBUG_STREAM << "PSI_PC::disable_interlocks(): entering... !" << endl;

        //if(connectionType == SerialConnection)
        if(true)
        {
                const int data = 0xE070;
                try
                {
                        psc_write(channel, 0x80, PSC_DIG_IN_MASK, data);
                }
                catch(Tango::DevFailed &e)
                {
                        Tango::Except::re_throw_exception(e,
                                (const char *)"Command failed",
                                (const char *)"Error from psc",
                                (const char *) "PSI_PC::disable_interlocks()", Tango::ERR);
                }
        }
        else
        {
                Tango::Except::throw_exception(
                                (const char *)"Command allowed only when connected by Serial Line (RS232)",
                                (const char *)"Error from Device Server",
                                (const char *) "PSI_PC::disable_interlocks()", Tango::ERR);

        }
        //      Add your own code to control device here

}

//+------------------------------------------------------------------
/**
 *	method:	PSI_PC::enable_interlocks
 *
 *	description:	method to execute "EnableInterlocks"
 *
 *
 */
//+------------------------------------------------------------------
void PSI_PC::enable_interlocks()
{
        DEBUG_STREAM << "PSI_PC::enable_interlocks(): entering... !" << endl;

        if(connectionType == SerialConnection)
        {
                const int data = 0xF877;
                try
                {
                        psc_write(channel, 0x80, PSC_DIG_IN_MASK, data);
                }
                catch(Tango::DevFailed &e)
                {
                        Tango::Except::re_throw_exception(e,
                                (const char *)"Command failed",
                                (const char *)"Error from psc",
                                (const char *) "PSI_PC::enable_interlocks()", Tango::ERR);
                }
        }
        else
        {
                Tango::Except::throw_exception(
                                (const char *)"Command allowed only when connected by Serial Line (RS232)",
                                (const char *)"Error from Device Server",
                                (const char *) "PSI_PC::enable_interlocks()", Tango::ERR);

        }
}




//+------------------------------------------------------------------
/**
 *	method:	PSI_PC::interlock_status
 *
 *	description:	method to execute "InterlockStatus"
 *
 * @return	Interlocks Status
 *
 */
//+------------------------------------------------------------------
Tango::DevString PSI_PC::interlock_status()
{
        //      POGO has generated a method core with argout allocation.
        //      If you would like to use a static reference without copying,
        //      See "TANGO Device Server Programmer's Manual"
        //              (chapter : Writing a TANGO DS / Exchanging data)
        //------------------------------------------------------------
        Tango::DevString        argout  = new char[25];

        DEBUG_STREAM << "PSI_PC::interlocks_status(): entering... !" << endl;

        string err_msg;
        /******** PC control status ********/

        int data;
        try
        {
                psc_read(channel,0x0, PSC_DIG_IN_MASK, &data);

        }
        catch(Tango::DevFailed &e)
        {
                 Tango::Except::re_throw_exception(e,
                         (const char *)"Reading PSC failed",
                         (const char *)"Error from psc",
                         (const char *) "Psc::interlocks_status()", Tango::ERR);
        }

        if((data & 0xFFFF) == 0xE070)
        {
                strcpy(argout, "Interlocks disabled");
        }
        else if((data & 0xFFFF) == 0xF877)
        {
                strcpy(argout, "Interlocks enabled");
        }
        else
        {
                strcpy(argout, "Interlocks customized");
        }
        return argout;
}










//+------------------------------------------------------------------
/**
 *      method: PSI_PC::read__psc_register
 *
 *      description:    method to execute "Read_PSC_register"
 *      Allows to read any register in PSC
 *
 * @param       argin   Register address
 * @return
 *
 */
//+------------------------------------------------------------------
//Tango::DevDouble PSI_PC::read__psc_register(Tango::DevShort argin)
//{
//      Tango::DevDouble        argout ;
//      DEBUG_STREAM << "PSI_PC::read__psc_register(): entering... !" << endl;
//
//      //      Add your own code to control device here
//
//
//
//      return argout;
//}


//+------------------------------------------------------------------
/**
 *	method:	PSI_PC::read__psc_register
 *
 *	description:	method to execute "Read_PSC_register"
 *
 * @param	argin	Register address and return format (f-float,i-integer,x-hex, by default x), ex input: 0x00 x - address 0, return value hex
 * @return	Data, hexadecimal
 *
 */
//+------------------------------------------------------------------
Tango::DevString PSI_PC::read__psc_register(Tango::DevString argin)
{
        //      POGO has generated a method core with argout allocation.
        //      If you would like to use a static reference without copying,
        //      See "TANGO Device Server Programmer's Manual"
        //              (chapter : Writing a TANGO DS / Exchanging data)
        //------------------------------------------------------------

        DEBUG_STREAM << "PSI_PC::read__psc_register(): entering... !" << endl;

        Tango::DevString        argout  = new char[20];
        string inputValue = argin;
        char format;
        int address;

        if(inputValue.size() == 4)
        {
                sscanf(inputValue.c_str(),"0x%x", &address);
                format = 'x';
        }
        else if(inputValue.size() == 6)
        {
                        sscanf(inputValue.c_str(),"0x%x %c", &address,&format);
        }
        else
        {
                TangoSys_MemStream out_stream;
                out_stream << "Input data format is incorrect: " << argin << endl;
                Tango::Except::throw_exception(
                        out_stream.str(),
                        "Error from DS",
                        "PSI_PC::read__psc_register()", Tango::ERR);
        }


        for(unsigned int i = 0; i < sizeof(psc_write_only_addresses)/sizeof(int); i++)  //get number of restricted addresses
        {
                if(psc_write_only_addresses[i] == address)
                {
                        TangoSys_MemStream out_stream;
                        out_stream << "You are not allowed to read address: " << argin << endl;
                        Tango::Except::throw_exception(
                                out_stream.str(),
                                "Error from DS - Write Only address",
                                "PSI_PC::read__psc_register()", Tango::ERR);

                }
        }


        int data;
        try
        {
                psc_read(channel,0x0, address, &data);
        }
        catch(Tango::DevFailed &e)
        {

                Tango::Except::re_throw_exception(e,
                        "Command failed, error reading error",
                        "Error from psc",
                        "PSI_PC::read__psc_register()", Tango::ERR);
        }

        if(format == 'x')
        {
                sprintf(argout, "0x%x",data);
        }
        else if(format == 'f')
        {
                sprintf(argout, "%f",*(float*)(&data));
        }
        else if(format == 'i')
        {
                sprintf(argout, "%d",data);
        }
        else
        {
                TangoSys_MemStream out_stream;
                out_stream << "Output data format parameter is incorrect: " << format << endl;
                Tango::Except::throw_exception(
                        out_stream.str(),
                        "Error from DS",
                        "PSI_PC::read__psc_register()", Tango::ERR);
        }


        return argout;
}

void PSI_PC::do_software_waveform()
{
        std::cout << "A cagar a l'horta" << std::endl;

        /// @todo No one should modify m_softwareWaveform while we are here
        size_t n =0;
        const size_t sz = m_softwareWaveform.size();

        NanoSeconds original_time;
        NanoSeconds begin_time, end_time, sleep_time, wanted_sleep_time;
        NanoSeconds next_step;

        wanted_sleep_time.set(0, 100000); // 100000 ns = 10KH

        while(m_softwareWaveformThreadRunning) {

                original_time.be_current_time();

                begin_time = original_time;
                next_step = begin_time;

                size_t no_slept = 0, no_slept_serie=0;
                bool previous_slept = true;

                for (n =0; n < sz; ++n) {
//                      wrCurrent(attr,"Iref");

                        const double value = m_softwareWaveform[n];
                        const double corrected_value = (double)currentFactor * (double)value  + (double)*attr_CurrentOffset_read;

                        attr_I_write = corrected_value;
                        attr_CurrentSetpoint_write = value;

                        //this->get_device_attr()->get_w_attr_by_name("Iref").set_write_value(corrected_value);

                        const float ftmp = (float)attr_I_write;
                        const int32_t data = reinterpret_cast<const int32_t&>(ftmp);

                        try {
                                psc_write(channel, 0x80, PSC_CURSET, data);
                        }
                        catch(Tango::DevFailed &e)
                        {
                                std::cout << "ERROR AT n=" << n << std::endl;
                                Tango::Except::re_throw_exception(e,
                                        (const char *)"Command failed",
                                        (const char *)"Error from psc",
                                        (const char *) "PSI_PC::do_software_waveform(()", Tango::ERR);
                        }

                        if(!m_softwareWaveformThreadRunning)
                                break;

                        end_time.be_current_time();
///--opt 1
//                      while (end_time > next_step) {
//                              next_step = next_step + wanted_sleep_time;
//                      }
//                      sleep_time = nextStep - end_time;
//                      sleep_time.sleep();
///--opt 2
                        next_step = next_step + wanted_sleep_time;

                        if (!(end_time > next_step)) {
                                sleep_time = next_step - end_time;
                                previous_slept = true;
                                sleep_time.sleep();
                        } else {
                                // we are late, no sleeping this time!
                                ++no_slept;
                                if (previous_slept)
                                        ++no_slept_serie;
                                previous_slept = false;
                        }
///-------
                        begin_time.be_current_time();
                }
                end_time.be_current_time();

                std::cout << "--------- 'SECOND' -----------" << std::endl
                          << "began at:       " << original_time << std::endl
                          << "finished at:    " << end_time << std::endl
                          << "difference:     " << (end_time - original_time) << std::endl
                          << "no_slept:       " << no_slept << std::endl
                          << "no_slept_serie: " << no_slept_serie << std::endl
                          << "********************" << std::endl;
        }

        std::cout << "Software Waveform Thread end" << std::endl;
}


/*static*/ void* PSI_PC::_software_waveform_thread(void* ptr)
{
        PSI_PC* obj = (PSI_PC*)ptr;
        obj->do_software_waveform();
        return 0;
}

//+------------------------------------------------------------------
/**
 *	method:	PSI_PC::stop_software_waveform
 *
 *	description:	method to execute "StopSoftwareWaveform"
 *
 *
 */
//+------------------------------------------------------------------
void PSI_PC::stop_software_waveform()
{
        DEBUG_STREAM << "PSI_PC::stop_software_waveform(): entering... !" << endl;

        if (m_softwareWaveformThreadRunning) {
                void* _ignored;
                omni_thread* thr = m_softwareWaveformThread;
                m_softwareWaveformThreadRunning = false;
                m_softwareWaveformThread = 0;

                thr->join(&_ignored);
        }
}

//+------------------------------------------------------------------
/**
 *	method:	PSI_PC::start_software_waveform
 *
 *	description:	method to execute "StartSoftwareWaveform"
 *
 *
 */
//+------------------------------------------------------------------
void PSI_PC::start_software_waveform()
{
        DEBUG_STREAM << "PSI_PC::start_software_waveform(): entering... !" << endl;

        //      Add your own code to control device here
        m_softwareWaveformThreadRunning = true;
        m_softwareWaveformThread = omni_thread::create(_software_waveform_thread, (void*)this, omni_thread::PRIORITY_HIGH);
}


//+------------------------------------------------------------------
/**
 *	method:	PSI_PC::connect
 *
 *	description:	method to execute "Connect"
 *
 *
 */
//+------------------------------------------------------------------
void PSI_PC::connect()
{
        DEBUG_STREAM << "PSI_PC::connect(): entering... !" << endl;
        if (connectionType == SerialConnection) {
                check_connection_to_PySerial();
        }
        //      Add your own code to control device here

}






//+------------------------------------------------------------------
/**
 *	method:	PSI_PC::update
 *
 *	description:	method to execute "Update"
 *
 *
 */
//+------------------------------------------------------------------
void PSI_PC::update()
{
    update_state();
}



}	//	namespace
