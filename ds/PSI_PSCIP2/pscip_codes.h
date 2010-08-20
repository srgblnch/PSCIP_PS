#define PSC_DS_ALBA_ERRMSG 				5		//describes the number of elements in the  table ( psc_ds_alba_errmsg[])

#define SERIAL_PYSERIAL_ERROR	 		0x0A
#define SERIAL_COMMUNICATION_ERROR 		0x0B
#define CONNECTIONTYPE_PROPERTY_ERROR 	0x0C
#define CONNECTIONTYPE_PROPERTY_ERROR 	0x0C
#define COMMUNICATION_DEVICE_ERROR		0x0D
#define RS232_PRIORITY_ERROR			0x0E

static char psc_communication_alba_errmsg[][PSCIP_SIZE_MSG] = {
	"Echo value doesn't match",						//"0x00",
	"Remote control disabled",						//"0x01",
	"Generic reading error",						//"0x02",
	"Data timeout",									//"0x03",
	"Input buffer full",							//"0x04",
	"Dsp stopped",									//"0x05",
	"Link is down",									//"0x06",
	"Transmission error",							//"0x07",	
	"Parameter error",								//"0x08",
	"PCI bus error",								//"0x09",
	"Connection to pySerial failed",				//"0x0A",
	"Serial connection error (RS232)", 				//"0x0B",
	"Unknown value of ConnectionType property",		//"0x0C"
	"Could not open communication device",			//"0x0D"
	"RS232 priority error"							//"0x0E"
	
};

