#define PSC_DS_ALBA_ERRMSG 				5		//describes the number of elements in the  table ( psc_ds_alba_errmsg[])

#define SERIAL_PYSERIAL_ERROR	 		0x0A
#define SERIAL_COMMUNICATION_ERROR 		0x0B
#define CONNECTIONTYPE_PROPERTY_ERROR 	0x0C
#define CONNECTIONTYPE_PROPERTY_ERROR 	0x0C
#define COMMUNICATION_DEVICE_ERROR		0x0D
#define RS232_PRIORITY_ERROR			0x0E

static const char psc_communication_alba_errmsg[][PSCIP_SIZE_MSG] = {
    "echo value does not match",						//"0x00",
    "remote control disabled",						//"0x01",
    "generic reading error",						//"0x02",
    "data timeout",									//"0x03",
    "input buffer full",							//"0x04",
    "DSP stopped",									//"0x05",
    "link is down",									//"0x06",
    "transmission error",							//"0x07",
    "parameter error",								//"0x08",
    "PCI bus error",								//"0x09",
    "connection to Serial failed",				//"0x0A",
    "serial connection error (RS232)", 				//"0x0B",
    "unknown value of ConnectionType property",		//"0x0C"
    "could not open communication device",			//"0x0D"
    "RS232 priority error"							//"0x0E"
};

