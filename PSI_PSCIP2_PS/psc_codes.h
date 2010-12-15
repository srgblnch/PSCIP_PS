//
// file :         psc_codes.h
//
// description :  definition of psc command codes and error messages
//
// project :      TANGO Device Server
//
// $Author:  $ Giulio Gaio
//
// $Revision:  $ 1.0
//
// $Log:  $
//
// copyleft :   Sincrotrone Trieste S.C.p.A.
//              Strada Statale 14 - km 163,5 in AREA Science Park
//              34012 Basovizza, Trieste ITALY
//



#define PSC_BEND_ID 120
#define PSC_QF_ID   100
#define PSC_QD_ID   101
#define PSC_SEXT_ID 106
#define PSC_CORR_ID 30

#define PSC_QF_DEVID   508
#define PSC_QD_DEVID   507

#define PSC_SW_TRIG       0x2  /* software trigger */
#define PSC_RELOADPRG     0x21 /* reload program */
#define PSC_MODEL_ID      0x22 /* model ID */
#define PSC_DEVSTATE      0x24 /* PC control status */
#define PSC_ERR_MSG       0x29 /* current error messages */
#define PSC_UPLOAD_WAVE   0x30
#define PSC_WRITE_WAVE    0x31
#define PSC_SAVE_WAVE     0x32
#define PSC_WAVE_IDX      0x33
#define PSC_SAMPLE_NUM    0x35
#define PSC_DEV_ID        0x3b
#define PSC_PC_ONOFF      0x3c
#define PSC_WAVE_NUM      0x50
#define PSC_WAVE_CMD      0x51
#define PSC_STEPSIZE      0x52
#define PSC_SCALEFACTOR   0x53
#define PSC_RAMPING       0x54
#define PSC_AUTOTRIG      0x55
#define PSC_WAITBREAK     0x56
#define PSC_SAMPLE_CNT    0x57
#define PSC_DIG_IN        0x7c
#define PSC_DIG_IN_MASK   0x7d
#define PSC_DIG_OUT       0x7e
#define PSC_WR_VDC_LINK   0x85
#define PSC_B_WR_VDC_LINK 0x4F /* bending vdc link */
#define PSC_PWRON_ACDC    0x8a /* Kempower contattore */
#define PSC_KEMP_ALARM    0x8b /* Kempower allarme */
#define PSC_CURSET        0x90
#define PSC_CURREF        0x96 /* Reference current*/
#define PSC_CURRENT       0x9c
#define PSC_VOLTAGE       0x9d
#define PSC_LOADRES       0x9e
#define PSC_RD_VDC_LINK   0x9f
#define PSC_IERROR        0xa0
#define PSC_OUTPWR        0xa2
#define PSC_RD1_VDC_LINK  0xa8 /* Kempower Bending ch1 */
#define PSC_RD2_VDC_LINK  0xa9 /* Kempower Bending ch2 */


#define ERR_PSI_IP      0
#define ERR_PSI_PSC     1
#define ERR_INT_TEMP    2
#define ERR_INT_FLOW    3
#define ERR_INT_OTHER   4

#define ERR_PSC_INTERLOCK 0x88          //changed 0x20

#define PSC_INTERLOCK1_BIT (1<<1)
#define PSC_INTERLOCK2_BIT (1<<14)
#define PSC_INTERLOCK_BITS (PSC_INTERLOCK1_BIT | PSC_INTERLOCK2_BIT)

static const int PSC_ERR_EXTERNAL1 = 0x85;
static const int PSC_ERR_EXTERNAL2 = 0x86;
static const int PSC_ERR_EXTERNAL3 = 0x87;
static const int PSC_ERR_EXTERNAL4 = 0x88;
static const int PSC_ERR_INIT = -1;
static const int PSC_ERR_RW = -2;


static const unsigned long communication_delay = 10000; //used in serial communication mainly

static const int psc_write_only_addresses[] = {
        0x02,
        0x20,
        0x21,
        0x31,
        0x32,
        0x40
        };

static const int PSC_ALBA_ERRMSG_NUM = 178;
static const char psc_alba_errmsg[2*PSC_ALBA_ERRMSG_NUM][PSCIP_SIZE_MSG] = {

        /* General return codes - 0x0 to 0xf */

        "No disorders","0x0",
        "Device state error","0x1",
        "Device supervisor disabled","0x2",
        "Command abort","0x3",
        "Data not stored","0x4",
        "Error erasing flash","0x5",
        "Communication break","0x6",
        "Internal communication error","0x7",
        "Master card error","0x8",
        "Internal buffer full","0x9",
        "Wrong sector","0xa",
        "Data not copied","0xb",
        "Wrong download parameters","0xc",
        "Device parametrization error","0xd",
        "Fpga version error","0xe",
        "Timeout DC link voltage","0x10",

        /* Timeouts - 0x10 to 0x1f */

        "Timeout Auxiliary relay on","0x11",
        "Timeout Auxiliary relay off","0x12",
        "Timeout main relay on","0x13",
        "Timeout main relay off","0x14",
        "Timeout data download","0x15",
        "Timeout SSC on","0x16",
        "Timeout MC on","0x17",
        "Timeout MBC off","0x18",
        "Timeout SSC off","0x19",
        "Timeout MC off","0x1a",
        "Timeout flex sync","0x1b",

        /* Digital I/O signals - 0x20 to 0x6f */

        "Interlock","0x20",
        "Master switch","0x21",
        "Magnet interlock","0x22",
        "Temperature transformer","0x23",
        "Temperature rectifier","0x24",
        "Temperature converter","0x25",
        "Current transducer","0x26",
        "Temperature polarity switch","0x27",
        "Power semiconductor","0x28",
        "Main relay","0x29",
        "AD converter card","0x2a",
        "Polarity switch","0x2b",
        "Auxiliary relay","0x2c",
        "Master switch T1","0x2d",
        "Master switch T2","0x2e",
        "Temperature magnet","0x2f",
        "Water magnet","0x30",
        "Water rack","0x31",
        "Cabinet fan","0x32",
        "Mb fan alarm","0x33",
        "Subrack fan 1","0x34",
        "Subrack fan 2","0x35",
        "Subrack fan 3","0x36",
        "Multiple subrack fans","0x37",
        "Bulk DC power","0x38",
        "Inverter","0x39",
        "Control power","0x3a",
        "Phase alarm","0x3b",
        "Emergency stop alarm","0x3c",
        "Door alarm","0x3d",
        "Earth stick alarm","0x3e",
        "Earth switch alarm","0x3f",
        "Earth stick or door alarm","0x40",
        "Dump switch alarm","0x41",
        "Isolation switch alarm","0x42",
        "Isolation switch warning","0x43",
        "Earth leakage alarm","0x44",
        "Multiple isolation switch alarm","0x45",
        "QF MPS interlock alarm","0x46",
        "QF temperature alarm","0x47",
        "QF MPS interlock alarm","0x48",
        "QF temperature alarm","0x49",
        "Temperature magnet alarm","0x4a",
        "Heatsink1 alarm","0x4b",
        "Heatsink2 alarm","0x4c",
        "SCR current alarm","0x4d",
        "SCR2 current alarm","0x4e",
        "Pss 1 alarm","0x4f",
        "Pss 2 alarm","0x50",
        "Aux 24v alarm","0x51",
        "Aux 15v alarm","0x52",
        "Mcb alarm","0x53",
        "Ssc alarm","0x54",
        "Mc alarm","0x55",
        "Sub mc alarm","0x56",
        "Charger enable","0x57",
        "Module dsp","0x58",
        "Dcct alarm","0x59",
        "Power module","0x5a",
        "Power module multiple","0x5b",
        "Door a","0x5c",
        "Ventilator","0x5d",
        "Emergency switch","0x5e",
        "Thermal switch alarm","0x5f",
        "Heatsink alarm","0x60",
        "Magnetics alarm","0x61",
        "Sez1 fuse alarm","0x62",
        "Sez2 fuse alarm","0x63",
        "Sez3 fuse alarm","0x64",

        /* FuG Controller digital inputs */

        "Alarm 24V PSU ok","0x65",
        "Alarm ground switch NC","0x66",
        "Alarm dump switch NC","0x67",
        "Alarm contractor AUX","0x68",
        "Alarm charger enable","0x69",
        "Alarm analogue input link","0x6a",
        "Timeout dump switch off","0x6b",
        "Timeout ground switch off","0x6c",
        "Timeout dump switch on","0x6d",
        "Timeout ground switch on","0x6e",
        "Alarm inverter over current","0x6f",

        /* Thresholds - 0x70 to 0x8f */

        "Load current too high","0x70",
        "DC link voltage too low","0x71",
        "DC link voltage too high","0x72",
        "Load voltage too high","0x73",
        "Load current ripple too high","0x74",
        "Load isolation not ok","0x75",
        "Load impedance out of range","0x76",
        "Shut off current too high","0x77",
        "Load dc current too high","0x78",
        "BR current too high","0x79",
        "QF IND ON current too high","0x7a",
        "QD IND ON current too high","0x7b",
        "QD IND OFF current too high","0x7c",
        "QD IND OFF current too high","0x7d",
        "Thyristor current too high","0x7e",
        "Thyristor current too high 2","0x7f",
        "Load voltage too high 1","0x80",

        /* External digital interlocks request JAD 23.07.08 */

        "External interlock 1","0x85",
        "External interlock 2","0x86",
        "External interlock 3","0x87",
        "External interlock 4","0x88",

        /* Booster Dipole alarms */

        "Zero I alarm","0x90",
        "Sez4 fuse alarm","0x91",
        "Sez5 fuse alarm","0x92",
        "OTEMP TX alarm","0x93",
        "OTEMP L9L13 alarm","0x94",
        "OP valid alarm","0x95",
        "Water flow alarm","0x96",
        "Temperature alarm","0x97",
        "OTEMP L1L5 alarm","0x98",
        "OTEMP l4L8 alarm","0x99",
        "OTEMP L4L8 alarm","0x9a",
        "OTEMP L2L6 alarm","0x9b",

        /* Slave communication errors */
        /* Autocheck errors */

        "Comms error autocheck","0xa0",
        "Comms error M","0xa1",
        "Comms error SL1","0xa2",
        "Comms error SL2","0xa3",
        "Comms error SL3","0xa4",
        "Comms error SL4","0xa5",
        "Comms error PPM","0xa6",
        "Comms error PP1","0xa7",
        "Comms error PP2","0xa8",
        "Comms error PP3","0xa9",
        "Comms error PP4","0xaa",

        /* SR Dipole extra alarms */

        "RMU alarm","0xb0",
        "ASSC alarm","0xb1",
        "SSCB alarm","0xb2",
        "Timeout RMU alarm ON","0xb3",
        "Timeout ASSC alarm ON","0xb4",
        "Timeout SSCB alarm ON","0xb5",
        "Timeout RMU alarm OFF","0xb6",
        "Timeout ASSC alarm OFF","0xb7",
        "Timeout SSCB alarm OFF","0xb8",
        "DSES alarm","0xb9",
        "Soft start alarm","0xba",

        /* For testing use only - 0xf0 to 0xff */

        "Test error 1","0xf0",
        "Test error 2","0xf1",
        "Test error 3","0xf2",
        "Test error 4","0xf3",
        "Test error 5","0xf4",
        "Test error 6","0xf5",
        "Test error 7","0xf6",
        "Test error 8","0xf7",
        "Test error 9","0xf8",
        "Test error 10","0xf9",
        "Test error 11","0xfa",
        "Test error 12","0xfb",
        "Test error 13","0xfc",
        "Test error 14","0xfd",
        "Test error 15","0xfe",
        "Test error 16","0xff",

};


#define PSC_KEMPOWER_ERRMSG 36
static const char psc_kempower_errmsg[][PSCIP_SIZE_MSG] = {
        "No disorders", "0x0",
        "Device state error", "0x1",
        "Device supervisor disabled", "0x2",
        "Command abort", "0x3",
        "Data not stored", "0x4",
        "Error erasing flash", "0x5",
        "Communication break", "0x6",
        "Internal communication error", "0x7",
        "Master card error", "0x8",
        "Internal buffer full", "0x9",
        "Wrong sector", "0xa",
        "Data not copied", "0xb",
        "Wrong download parameters", "0xc",
        "Device parametrization error", "0xd",
        "Timeout dc link voltage", "0x10",
        "Timeout data download", "0x15",
        "Magnet fault", "0x21",
        "Kempower AC DC off", "0x23",
        "Warning some module fault", "0x24",
        "Kempower fault", "0x25",
        "Kempower not ready", "0x26",
        "Power semiconductor", "0x28",
        "Kempower controller off", "0x29",
        "AD converter card", "0x2a",
        "Kempower control off line", "0x2c",
        "Emergency switch", "0x5e",
        "Load current too high", "0x70",
        "DC link voltage too low", "0x71",
        "DC link voltage too high", "0x72",
        "Load voltage too high", "0x73",
        "Load current ripple too high", "0x74",
        "Current_regulation_error",  "0x75",
        "Load impedance out of range", "0x76",
        "Shut off current too high", "0x77",
        "Load DC current too high", "0x78",
        "Shut off voltage too high", "0x7a",
        "Output voltage out off range", "0x80"
};


static const char psc_diamond_errmsg[][PSCIP_SIZE_MSG] = {
        "No disorders","0x0",
        "Device state error","0x1",
        "Device supervisor disabled","0x2",
        "Command abort","0x3",
        "Data not stored","0x4",
        "Error erasing flash","0x5",
        "Communication break","0x6",
        "Internal communication error","0x7",
        "Master card error","0x8",
        "Internal buffer full","0x9",
        "Wrong sector","0xa",
        "Data not copied","0xb",
        "Wrong download parameters","0xc",
        "Device parametrization error","0xd",
        "Fpga version error","0xe",
        "Timeout DC link voltage","0x10",
        "Timeout data download","0x15",
        "Interlock","0x20",
        "Temperature converter","0x25",
        "Current transducer","0x26",
        "Power semiconductor","0x28",
        "AD converter card","0x2a",
        "Auxiliary relay","0x2c",
        "Cabinet fan","0x32",
        "Mb fan alarm","0x33",
        "Subrack fan 1","0x34",
        "Subrack fan 2","0x35",
        "Subrack fan 3","0x36",
        "Multiple subrack fans","0x37",
        "Bulk DC power","0x38",
        "Inverter","0x39",
        "Control power","0x3a",
        "Phase alarm","0x3b",
        "Emergency stop alarm","0x3c",
        "Door alarm","0x3d",
        "Earth stick alarm","0x3e",
        "Earth switch alarm","0x3f",
        "Earth stick or door alarm","0x40",
        "Dump switch alarm","0x41",
        "Isolation switch alarm","0x42",
        "Isolation switch warning","0x43",
        "Earth leakage alarm","0x44",
        "Multiple isolation switch alarm","0x45",
        "Heatsink1 alarm","0x4b",
        "Heatsink2 alarm","0x4c",
        "Pss 1 alarm","0x4f",
        "Pss 2 alarm","0x50",
        "Aux 24v alarm","0x51",
        "Aux 15v alarm","0x52",
        "Mcb alarm","0x53",
        "Ssc alarm","0x54",
        "Mc alarm","0x55",
        "Sub mc alarm","0x56",
        "Charger enable","0x57",
        "Module dsp","0x58",
        "Dcct alarm","0x59",
        "Power module","0x5a",
        "Power module multiple","0x5b",
        "Door a","0x5c",
        "Ventilator","0x5d",
        "Emergency switch","0x5e",
        "Thermal switch alarm","0x5f",
        "Heatsink alarm","0x60",
        "Magnetics alarm","0x61",
        "Load current too high","0x70",
        "DC link voltage too low","0x71",
        "DC link voltage too high","0x72",
        "Load voltage too high","0x73",
        "Load current ripple too high","0x74",
        "Load impedance out of range","0x76",
        "Shut off current too high","0x77",
        "Load dc current too high","0x78",
        "BR current too high","0x79",
        "Load voltage too high 1","0x80",
        "Comms error autocheck","0xa0"
};

#define PSC_BENDING_PARAM 52
static const char psc_bending_control_param[][PSCIP_SIZE_MSG] = {
"single_quadrant",          "0x00", "i",  /* i : 1 if single quadrant (1q) ps */
"pwm_frequency",                                    "0x01",     "i",    /* i : pwm frequency in khz (!float in booster quad version only!)*/
"pwm_uv_phase",                               "0x02",   "i",    /* i : pwm phase in degrees, !!! < 350 � !!! */
"pwm_mode_fpga",                                    "0x03",     "x",    /* x : pwm_mode: 0,1; see documentation fpga  */
"min_pwm_sync",                               "0x04",   "x",    /* x : min/max phase between sync input and */
"max_pwm_sync",                               "0x05", "x",      /* x : internal sync n * 33ns */
"pwm_limiter",                                      "0x06", "x",        /* x : pwm input limiter; n * 0.4%  e.g: ...*/
"restart_latency",                          "0x10", "i",        /* i : waiting time between two device on commands (in seconds !!!) */
"ss_on_time",                                         "0x11", "i",      /* i : slow start on time (sec) */
"pwm_fb_mask",                                      "0x12", "x",        /* x : def. witch pwm feedback sig. are supervised */
"pwm_dead_time",                              "0x13", "i",      /* i : deadtime n * [33ns] */
"digital_inputs_mask",      "0x14", "x",        /* x : mask of used digital inputs*/
"digital_outputs_mask",     "0x15", "x",        /* x : mask of used digital outputs*/
"shutdown_timeout",                         "0x16", "i",        /* i : timeout value (in ms) for off command (e.g. 1500*max_i_ref/max_di_ref_dt )*/
"t_int_i_out_dc",                                   "0x19", "i",        /* i : integration time for i_dc monitoring (in ms) set to integrate about 3 sinewave periods*/
"meas_range_i_load",                "0x20", "f",        /* f : range of current measurement system; witch current equals 5v ? */
"meas_range_v",                               "0x21", "f",      /* f : range of voltage measurement system; witch voltage equals 10v ? */
"meas_range_v_load",                "0x23", "f",        /* f : range of v load master card; which current equals 10v */
"max_switch_off_current",   "0x27", "f",        /* f : max current before switching main switch off or on (e.g. meas_range_i_load/20.)*/
"delay_time_ps_on",                         "0x29", "i",        /* i : delay time during ps on sequence (in ms) */
"wait_after_slaves_on",           "0x2a", "i",  /* i : wait time after slaves are on on state (in ms) */
"max_i_load_itlk",                          "0x30", "f",        /* f : max. valid current before an interlock occurs  */
"min_i_load_itlk",                                "0x31", "f",  /* f : min. valid current before an interlock occurs  */
"max_i_load_dc_itlk",                     "0x32", "f",  /* f : max. valid dc current bevor interlock */
"max_load_resistance",                  "0x35", "f",    /* f : max. impedance before an interlock occurs */
"min_load_resistance",                  "0x36", "f",    /* f : min. impedance before an interlock occurs */
"ref_load_resistance",                  "0x37", "f",    /* f : reference load resistance */
"tolerance_load_resistance","0x38", "f",        /* f : � resistance tolerance in %*/
"max_v_load_itlk",                                "0x39", "f",  /* f : max. load voltage before an interlock occurs */
"k_i_ref",                                                    "0x3f", "f",      /* f : filter constant for reference value */
"k_filt_i",                                             "0x40", "f",    /* f : lowpass filter frequence for load current */
"min_k_p_curr",                               "0x41", "f",      /* f : kp min value for pi controller; current loop */
"max_k_p_curr",                               "0x42", "f",      /* f : kp max value for pi controller; current loop*/
"k_i_curr",                                             "0x43", "f",    /* f : ki value for pi controller; current loop*/
"k_filt",                                                       "0x44", "f",    /* f : filter constant for reference value used in waveform*/
"k_filt_r_load",                                    "0x45", "f",        /* f : filter for calculate r load */
"k_v_ref",                                                    "0x46", "f",      /* f : filter constant for dv2/dt limiter; current loop*/
"k_aw_curr",                                          "0x47", "f",      /* f : constant for anti wind-up; current loop */
"max_i_ref",                                          "0x48", "f",      /* f : max. limiter value for reference value */
"min_i_ref",                                          "0x49", "f",      /* f : min. limiter value for reference value */
"max_di_ref_dt",                                    "0x4a", "f",        /* f : max. value for di/dt limiter used in waveform*/
"min_di_ref_dt",                                    "0x4b", "f",        /* f : min. value for di/dt limiter used in waveform*/
"k_p_curr_min",                               "0x4c", "f",  /* f : min kp value */
"max_int_pi_curr",                                "0x4e", "f",  /* f : max. value for m0_1 limiter; current loop*/
"min_int_pi_curr",                                "0x4f", "f",  /* f : min. value for m0_1 limiter; current loop*/
"max_prop_pi_curr",                         "0x50", "f",        /* f : max. value for m0_2 limiter; current loop*/
"min_prop_pi_curr",                         "0x51", "f",        /* f : min. value for m0_2 limiter; current loop*/
"max_dv2_dt",                                         "0x52", "f",      /* f : max. value for dv2/dt limiter; current loop*/
"min_dv2_dt",                                         "0x53", "f",      /* f : min. value for dv2/dt limiter; current loop*/
"v_ref_range",                                      "0x54", "f",        /* f : voltage referance range*/
"test_denis_1",                               "0x79", "f",      /* f : controller test denis */
"test_denis_2",                               "0x7a", "f"         /* f : controller test denis */
};

#define PSC_QUADRUPOLE_PARAM 61
static const char psc_quadrupole_control_param[][PSCIP_SIZE_MSG] = {
"single_quadrant",                           "0x00", "i", /* i : 1 if single quadrant (1q) ps */
"pwm_frequency",                                     "0x01", "i",       /* i : pwm frequency in khz */
"pwm_uv_phase",                                "0x02", "i",     /* i : pwm phase in degrees, !!! < 350 � !!! */
"pwm_mode_fpga",                                     "0x03", "x",       /* x : pwm_mode: 0,1; see documentation fpga  */
"min_pwm_sync",                                "0x04", "x",     /* x : min/max phase between sync input and */
"max_pwm_sync",                                "0x05", "x",     /* x : internal sync n * 33ns */
"pwm_limiter",                                 "0x06", "x",     /* x : pwm input limiter; n * 0.4%  e.g: ...*/
"restart_latency",                                 "0x10", "i", /* i : waiting time between two device on commands (in seconds !!!) */
"pwm_fb_mask",                                       "0x12", "x",       /* x : def. witch pwm feedback sig. are supervised */
"pwm_dead_time",                                     "0x13", "i",       /* i : deadtime n * [33ns] */
"digital_inputs_mask",                   "0x14", "x",   /* x : mask of used digital inputs*/
"digital_outputs_mask",            "0x15", "x", /* x : mask of used digital outputs*/
"shutdown_timeout",                          "0x16", "i",       /* i : timeout value (in ms) for off command (e.g. 1500*max_i_ref/max_di_ref_dt )*/
"t_int_i_out_dc",                                    "0x19", "i",       /* i : integration time for i_dc monitoring (in ms) set to integrate about 3 sinewave periods*/
"meas_range_i_out",                          "0x20", "f",       /* f : range of current measurement system; witch current equals 5v ? */
"meas_range_v",                                "0x22", "f",     /* f :*range of voltage dc_link measurement; which current equals 5v ? */
"meas_range_v_load",                       "0x23", "f", /* f :*range of voltage measured on the load; which voltage equals 5v ? */
"max_switch_off_current",                "0x25", "f",   /* f : max current before switching main switch off (e.g. meas_range_i_out/20.)*/
"max_u_dc_itlk",                                     "0x27", "f",       /* f : max. dc link voltage before an interlock occurs */
"min_u_dc_itlk",                                     "0x28", "f", /* f : min. dc link voltage before an interlock occurs */
"u_dc_on",                                                     "0x29", "f",     /* f : min. dc link voltage for ps startup */
"max_i_out_itlk",                                    "0x2b", "f",       /* f : max. valid current before an interlock occurs  */
"min_i_out_itlk",                                    "0x2c", "f",       /* f : min. valid current before an interlock occurs  */
"max_i_out_dc_itlk",                       "0x2d", "f", /* f : max. valid dc current bevor interlock */
"max_u_out_dyn_itlk",                      "0x2e", "f", /* f : ripple supervisor (e.g. 0.000025*min_u_dc_itlk*max_dm2_dt) */
"max_load_resistance",                   "0x30", "f",   /* f : max. impedance before an interlock occurs only if i_load > 10% current measurement */
"min_load_resistance",                   "0x31", "f",   /* f : min. impedance before an interlock occurs only if i_load > 10% current measurement */
"ref_load_resistance",                   "0x32", "f",   /* f : reference load resistance */
"tolerance_load_resistance", "0x33", "f", /* f : +/- resistance tolerance in %*/
"k_filt_v_load",                                     "0x36", "f", /* f : filter constant for load voltage */
"k_i_ref",                                                     "0x37", "f",     /* f : filter constant for reference value */
"k_filt_i",                                              "0x38", "f",   /* f : lowpass filter frequency for analogue measurments [1/s] */
"k_filt_dc_v",                                       "0x39", "f",       /* f : filter constant for dc-link voltage */
"k_p_min",                                                     "0x3a", "f",     /* f : constant for amplitude dependant prop-amplification */
"k_i",                                                           "0x3b", "f",   /* f : k_i value for pi controller */
"k_filt_m",                                              "0x3c", "f",   /* f : filter constant for dm2/dt limiter */
"k_aw",                                                    "0x3d", "f", /* f : constant for anti wind-up */
"k_u_dc_hp",                                           "0x3e", "f",     /* f : filter constant for high pass filter (u_dc) */
"k_filt_r_load",                                     "0x3f", "f",       /* f : filter constant for r-load supervisor [1/s] */
"k_dudt_v_magnet",                                 "0x40", "f", /* f : for current ripple supervisor */
"max_k_p",                                                     "0x41", "f",     /* f : max kp prop  */
"min_k_p",                                                     "0x42", "f",     /* f : min kp prop  */
"max_i_ref",                                           "0x43", "f",     /* f : max. limiter value for reference value */
"min_i_ref",                                           "0x44", "f",     /* f : min. limiter value for reference value */
"max_di_ref_dt",                                     "0x46", "f",       /* f : max. value di/dt limiter for i  <= switch_di_ref_dt  [a/s] */
"min_di_ref_dt",                                     "0x47", "f",       /* f : min. value di/dt limiter for i  <= switch_di_ref_dt  [a/s] */
"max_int_pi",                                          "0x49", "f",     /* f : max. value for integral limiter */
"min_int_pi",                                          "0x4a", "f",     /* f : min. value for integral limiter */
"max_prop_pi",                                       "0x4b", "f",       /* f : max. value for proportional limiter */
"min_prop_pi",                                       "0x4c", "f",       /* f : min. value for proportional limiter */
"max_dm2_dt",                                          "0x4d", "f",     /* f : max. value for dm2/dt limiter */
"min_dm2_dt",                                          "0x4e", "f",     /* f : min. value for dm2/dt limiter */
"max_m3",                                                        "0x50", "f",   /* f : max. value for m3 limiter */
"min_m3",                                                        "0x51", "f",   /* f : min. value for m3 limiter */
"max_m",                                                         "0x52", "f",   /* f : max. value for m limiter */
"min_m",                                                         "0x53", "f",   /* f : min. value for m limiter */
"max_m0_4",                                              "0x54", "f",   /* f : max. value for ripple cancellation limiter */
"min_m0_4",                                              "0x55", "f",   /* f : min. value for ripple cancellation limiter */
"device_id_number",                          "0xf0", "i",       /* i : power supply id number */
"frame_version",                                     "0xfe", "x",       /* x : parameter block frame version */
"file_active",                                       "0xff", "x"        /* x : defines if these parameter sector is active or not (last address) */
};

#define PSC_CORRECTOR_PARAM 58
static const char psc_corrector_control_param[][PSCIP_SIZE_MSG] = {
"single_quadrant",                           "0x00", "i", /* i : 1 if single quadrant (1q) ps */
"pwm_frequency",                                     "0x01", "i",       /* i : pwm frequency in khz */
"pwm_uv_phase",                                "0x02", "i",     /* i : pwm phase in degrees, !!! < 350 � !!! */
"pwm_mode_fpga",                                     "0x03", "x",       /* x : pwm_mode: 0,1; see documentation fpga  */
"min_pwm_sync",                                "0x04", "x",     /* x : min/max phase between sync input and */
"max_pwm_sync",                                "0x05", "x",     /* x : internal sync n * 33ns */
"pwm_limiter",                                 "0x06", "x",     /* x : pwm input limiter; n * 0.4%  e.g: ...                                    */
"restart_latency",                                 "0x10", "i", /* i : waiting time between two device on commands (in seconds !!!) */
"auxiliary_relay_on_time",       "0x11", "i",   /* i : auxiliary relay on time (ms) */
"pwm_fb_mask",                                 "0x12", "x",     /* x : def. witch pwm feedback sig. are supervised */
"pwm_dead_time",                                     "0x13", "i",       /* i : deadtime n * [33ns] */
"digital_inputs_mask",                   "0x14", "x",   /* x : mask of used digital inputs*/
"digital_outputs_mask",                  "0x15", "x",   /* x : mask of used digital outputs*/
"shutdown_timeout",                          "0x16", "i",       /* i : timeout value (in ms) for off command (e.g. 1500*max_i_ref/max_di_ref_dt )*/
"fan_min_rpm",                                       "0x17", "i",       /* i : min speed of fan before reporting failure */
"max_inverter_off_time",                 "0x18", "i",   /* i : max. startup timeout for inverter (in s) */
"meas_range_i_out",                          "0x20", "f",       /* f : range of current measurement system; witch current equals 5v ? */
"max_switch_off_current",                "0x21", "f",   /* f : max current before switching main switch off (e.g. meas_range_i_out/20.)*/
"meas_range_u",                                "0x22", "f",     /* f : range of voltage measurement system; witch voltage equals 10v ? */
"max_u_dc_itlk",                                     "0x23", "f",       /* f : max. dc link voltage before an interlock occurs */
"min_u_dc_itlk",                                     "0x24", "f",       /* f : min. dc link voltage before an interlock occurs */
"u_dc_on",                                               "0x25", "f",   /* f : min. dc link voltage for ps startup */
"max_i_out_itlk",                                    "0x26", "f",       /* f : max. valid current before an interlock occurs  */
"min_i_out_itlk",                                    "0x27", "f",       /* f : min. valid current before an interlock occurs  */
"max_i_out_dc_itlk",                       "0x28", "f", /* f : max. valid dc current bevor interlock */
"max_u_out_dyn_itlk",                      "0x29", "f", /* f : ripple supervisor (e.g. 0.000025*min_u_dc_itlk*max_dm2_dt) */
"max_load_resistance",                   "0x2a", "f",   /* f : max. impedance before an interlock occurs */
"min_load_resistance",                   "0x2b", "f",   /* f : min. impedance before an interlock occurs */
"ref_load_resistance",                   "0x2c", "f",   /* f : reference load resistance */
"tolerance_load_resistance", "0x2d", "f",       /* f : � resistance tolerance in %*/
"max_u_asym_load_itlk",            "0x2e", "f", /* f : max. asymmetrical load voltage before an interlock occurs*/
"slow_start_period",                       "0x2f", "f", /* f : period (in s, 1/pwm_freq < x < 1) to freeze controller integrators at startup */
"k_lp_i",                                                  "0x30", "f", /* f : lowpass filter frequence for load current */
"k_dudt_u_magnet",                                 "0x33", "f", /* f : for current ripple supervisor */
"max_i_ref",                                     "0x40", "f",   /* f : max. limiter value for reference value */
"min_i_ref",                                     "0x41", "f",   /* f : min. limiter value for reference value */
"max_di_ref_dt",                                     "0x42", "f",       /* f : max. value for di/dt limiter */
"min_di_ref_dt",                                     "0x43", "f",       /* f : min. value for di/dt limiter */
"max_k_p",                                                     "0x46", "f",     /* f : max. k_p value for adaptive pi controller */
"min_k_p",                                                     "0x47", "f",     /* f : min. k_p value for adaptive pi controller */
"max_m0_1",                                              "0x48", "f",   /* f : max. value for m0_1 limiter */
"min_m0_1",                                              "0x49", "f",   /* f : min. value for m0_1 limiter */
"max_m0_2",                                              "0x4a", "f",   /* f : max. value for m0_2 limiter */
"min_m0_2",                                              "0x4b", "f",   /* f : min. value for m0_2 limiter */
"max_m0_4",                                              "0x4c", "f",   /* f : max. value for m0_4 limiter */
"min_m0_4",                                              "0x4d", "f",   /* f : min. value for m0_4 limiter */
"max_dm2_dt",                                          "0x4e", "f",     /* f : max. value for dm2/dt limiter */
"min_dm2_dt",                                          "0x4f", "f",     /* f : min. value for dm2/dt limiter */
"max_m3",                                                        "0x50", "f",   /* f : max. value for m3 limiter */
"min_m3",                                                        "0x51", "f",   /* f : min. value for m3 limiter */
"max_m",                                                         "0x52", "f",   /* f : max. value for m limiter */
"min_m",                                                         "0x53", "f",   /* f : min. value for m limiter */
"k_i",                                                           "0x60", "f",   /* f : k_i value for pi controller */
"k_min",                                                         "0x61", "f",   /* f : constant for amplitude dependant prop-amplification */
"k_aw",                                                    "0x62", "f", /* f : constant for anti wind-up */
"k_filt",                                                        "0x63", "f",   /* f : filter constant for reference value */
"k_filt_1",                                              "0x64", "f",   /* f : filter constant for dm2/dt limiter */
"k_hp",                                                    "0x65", "f"  /* f : filter constant for high pass filter (u_dc) */
};
