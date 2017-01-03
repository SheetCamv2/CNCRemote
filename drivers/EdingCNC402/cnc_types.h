//=================================================================
//    _____       _   _                    ____   _   _    ____
//   | ____|   __| | (_)  _ __     __ _   / ___| | \ | |  / ___|
//   |  _|    / _` | | | | '_ \   / _` | | |     |  \| | | |
//   | |___  | (_| | | | | | | | | (_| | | |___  | |\  | | |___
//   |_____|  \__,_| |_| |_| |_|  \__, |  \____| |_| \_|  \____|
//                                |___/
// ================================================================= 


#ifndef CNC_TYPES_H_INCLUDED
#define CNC_TYPES_H_INCLUDED

#include "cnc_basic_types.h"
#include "cnc_kin_types.h"

//Use a packing/alignment of 1 byte
#pragma pack(1)

typedef enum CNC_ONOFF
{
    CNC_OFF = 0,
    CNC_ON  = 1,
} CNC_ONOFF;


//shared Enums
typedef enum CNC_MACHINE_TYPE
{
	CNC_MACHINE_TYPE_MILLING = 0,
	CNC_MACHINE_TYPE_TURNING = 1,
	CNC_MACHINE_TYPE_TURNING_BACK_X = 2,
	CNC_MACHINE_TYPE_4AXES_FOAMCUTTER = 3,
    CNC_MACHINE_TYPE_PLASMACUTTER = 4,
    CNC_MACHINE_TYPE_3D_PRINTING = 5,
    CNC_MACHINE_TYPE_LASER = 6
} CNC_MACHINE_TYPE;


typedef enum CNC_ROT_AXIS_OPTION
{
	CNC_ROT_OPT_ROT = 0,
	CNC_ROT_OPT_SLAVE_TO_X = 1,
	CNC_ROT_OPT_SLAVE_TO_Y = 2,
	CNC_ROT_OPT_SLAVE_TO_Z = 3,
	CNC_ROT_OPT_FOAMCUT = 4,
	CNC_ROT_OPT_4AX_MILLING = 5,
	CNC_ROT_OPT_2ND_Z = 6,
	CNC_ROT_OPT_TAN_KNIFE = 7,
	CNC_ROT_OPT_LAST
} CNC_ROT_AXIS_OPTION;




typedef enum CNC_IO_ID
{
    CNC_IOID_NONE = 0,             //No IO defined
    //1 dummy io

    CNC_IOID_MACHINE_ON_OUT=1,     //Machine ON output
    CNC_IOID_DRIVE_ENABLE_OUT=2,   //Amplifier enable
    CNC_IOID_TOOL_OUT=3,           //Tool on
    CNC_IOID_COOLANT1_OUT=4,       //Flood
    CNC_IOID_COOLANT2_OUT=5,       //Mist
	CNC_IOID_TOOLDIR_OUT=6,        //Tool direction output
	//6 total standard CNC outputs

    CNC_IOID_AUX1_OUT=7,           //Aux1 output
	CNC_IOID_AUX2_OUT=8,           //Aux2 output
	CNC_IOID_AUX3_OUT=9,           //Aux3 output
	CNC_IOID_AUX4_OUT=10,          //Aux4 output
	CNC_IOID_AUX5_OUT=11,          //Aux5 output
	CNC_IOID_AUX6_OUT=12,          //Aux6 output
	CNC_IOID_AUX7_OUT=13,          //Aux7 output
	CNC_IOID_AUX8_OUT=14,          //Aux8 output
	CNC_IOID_AUX9_OUT=15,          //Aux9 output
    //9 AUX outputs
    //15  total digital CPU outputs

	CNC_IOID_PWM_VAL1_OUT=16,      //PWM value output 0..1000%%
	CNC_IOID_PWM_VAL2_OUT=17,      //PWM value output 0..1000%%
	CNC_IOID_PWM_VAL3_OUT=18,      //PWM value output 0..1000%%
    //3 total analog CPU outputs
    
    CNC_IOID_EMSTOP1_IN=19,         //Emergency stop input
    CNC_IOID_EMSTOP2_IN=20,         //Emergency stop input
    CNC_IOID_EXTERR_IN=21,          //Ext error input
	CNC_IOID_PROBE_IN=22,           //Probe
	CNC_IOID_SYNC_IN=23,            //Spindle Sync pulse
	CNC_IOID_RUN_IN=24,             //Run Button
	CNC_IOID_PAUSE_IN=25,           //Pause Button
	CNC_IOID_HOME_X_IN=26,          //Home sensor
	CNC_IOID_HOME_Y_IN=27,          //idem
	CNC_IOID_HOME_Z_IN=28,          //idem
	CNC_IOID_HOME_A_IN=29,          //idem
	CNC_IOID_HOME_B_IN=30,          //idem
	CNC_IOID_HOME_C_IN=31,          //idem
	CNC_IOID_HOME_1_IN=32,          //Home sensor
	CNC_IOID_HOME_2_IN=33,          //idem
	CNC_IOID_HOME_3_IN=34,          //idem
	CNC_IOID_HOME_4_IN=35,          //idem
	CNC_IOID_HOME_5_IN=36,          //idem
	CNC_IOID_HOME_6_IN=37,          //idem
    CNC_IOID_HW1A_IN=38,            //Hand wheel counter
    CNC_IOID_HW1B_IN=39,            //Hand wheel counter

    CNC_IOID_EMSTOP_FROM_GPIO=40,   //EStop from GPIO
    CNC_IOID_SSTOP_FROM_GPIO=41,    //SmoothStop from GPIO
    CNC_IOID_WARNING_FROM_GPIO=42,  //Warning from GPIO

    //21 total CPU digital CNC inputs

    CNC_IOID_AUX1_IN=43,            //Aux1 input
	CNC_IOID_AUX2_IN=44,            //Aux2 input
	CNC_IOID_AUX3_IN=45,            //Aux3 input
	CNC_IOID_AUX4_IN=46,            //Aux4 input
	CNC_IOID_AUX5_IN=47,            //Aux5 input
	CNC_IOID_AUX6_IN=48,            //Aux6 input
    CNC_IOID_AUX7_IN=49,            //Aux6 input
    CNC_IOID_AUX8_IN=50,            //Aux6 input
    //6 total CPU digital AUX inputs
    //27 total CPU digital inputs

    CNC_IOID_ANA1_IN=51,            //Analog1
    CNC_IOID_ANA2_IN=52,            //Analog2
    CNC_IOID_ANA3_IN=53,            //Analog3
    CNC_IOID_ANA4_IN=54,            //Analog3
    CNC_IOID_ANA5_IN=55,            //Analog3

    //3 total CPU analog inputs
    CNC_IOID_DRIVEALM_IN=56,        //Drive alarm input
    CNC_IOID_DRIVEWARN_IN=57,       //Drive warning input
    CNC_IOID_OUTPUTPROBLEM_IN=58,      //Drive alarm input
    CNC_IOID_INPUTPROBLEM_IN=59,      //Drive warning input

    CNC_IOID_LAST=60                //Serves as last

} CNC_IO_ID;


typedef enum CNC_GPIO_ID
{
    CNC_GPIOID_NONE = 0,             //No IO defined
    //1 dummy gpio

    //General purpose IO
    CNC_GPIOID_OUT_1=1,          //Ext io card output
    CNC_GPIOID_OUT_2=2,          //Ext io card output
    CNC_GPIOID_OUT_3=3,          //Ext io card output
    CNC_GPIOID_OUT_4=4,          //Ext io card output
    CNC_GPIOID_OUT_5=5,          //Ext io card output
    CNC_GPIOID_OUT_6=6,          //Ext io card output
    CNC_GPIOID_OUT_7=7,          //Ext io card output
    CNC_GPIOID_OUT_8=8,          //Ext io card output
    CNC_GPIOID_OUT_9=9,          //Ext io card output
    CNC_GPIOID_OUT_10=10,         //Ext io card output

    CNC_GPIOID_ANOUT_1=11,       //Ext io card output
    CNC_GPIOID_ANOUT_2=12,       //Ext io card output
    CNC_GPIOID_ANOUT_3=13,       //Ext io card output
    CNC_GPIOID_ANOUT_4=14,       //Ext io card output

    CNC_GPIOID_IN_1=15,           //Ext io card input
    CNC_GPIOID_IN_2=16,           //Ext io card input
    CNC_GPIOID_IN_3=17,           //Ext io card input
    CNC_GPIOID_IN_4=18,           //Ext io card input
    CNC_GPIOID_IN_5=19,           //Ext io card input
    CNC_GPIOID_IN_6=20,           //Ext io card input
    CNC_GPIOID_IN_7=21,           //Ext io card input
    CNC_GPIOID_IN_8=22,           //Ext io card input
    CNC_GPIOID_IN_9=23,           //Ext io card input
    CNC_GPIOID_IN_10=24,          //Ext io card input

    CNC_GPIOID_ANIN_1=25,         //Ext io card analogue input
    CNC_GPIOID_ANIN_2=26,         //Ext io card analogue input
    CNC_GPIOID_ANIN_3=27,         //Ext io card analogue input
    CNC_GPIOID_ANIN_4=28,         //Ext io card analogue input

    CNC_GPIOID_RULE_TRIGGER_STATUS = 29, //Ext io card trigger status, bit 0 .. 15

    CNC_GPIO_ID_WARNING_OUT = 30,     //These are outputs on the CPU5B OPT connector
    CNC_GPIO_ID_ESTOP_OUT   = 31,     //Output for the GPIO
    CNC_GPIO_ID_SSTOP_OUT   = 32,     //Inputs for the CPU

    CNC_GPIOID_LAST=33           //Serves as last

} CNC_GPIO_ID;



typedef enum CNC_IE_STATE
{
    CNC_IE_POWERUP_STATE        = 0, /* no interpreter threads running yet */
    CNC_IE_IDLE_STATE,               /* thread is started but IE not initialized */
    CNC_IE_READY_STATE,              /* ready to load/run */
	CNC_IE_EXEC_ERROR_STATE,         /* Execution error */
	CNC_IE_INT_ERROR_STATE,          /* interpreter error */
	CNC_IE_ABORTED_STATE,            /* job was aborted */

	/* Running states from which Pause is possible */
	CNC_IE_RUNNING_JOB_STATE,        /* Job running */
	CNC_IE_RUNNING_LINE_STATE,       /* single line running */
	CNC_IE_RUNNING_SUB_STATE,        /* single subroutine running */
	CNC_IE_RUNNING_SUB_SEARCH_STATE, /* single subroutine running from search */
	CNC_IE_RUNNING_LINE_SEARCH_STATE,/* single line running from search*/
		
	/* the belonging paused states */
	CNC_IE_PAUSED_LINE_STATE,        /* single line paused, by pause command */
	CNC_IE_PAUSED_JOB_STATE,         /* paused running job, by pause command */
	CNC_IE_PAUSED_SUB_STATE,         /* paused running sub , by pause command */
	CNC_IE_PAUSED_LINE_SEARCH_STATE, /* paused running line search line running from search*/
	CNC_IE_PAUSED_SUB_SEARCH_STATE,  /* paused running sub search subroutine running from search */

	/* special Running states, no pause possible */
	CNC_IE_RUNNING_HANDWHEEL_STATE,  /* hand wheel operation */
	CNC_IE_RUNNING_LINE_HANDWHEEL_STATE, /* single line running from hand wheel mode, can only be G92... */
	CNC_IE_RUNNING_LINE_PAUSED_STATE,/* single line running from hand wheel mode, can only be G92... */
	CNC_IE_RUNNING_AXISJOG_STATE,    /* running joint jog */
	CNC_IE_RUNNING_IPJOG_STATE,      /* running joint jog */
	
	/* Rendering and searching states */
	CNC_IE_RENDERING_GRAPH_STATE,    /* running interpreter for graph view only */
	CNC_IE_SEARCHING_STATE,          /* searching line */
	CNC_IE_SEARCHED_DONE_STATE,      /* searched line found */

    CNC_IE_LAST_STATE                /* keep last */
} CNC_IE_STATE;


typedef enum CNC_JOINT_STATE
{
    CNC_JOINT_POWER_UP            = 0,
    CNC_JOINT_MOVING_STATE        = 1,
    CNC_JOINT_READY_STATE         = 2,
    CNC_JOINT_READY_STOPPED_STATE = 3,
    CNC_JOINT_FREE_STATE          = 4,
    CNC_JOINT_ERROR_STATE         = 5, 
    CNC_JOINT_LAST_STATE          = 6
} CNC_JOINT_STATE;

typedef enum CNC_LANG_T
{
	CNC_LANG_ENGLISH  = 0,
	CNC_LANG_GERMAN   = 1,
	CNC_LANG_DUTCH    = 2,
	CNC_LANG_ITALIAN  = 3,
	CNC_LANG_FRENCH   = 4,
	CNC_LANG_SPANISH  = 5,
	CNC_LANG_PORTUGESE = 6,
	CNC_LANG_TURKISH   = 7,
	CNC_LANG_JAPANESE  = 8,
	CNC_LANG_GRC       = 9,
	CNC_LANG_HUN       = 10,
	CNC_LANG_CZE       = 11,
	CNC_LANG_HKR       = 12,
	CNC_LANG_CN        = 13,
	CNC_LANG_NEW1      = 14,
	CNC_LANG_LAST      = 15,		//Keep last

} CNC_LANG_T;

//Move types used for graphics in GUI
typedef enum CNC_MOVE_TYPE
{
	CNC_MOVE_TYPE_UNKNOWN = 0,
    CNC_MOVE_TYPE_G0  = 1,
    CNC_MOVE_TYPE_G1  = 2,
    CNC_MOVE_TYPE_ARC = 3,
	CNC_MOVE_TYPE_PROBE = 4,
    CNC_MOVE_TYPE_JOG = 5,
	CNC_MOVE_TYPE_HOME = 6,
    CNC_MOVE_TYPE_ORIGIN_OFFSET = 7,
	CNC_MOVE_TYPE_START_POSITION = 8,
	CNC_MOVE_TYPE_SET_GRAPH_START_POINT = 9,
	CNC_MOVE_TYPE_OUT_LINE = 10,
    CNC_MOVE_TYPE_END = 11,
	CNC_MOVE_TYPE_END_COLLISION = 12
} CNC_MOVE_TYPE;

typedef enum CNC_TRAFFIC_LIGHT_COLOR
{
	CNC_TRAFFIC_LIGHT_COLOR_OFF = 0,
	CNC_TRAFFIC_LIGHT_COLOR_GREEN = 1,
	CNC_TRAFFIC_LIGHT_COLOR_YELLOW = 2,
	CNC_TRAFFIC_LIGHT_COLOR_RED = 3,
} CNC_TRAFFIC_LIGHT_COLOR;


typedef enum CNC_TRACKING_MODE
{
	CNC_TRACKMODE_OFF = 0,
	CNC_TRACKMODE_HANDWHEEL_POS,
	CNC_TRACKMODE_HANDWHEEL_VEL,
	CNC_TRACKMODE_POSITION,
	CNC_TRACKMODE_VELOCITY
} CNC_TRACKING_MODE;

typedef enum CNC_TRACK_SOURCE
{
	CNC_TRACKSOURCE_NONE = 0,
	CNC_TRACKSOURCE_INTERNAL_HANDWHEEL1,
	CNC_TRACKSOURCE_INTERNAL_HANDWHEEL2,
    CNC_TRACKSOURCE_INTERNAL_AUXANALOGIN1,
    CNC_TRACKSOURCE_INTERNAL_AUXANALOGIN2,
    CNC_TRACKSOURCE_INTERNAL_AUXANALOGIN3,
    CNC_TRACKSOURCE_INTERNAL_AUXANALOGIN4,
    CNC_TRACKSOURCE_INTERNAL_AUXANALOGIN5,
    CNC_TRACKSOURCE_THC_ANALOGIN,
	CNC_TRACKSOURCE_TRACK_DATA_POSITION,
	CNC_TRACKSOURCE_TRACK_DATA_VELOCITY,
	CNC_TRACKSOURCE_TRACK_DATA_HANDWHEEL1,
	CNC_TRACKSOURCE_TRACK_DATA_HANDWHEEL2,
	CNC_TRACKSOURCE_TRACK_DATA_HANDWHEEL3

} CNC_TRACK_SOURCE;

typedef enum CNC_FEEDSPEED_OVERRIDE_SOURCE
{
    CNC_FSOV_SOURCE_UI = 0,
    CNC_FSOV_SOURCE_UI_HANDWHEEL,
    CNC_FSOV_SOURCE_ANALOGIN1,
    CNC_FSOV_SOURCE_ANALOGIN2,
    CNC_FSOV_SOURCE_ANALOGIN3,
    CNC_FSOV_SOURCE_ANALOGIN4,
    CNC_FSOV_SOURCE_ANALOGIN5

} CNC_FEEDSPEED_OVERRIDE_SOURCE;

typedef enum CNC_GPIO_ACTION_TYPE
{
	CNC_GPIO_ACTION_TYPE_NONE = 0,
	CNC_GPIO_ACTION_TYPE_WARNING = 1,
	CNC_GPIO_ACTION_TYPE_SSTOP = 2,
	CNC_GPIO_ACTION_TYPE_ESTOP = 3

} CNC_GPIO_ACTION_TYPE;

typedef enum CNC_IO_GUARD_STOP_ACTION_TYPE
{
    CNC_IO_GUARD_STOP_ACTION_NONE = 0,
    CNC_IO_GUARD_STOP_ACTION_WARNING = 1,
    CNC_IO_GUARD_STOP_ACTION_SLOW_FEED = 2,
    CNC_IO_GUARD_STOP_ACTION_STOP = 3,
    CNC_IO_GUARD_STOP_ACTION_ESTOP = 4

} CNC_IO_GUARD_STOP_ACTION_TYPE;


/* CNC_CMD_ARRAY_PARAMETERS */
typedef struct 
{
    //A job may be executed multiple
	int			 doArray;				//1 if nesting switched on
	double		 arrayStartOffsetX;     //All objects are offset by this
	double		 arrayStartOffsetY;     //All objects are offset by this
	double		 arrayDX;				//Shift (mm) for X
	double		 arrayDY;				//Shift (mm) for y
	int          arrayNX;				//Number in x
	int          arrayNY;				//Number in y

	double       materialSizeX;         //Only used with GetJobArrayParameters
	double       materialSizeY;         //Only used with GetJobArrayParameters
	double       materialSizeZ;         //Only used with GetJobArrayParameters

} CNC_CMD_ARRAY_DATA;


/* Fiducials for vision, fiducial number is 0..9 */
typedef struct 
{
	int		fidn;   //%fidn=... number, -1 if not valid
	int		fidt;	//%fidt=... type
	double	fidcx;	//%fidcx=.. coordinate x
	double	fidcy;	//%fidcy=.. coordinate y
	double	fidox;  //%fidox=.. offset x
	double	fidoy;  //%fidoy=.. offset y
	double	fidor;	//%fidor=.. orientation

} CNC_FIDUCIAL_DATA;


/* CNC_CMD_RUN_JOB, Pro log, for starting after search to line */
typedef struct 
{
	//Before execution a pro log may be executed
	int			 prologEnable;				//0=no prolog, 1=prolog
	int			 prologChangeTool;			//Perform change tool with given tool before start of job
	CNC_CART_DOUBLE prologMoveToPlanePosition; //Perform XY positioning with Z at safe height (G17)
	int			 prologFlood;               //1=flood on, 0-flood off, -1 leave as is
	int          prologMist;                //Idem for mist
	int          prologSpindle;             //3 turn right, 4 turn left
	double       prologSpeed;				//Spindle speed
	int			 prologPlungeFeed;          //Feed for Z down (G17)
	double       prologPlungePos;           //Z down position
	double       prologFeed;				//Feed after plunge.
} CNC_CMD_JOB_PROLOG_DATA;


struct CNC_LOG_MESSAGE
{
    /* code where the error or what kind of message did occur, see CNC_RC */
    CNC_RC code;          

    /* the error class */
    CNC_ERROR_CLASS errorClass;

    /* subcode is only relevant when code specifies a subcode */
    int subCode;

    /* textual error or dialog description */
    char text[CNC_MAX_LOGGING_TEXT];

    /* parameter names */
    char par1Name[CNC_MAX_NAME_LENGTH];
    char par2Name[CNC_MAX_NAME_LENGTH];
    char par3Name[CNC_MAX_NAME_LENGTH];
    char par4Name[CNC_MAX_NAME_LENGTH];
    char par5Name[CNC_MAX_NAME_LENGTH];
    char par6Name[CNC_MAX_NAME_LENGTH];
	char par7Name[CNC_MAX_NAME_LENGTH];
	char par8Name[CNC_MAX_NAME_LENGTH];
	char par9Name[CNC_MAX_NAME_LENGTH];
	char par10Name[CNC_MAX_NAME_LENGTH];
	char par11Name[CNC_MAX_NAME_LENGTH];
    char par12Name[CNC_MAX_NAME_LENGTH];
    char par13Name[CNC_MAX_NAME_LENGTH];
    char par14Name[CNC_MAX_NAME_LENGTH];
    char par15Name[CNC_MAX_NAME_LENGTH];

    /* parameter numbers to set, 1..MAX_VARS -1 */
    int par1Number;
    int par2Number;
    int par3Number;
    int par4Number;
    int par5Number;
	int par6Number;
	int par7Number;
	int par8Number;
	int par9Number;
	int par10Number;
	int par11Number;
    int par12Number;
    int par13Number;
    int par14Number;
    int par15Number;


    /* textual description of c-source and line number,
     * this extra is debug information to see where the error
     * occurred in the server.
     */
    char sourceInfo[CNC_MAX_SOURCE_INFO_TEXT];
    char functionName[CNC_MAX_FUNCTION_NAME_TEXT];

	time_t timeStamp;

    /* number */
    int n; 

    /* for internal use only */
    int hint;
    
};



/*
  __joint_configuration_type_definitions_) 
*/
struct CNC_JOINT_CFG
{
    /* logical name of the joint, used in GUI, one character */
    char name;
	int isVisible;      //True if axis visible in GUI
    int cpuPortIndex;   //0-5 for 6 axes board, -1 if not used

    /* Axis resolution nuber of increments for one application unit */
    double resolution;
    double positiveLimit;
    double negativeLimit;

    /* max values for velocity, acceleration, AU  */
    double maxVelocity;
    double maxAcceleration;

    /* homing parameters, AU  */
	double homeVelocity;     //Sign is direction
	double homeVelocitySlow; //Slow vel for 2nd move
    double homePosition;     //Position at home sensor

	/* backlash parameters */
	double backLash;

	/* jog speed percentages */
	double lowSpeedJogPercent;
	double medSpeedJogPercent;
	double highSpeedJogPercent;

    /* pitch compensation */
    char pitchCompensationFileName[CNC_MAX_PATH];
    int  pitchCompensationOn;

};

/* kinematics configuration, all related to XYZABC axes */
typedef struct _CNC_KIN_CFG
{
    CNC_CART_BOOL axesInUse;	    //True if axis in use
    CNC_CART_BOOL axesVisible;	    //True if visible (implies inUse)
	CNC_JOINT_BOOL jointInUse;	//0-5 for 6 axes board. -1 if not connected
    CNC_JOINT_BOOL jointVisible;//True if visible
	CNC_JOINT_BOOL jointIsSlave;//True if a joint is slave
    CNC_ROT_AXIS_OPTION aAxisOption;
    CNC_ROT_AXIS_OPTION bAxisOption;
    CNC_ROT_AXIS_OPTION cAxisOption;
	/* for run time loading of user kins */
	char kinematicsDLLName[CNC_MAX_PATH];

    /* max values for Cartesian velocity, acceleration, AU  */
    CNC_CART_DOUBLE maxCartesianVelocity;
    CNC_CART_DOUBLE maxCartesianAcceleration;
    CNC_CART_DOUBLE positiveCartesianLimit;
    CNC_CART_DOUBLE negativeCartesianLimit;

    /* max values for joint velocity, acceleration, AU  */
    CNC_JOINT_DOUBLE maxJointVelocity;
    CNC_JOINT_DOUBLE maxJointAcceleration;
    CNC_JOINT_DOUBLE positiveJointLimit;
    CNC_JOINT_DOUBLE negativeJointLimit;


	/* allowed tool length with z completely down */
	double zDownToolLength;

	/* prohibited area for tool changer */
	CNC_CART_DOUBLE positiveLimitTCA;
	CNC_CART_DOUBLE negativeLimitTCA;
	
	/* disable guard in Z if 1, enable if 0 */
    int disableZToolCollisionGuard;

    /* 4th axis milling parameters */
    /* 0 = not calibrated, work position 0 is used as rotation point */
    int  aAxisRotationPointIsCalibrated;
    CNC_VECTOR aAxisCalibratedRotationPoint;

    /* the calibrated radius for milling  on the cylinder*/
    int     aAxisCylinderRadiusIsCalibrated;
    double  aAxisCalibratedCilinderRadius;

    /* linear delta robot parameters */
    double linDeltaArmLength;
    double linDeltaRadius;
    double linDeltaSafetyRadius;

    /* machine type, copy from system config */
    CNC_MACHINE_TYPE  machineType;

} CNC_KIN_CFG;


typedef struct _CNC_TAN_KNIFE_STATUS
{
    /* 1 if tangential knife mode is active, C-Axis follows X,Y direction */
    int                  tanKnifeIsOn;

    /* -1 for bend angle negative from rotation angle, +1 positive, 0 no bending so normal tan knife */
    int                  tanknifeBending;

    /* 0: no special mapping, 1: Mapped to B, 2: Mapped to BC, dual tan knife */
    int                  tanKnifeBCMapping;
    double               tanknifeZloMachine;                
    double               tanknifeZhiMachine;  

} CNC_TAN_KNIFE_STATUS;


/* kinematics status */
typedef struct _CNC_KIN_STATUS
{
   	CNC_VECTOR           m_activeToolOffset; //Always active tool value, not related to G43/G49
	CNC_KINEMATICS_TYPE  m_kinType;           
    char                 m_version[CNC_MAX_NAME_LENGTH]; //Read from DLL
	int                  m_kinActive;   //1 of active
    CNC_OFFSET_AND_PLANE m_offsetAndPlane;
    CNC_KIN_CFG          m_actConfig;
    CNC_TAN_KNIFE_STATUS m_tanknifeSts;
    int                  m_mcaActive;
    int                  m_tcaActive;   

} CNC_KIN_STATUS;

struct CNC_CAMERA_CONFIG
{
	int    cameraOn;	     //o=off, 1=on
	int    cameraIndex;      //0 is first, 1 is second etc.
    int    cameraFlip;
    int    cameraMirror;
    int    cameraRotationAngle;
};


//Note that the tool length/diameter
//Are stored into user variables 
//Index of variable given by lenghtIndex and
//diameter index
struct CNC_TOOL_DATA_INTERNAL
{
    int     id;
    char    description[80];
    int     diameterIndex;	    //Variable index 5400 .. 5499
    int     zOffsetIndex;	    //Variable index 5500 .. 5599
    int     xOffsetIndex;	    //Variable index 5600 .. 5699
    int     orientationIndex;   //Variable index 5700 .. 5799
    int     xDeltaIndex;         //Variable index 5800 .. 5899
    int     zDeltaIndex;         //Variable index 5900 .. 5999
    int     locationCode;       
};

//Note that the tool length/diameter
//Are stored into user variables 
//Index of variable given by lenghtIndex and
//diameter index
struct CNC_TOOL_DATA
{
    int     id;
    int     locationCode;
    char    description[80];
    double  diameter;	//Variable index 5400 .. 5499
    double  zOffset;    //Variable index 5500 .. 5599  (Length)
    double  xOffset;    //Variable index 5600 .. 5699  (width, for turning)
    double  zDelta;      //Variable index 5900 .. 5999
    double  xDelta;      //Variable index 5800 .. 5899
    int     orientation;
};

struct CNC_IO_CONFIG
{
    int invertAmpEnableOut;
    int invertToolOut;	
    int invertToolDirOut;

    int invertFloodOut;	 
    int invertMistOut;	
    int invertAux1Out;	
    int invertAux2Out;	
    int invertAux3Out;	
    int invertAux4Out;	
    int invertAux5Out;	
    int invertAux6Out;	
    int invertAux7Out;	
    int invertAux8Out;	
    int invertAux9Out;	
    int invertPwm1Out;	
    int invertPwm2Out;	
    int invertPwm3Out;	
    int invertAux1In;	
    int invertAux2In;	
    int invertAux3In;	
    int invertAux4In;	
    int invertAux5In;	
    int invertAux6In;	
    int invertAux7In;	
    int invertAux8In;	

    int invertPauseIn; 
    int invertDriveErrIn;
    int invertDriveWarnIn;

    char nameExtErrIn[CNC_MAX_IO_NAME_LENGTH];
    char nameFloodOut[CNC_MAX_IO_NAME_LENGTH];	 
    char nameMistOut[CNC_MAX_IO_NAME_LENGTH];	
    char nameAux1Out[CNC_MAX_IO_NAME_LENGTH];	
    char nameAux2Out[CNC_MAX_IO_NAME_LENGTH];	
    char nameAux3Out[CNC_MAX_IO_NAME_LENGTH];	
    char nameAux4Out[CNC_MAX_IO_NAME_LENGTH];	
    char nameAux5Out[CNC_MAX_IO_NAME_LENGTH];	
    char nameAux6Out[CNC_MAX_IO_NAME_LENGTH];	
    char nameAux7Out[CNC_MAX_IO_NAME_LENGTH];	
    char nameAux8Out[CNC_MAX_IO_NAME_LENGTH];	
    char nameAux9Out[CNC_MAX_IO_NAME_LENGTH];	
    char namePwm1Out[CNC_MAX_IO_NAME_LENGTH];	
    char namePwm2Out[CNC_MAX_IO_NAME_LENGTH];	
    char namePwm3Out[CNC_MAX_IO_NAME_LENGTH];	
    char nameAux1In[CNC_MAX_IO_NAME_LENGTH];	
    char nameAux2In[CNC_MAX_IO_NAME_LENGTH];	
    char nameAux3In[CNC_MAX_IO_NAME_LENGTH];	
    char nameAux4In[CNC_MAX_IO_NAME_LENGTH];	
    char nameAux5In[CNC_MAX_IO_NAME_LENGTH];	
    char nameAux6In[CNC_MAX_IO_NAME_LENGTH];	
    char nameAux7In[CNC_MAX_IO_NAME_LENGTH];	
    char nameAux8In[CNC_MAX_IO_NAME_LENGTH];	

};


struct CNC_SPINDLE_CONFIG
{
    int spindleIndex;
    
    /* time to wait after spindle on, before moving */
	double rampUpTime;

	/* if 1 ramp uptime is proportional with speed */
	/* int proportionalRampUpTime; */

	/* max/min spindle revolutions/sec five 100% PWM output */
	double NmaxRPM;
	double NminRPM;

    /* for step/pulse spindle, 0 for normal PWM spindle */
    int countPerRev;
    int stepperMotorMode;

    /* smooth count mode uses 32 bit counter i.s.o 16 bit, this however makes PWM2/3 no longer as separate PWM usable on cpu5b */
    int smoothCountMode; 

	/* if true, use sensor (if available) to measure spindle speed */
	int useRPMSensor;

    /* IO's to be used for spindle */
    CNC_IO_ID onOffOutputPortID;
    CNC_IO_ID directionOutputPortID;
    CNC_IO_ID pwmOutputPortID;
    CNC_IO_ID spindleReadyPortID;

    /* id this is set our direction and onoff port change into rightOnOff and leftOnOff port */
    int rightOnLeftOnMNode;

    /* offset for this spindle */
    CNC_VECTOR coordinateSystemOffset;

    /* spindle PWM correction table */
    char pwmCompensationFileName[CNC_MAX_PATH];
    int  pwmCompensationOn;

    /* correction of speed measured by sensor */
    int sensorSpeedControlOn;
    double sensorSpeedControlCycleTime;
};

struct CNC_FEEDSPEED_CFG
{
    CNC_FEEDSPEED_OVERRIDE_SOURCE feedOverrideSource;
    CNC_FEEDSPEED_OVERRIDE_SOURCE speedOverrideSource;
    CNC_IO_ID feedHoldInputPortID;
    CNC_IO_ID speedHoldInputPortID;
};


struct CNC_HANDWHEEL_CFG
{
    /* hand wheel encoder count for one revolution */
    int handwheelCountPerRev;
    /* vel limit for hand wheel percentage of selected axis max vel */
    int handweelVPercent;
    /* acc limit for hand wheel percentage of selected axis max acc */
    int handwheelAPercent;

    /* velocity mode for 1x scale */
    int handwheelX1VelMode;
    /* velocity mode for 10x scale */
    int handwheelX10VelMode;
    /* velocity mode for 100x scale */
    int handwheelX100VelMode;

    /* use analog input to select hand wheel axis */
    CNC_IO_ID axisSelectAnInputPortID;

    /* use analog input to select hand wheel multiplication factor */
    CNC_IO_ID mulfactorSelectAnInputPortID;

};

struct CNC_TRAFFIC_LIGHT_CFG
{
	/* output numbers for traffic light */
	int trafficLightGreenIOID;
	int trafficLightYellowIOID;
	int trafficLightRedIOID;
};


struct CNC_PROBING_CFG
{
	/* 1 if height probe is present, 0 if not */
	int heightProbePresent;
	/* if 1, probe touch points are stored in digitize file name */
	int storeProbeTouchPoints;
	/* file name to store probe touch points */
    char probeTouchPointFileName[CNC_MAX_PATH];
	/* beep if probe triggers */
	int probeBeep;
    /* probe input */
    int probeUseHomeInput4;
    /* analog probing using analog input 3 of CPU */
    double probeVoltIncToMM;
    /* protect touch probe, detect unexpected triggers */
    int probeDetectUnexpectedTrigger;
 };


struct CNC_SAFETY_CONFIG
{
    /* switch on e stop function on home sensors if all axes are homed */
    int homeIsEstopAfterHomingAllAxes;

    /* emergency stop sense level */
	int EStopInputSenseLevel1;

    /* only cpu5b */
	int EStopInputSenseLevel2;

    /* drive warning input sense level for i600 */
    int driveWarningInputSenseLevel;
    int driveErrorInputSenseLevel;
    int isoInputSenseLevel;
    int isoOutputSenseLevel;

    /* input sense level, 0=low-e-stop, 1=hi-e-stop, 2=none, 3=low-stop, 4=hi-stop, 5=low-warning, 6=hi-warning*/
    int extErrorInputSenseLevel;

    /* aux inputs guarding */
    /* input sense level, 0=low-e-stop, 1=hi-e-stop, 2=none, 3=low-stop, 4=hi-stop, 5=low-slowfeed, 6=hi-slowfeed, 7=low warning, 8 = hi-warning*/
    int auxInputCheckSenseLevel[CNC_MAX_AUX_GUARD_INPUTS];

    /* switch on/off gpio warnings/sstop/estop */
    int enableGPIOActions;

    /* max feed in safety mode */
    double safetyFeed;

    /* max distance between master-slave axis after both are homed */
    double maxMasterSlaveDistance;

    /* use end of stroke switch as emergency stop after homing */
    int useXHomeinputForAllAxes;

    /* end of stroke sense level, used for homing */
    int endOfStrokeInputSenseLevel;


    /* Homing mandatory or not, 1=mandatory */
    int mandatoryHoming;

    /* Id 1 stop spindle on pause */
    int stopSpindleOnPause;
    int aux1_OffOnPause;
    int aux2_OffOnPause;
    int aux3_OffOnPause;
    int aux4_OffOnPause;
    int aux5_OffOnPause;
    int aux6_OffOnPause;
    int aux7_OffOnPause;
    int aux8_OffOnPause;
    int aux9_OffOnPause;
    int aux10_OffOnPause;

    int mist_OffOnPause;
    int flood_OffOnPause;


    /* start spindle when resumed from pause */
    int autoStartAfterPause;

    /* move Z up on pause */
    int zUpOnPause;

    /* Z up distance */
    double zUpMoveDistanceOnPause;

    /* feed during approach of Z at resume */
    double approachFeed;

    /* safety relay present */
    int safetyRelayPresent;
    CNC_IO_ID systemReadyOutPortID;
    CNC_IO_ID safetyRelayResetOutPortID;

    /* pulse length of reset pulse for safety relay */
    int safetyRelayResetDelayMs;
    int safetyRelayPulseLengthMs; 

};


/* THC, Torch height control
   start condition machine is at safe height above piercing X,Y point.

   1. Machine performs G38.2 to zero Z on top surface.
   2. Machine moves to Ignition height
   3. Ignition (M3)?
   4. Machine moves to pierce height.
   5. Machine waits pierce time
   6. Machine moves to cutting height.
   7. Machine starts moving, after controller delay THC is activated.
   8. THC controls Z height
   9. When speed lower than anti dive speed, THC is temporary switched off.

  10. At the end, Z is moved to safe height
*/



struct CNC_THC_PROCESS_PARAMETERS
{
    /* set point voltage */
    double setPointVoltage ;     

    /* control delay, delay after piercing and starting machine move till switch on THC */
    double controlDelay;

    /* proportional gain, typical 0.1 mm/volt */
    double KPUp;                   
    double KPDown;                   
	double deadBand;                   
	double KD;                
   	double filterTime;
	double holeDetectVoltage;
	double holeDetectTime;

    /* off when feed below this value */
    double cornerFeedFactor;         


    /* THC will not raise/lower Z more than these limits */
    double zMax;              
    double zMin;  

    /* 1 to switch THC ON, 0 to switch OFF */
    int thcON;
	int measuredIsSetpoint;

	/* tuning, 1 is on, 2 = off */
	/* voltage 1 and 2 can be applied as if from external measurement */
	int tuningMode; 
};

struct CNC_THC_CFG
{

    /* default process parameters for THC */
    CNC_THC_PROCESS_PARAMETERS processPars;

    /* ADC 10 bit conversion to THC voltage		  */
	/* Volt = adcMulFactor * adcValue + adcOffset */
    double adcMulFactor;   
	double adcOffset;

    /* plasma ON input port, 0 if not used */
	CNC_IO_ID plasmaIsOnInputPortID;
	CNC_IO_ID plasmaIsCornerOutputPortID;
    CNC_IO_ID plasmaAnalogInputPortID;
	CNC_IO_ID plasmaZUPInputPortID;
	CNC_IO_ID plasmaZDownInputPortID;

    int allowTHCWithoutM3M4;
    int allowTHCWithoutPlasmaMachine;



};

/************************************************************* 
    3D printing.

    We have a extruder with temp-control by PID.
    Extruder temp sensor is on analog 1
    Extruder PWM power is PWM 1

    Heat bed with temp control by PID
    heat Bed temp sensor is in analog 2
    Heat Bed PWM power is on PWM 2

    Workpiece cooling fan is on the PWM3.
    Workpiece cooling has no temperature feedback.

 *************************************************************/
struct CNC_3DPRINTING_VOLT_TEMP_TUPLE
{
    int   volt;
    float temp;
};

struct CNC_3DPRINTING_TEMP_CALIBRATION_TABLE
{
    CNC_3DPRINTING_VOLT_TEMP_TUPLE table[CNC_MAX_3D_TEMP_TABLE];
    int actualTableSize;
};

struct CNC_3DPRINTING_TEMP_PID_PARS
{
    float KP;
    float KD;
    float KI;
    float maxIntegratorTerm;
    float setPointReachedWindow;
    float maxPower;
    float maxTemp;
    float standByTemp;
    float sampleTime;
    float setpointTemp;
    int   ignoreTempError;

};

struct CNC_3DPRINTING_CONFIG
{
    /* main extruder parameters */
    CNC_3DPRINTING_TEMP_PID_PARS extruderPIDPars;
    CNC_3DPRINTING_TEMP_CALIBRATION_TABLE extruderVoltTempTable;
    char extruderVoltTempTableFileName[CNC_MAX_PATH];

    /* heat bed parameters */
    CNC_3DPRINTING_TEMP_PID_PARS heatBedPIDPars;
    CNC_3DPRINTING_TEMP_CALIBRATION_TABLE heatBedVoltTempTable;
    char heatBedVoltTempTableFileName[CNC_MAX_PATH];
    
    /* Workpiece cooling parameters */
    float workPieceFanStandbyPower; 

    /* below this power the fan is switched off */
    float workPieceFanPowerTreshold;

    /* kik start time with 100% PWM when starting the FAN */
    float workpieceFanStartTimeSeconds;

    /* translate g0 to g1 if 1 */
    int translateG0ToG1;
};


typedef enum
{
    CNC_3DP_SET_EXTRUDER_ONOFF     = 1,
    CNC_3DP_SET_EXTRUDER_STANDBY   = 2,
    CNC_3DP_SET_EXTRUDER_PIDONOFF  = 3,
    CNC_3DP_SET_EXTRUDER_TEMP_MAX  = 4,
    CNC_3DP_SET_EXTRUDER_TEMP      = 5,
    CNC_3DP_SET_EXTRUDER_TEMP_WAIT = 6,
    CNC_3DP_SET_EXTRUDER_PARS      = 7,
    CNC_3DP_SET_EXTRUDER_POWER     = 8,
    CNC_3DP_SET_HEATBED_ONOFF      = 9,
    CNC_3DP_SET_HEATBED_STANDBY    = 10,
    CNC_3DP_SET_HEATBED_PIDONOFF   = 11,
    CNC_3DP_SET_HEATBED_TEMP_WAIT  = 12,
    CNC_3DP_SET_HEATBED_TEMP       = 13,
    CNC_3DP_SET_HEATBED_PARS       = 14,
    CNC_3DP_SET_HEATBED_POWER      = 15,
    CNC_3DP_SET_WPFAN_ONOFF        = 16,
    CNC_3DP_SET_WPFAN_STANDBY      = 17,
    CNC_3DP_SET_WPFAN_POWER        = 18,
    CNC_3DP_SET_WPFAN_PARS         = 19,
    CNC_3DP_ALL_OFF                = 20,
    CNC_3DP_DEFAULT_PARS           = 21,
    CNC_3DP_EXTRUDER_RES_OVERRIDE  = 22,
    CNC_3DP_UPDATE_SHOW_AS_GO_FEED = 23
} CNC_3DP_COMMAND_ID;


typedef struct CNC_3DP_ONOFF_DATA
{
    CNC_ONOFF onOff;

} CNC_3DP_ONOFF_DATA;

typedef struct CNC_3DP_FLOATVAL_DATA
{
    float value;

} CNC_3DP_FLOATVAL_DATA;


typedef struct
{
    CNC_3DPRINTING_TEMP_PID_PARS pidPars;
    CNC_3DP_ONOFF_DATA onOffVal;
    CNC_3DP_FLOATVAL_DATA floatVal;

} CNC_3DP_CMD_DATA;

struct CNC_3DPRINTING_COMMAND
{
    CNC_3DP_COMMAND_ID cmdID;
    CNC_3DP_CMD_DATA   d;
};

struct CNC_3DPRINTING_STS
{
    //Actual PID parameters
    CNC_3DPRINTING_TEMP_PID_PARS extruderPIDPars;
    CNC_3DPRINTING_TEMP_PID_PARS heatBedPIDPars;

    //Extruder status visible in GUI
    float extruderSetPointTemp;
    float extruderActualTemp;
    float extruderHeatingPower;
    float extruderOverride;
    CNC_ONOFF extruderPIDIsON;
    CNC_ONOFF extruderHeatingIsOn;
    CNC_ONOFF extruderInStandByMode;
    CNC_ONOFF extruderSetPointReached;

    //HeatdBED status
    float heatBedSetPointTemp;
    float heatBedActualTemp;
    float heatBedHeatingPower;
    CNC_ONOFF    heatBedPIDIsON;
    CNC_ONOFF    heatBedHeatingIsOn; 
    CNC_ONOFF    heatBedInStandByMode;
    CNC_ONOFF    heatBedSetPointReached;

    //Workpiece FAN
    float workPieceActualCoolFanPower;
    float workPieceActualCoolFanPowerThreshold;
    CNC_ONOFF workPieceCoolingFanIsOn;
};



struct CNC_TRAJECTORY_CFG
{
    /* trajectory interpolation time in seconds e.g. 0.01 second */
	double interpolationTime;
	double fifoTime;
	double minCircleSegmentSize;

    /* Laf reports G1 is G0 when velocity higher than this */
    /* required for 3D printing because 3D printing CAM software always outputs G1 */
    double g1ShowsAsG0VelLimit;

    /* look ahead feed angle in degrees, connecting lines with 
       angle between this value and 180 are considered straight */
    double lafAngle;
    double lafAccFilter;

    /* on some hi speed machines, the velocity and acceleration for milling (G1/G2/G3) can be different as for positioning (G0) */
    /* the values below are factors for the velocity and acceleration for positioning movements (G0) */
    double g0VelocityFactor;
    double g0AccelelerationFactor;
};


struct CNC_INTERPRETER_CONFIG
{
    /* g64 active=1 or not=0 and its p-value  */
    double g64pvalue;
    int    g64Active; 

	/* long file mode criterion in bytes, GUI switches to low memory usage */
	int longFileModeCriterion;

	/* super long file mode criterion in bytes, Server runs job from file in stead of memory */
	int superLongFileModeCriterion;

    /* macro file name */
    char macroFileName[CNC_MAX_PATH];
    char userMacroFileName[CNC_MAX_PATH];

    /* line of g-code that is executed initially */
    char initialString[CNC_MAX_INTERPRETER_LINE];

    /* if 1, G4 timing is in Milli seconds in stead of seconds */
	int  g4InMilliseconds;

	/* INCH mode active */
	int		inchModeActive;
 
    int diameterProgramming;  /* Special for turning */
    int absoluteCenterCoords; /* if 1, circle center coordinates are absolute */

    /* tool table */
    CNC_TOOL_DATA_INTERNAL toolTable[CNC_MAX_TOOLS + 1];
	
	/* do not halt program on tool change request */
	int noHaltForToolChange;

    /* halt or not if Errmsg ... is executed during rendering */
    /* this setting is needed from compatibility */
    /* stopping on errmsg while render allows to detect and stop interpreter errors during render */
    /* however not all existing g-code, especially macro.cnc code can handle this */
    int noHaltOnErrMsgDuringRender;

    /* additional for Tan Knife */
    double tanKnifeAngleForLiftUp;
    double tanKnifeBlendAngle;
    double tanKnifeBlendDistance;
    double tanKnifeLiftUpDistance;
    /* bending angle with respect to rotation angle, + or - 90 or 0 for no bending */
    int    tanknifeBendingAngle;   
    /* Z hi use with 45 degree bend knife */
    double tanknifeZhiWork;                
    /* Z lo use with 45 degree bend knife */
    double tanknifeZloWork;                
    /* nt of turns after which a rewind is executed to unwind cabling */
    int tanknifeRewindTurns;            


};



struct CNC_UI_CFG
{
	/* The language -1=eng, 0=duits. 1=ned ... etc */
	CNC_LANG_T language;

    /* If 1 GUI uses OpenGL */
	int useOpenGL; 

    /* Save memory on RT view */
    int OpenGLRTMaxLines;
            
    /* OpenGL Pen Size */
    double openGlPenScaleFactor; 

    /* grid scale, standard grid size is 50mm, with scale 2 it becomes 100 etc*/
    double gridScaleFactor;
	
    /* OpenGL shows orthogonal, no perspective */
    int orthogonalView; 

    /* OpenGL background grey scale color 0-255 */
    int backGroundColor;

    /* -1 rotates inversed, 1 rotates normally */
    int rotationDirectionAAxis;

	/* show maximized */
	int restoreWindowPosition;

    /* Editor use for edit .cnc files files */
    char favoriteEditor[CNC_MAX_PATH];
    
    /* Directory where user GUI icons are loaded from */
    char iconDirectory[CNC_MAX_PATH];
    
    /* User logo file name */
	char logoFileName[CNC_MAX_PATH];
    
    /* Password for entering setup page */
	char setupPassword[80];

   
    /* id 1, no pop up dialog while zeroing. */
    int simpleZeroing; 

    /* The GUI automatically loads and runs this job file */
    char watchLoadFileName[CNC_MAX_PATH];
    int  watchFileChanged;
	int  watchAutoLoad;
    int  watchAutoRun;

    /* show terms if not already shown */
    int showTerms;

    /* show startup screen */
    int showStartupScreen;

    /*show graph buttons for zooming etc */
    int showGraphZoomButtons;

    /* for machines with moving bed */
    int invertJogKeysX;
	int invertJogKeysY;
	int invertJogKeysZ;

    /* after keyboardTimeOut UI will stop jogging */
    double keyBoardTimeOut;

    /* correction factor for job time estimation */
	double timeEstimateFactor; 

    /* adjust estimation runtime based on average speed and distToGo */
	int adjustEstimation; 

    /* possibility to hide function key's */
    int showM7key;
    int showM8key;
    int showAux1Key;
    int showSpindleOnButton;
    int showTrafficButton;
    int showStartButton;
    int showZeroButtons;

    int showLeftUser11Button;
    int showLeftUser12Button;
    int showLeftUser13Button;
    int showLeftUser14Button;
    int showLeftUser15Button;

    int showCoordinatesTab;
    int showProgramTab;
    int showToolTab;
    int showVariablesTab;
    int showIOTab;
    int showFifoSize;
    int showServiceTab;
    int showUtilTab;

    int showEngrave;
    int showDrill;
    int showOffset;
    int showPocket;
    int showICP;

    int clickDROIsG0;

    int windowRectLeft;
    int windowRectTop;
    int windowRectRight;
    int windowRectBottom;
    int windowRestoreFlags;
    int windowRestoreShowCmd;

    /* for spindle visualisation */
    /* show analog input (power) or PWM value at programmed spindle speed */
    /* 0 =  prog speed, 1 = PWM value, 2 = analogIn1, 3 = analogin2, 4 is analog in 3 */
    int    showInProgSpeed;
    double showInProgSpeedAnaMulFactor;

    /* show feed in mm/sec */
    int    feedUnitsMMPerSecond;

    /* show position units in 0.0001 format for mm mode */
    int    mmMode4digits;

    /* feed override step size when pressing +/- FeedOverride buttons */
    double feedOVStepSize;

    /* start XHC connect automatically if option enabled*/
    int autoStartXHCPendant;
    
};


struct CNC_SYSTEM_CONFIG
{
    //Server path and version
    char serverPath[CNC_MAX_PATH];
    char serverVersion[CNC_MAX_NAME_LENGTH];
    char lastKnownServerVersion[CNC_MAX_NAME_LENGTH];
    char lastKnownFirmwareVersion[CNC_MAX_NAME_LENGTH];
    int64_t et;
    int  newIniFileWasCreated;

    //Name of com port e.g. "COM3"
    char comPortName[CNC_COMMPORT_NAME_LEN];

	/* Scan Ethernet  */
	int  scanEthernet;
    
    /* number of physical joints in the mechanism*/
    int nrOfJoints;

  	/* choose max step rate of controller */
	int controllerTimerDividerIndex;

	/* choose polarity of step pulse output */
	int stepPulsePolarity;

    /* machine type */
    CNC_MACHINE_TYPE machineType;

    /* base PWM frequency */
    int pwmFrequency;

    int reserved[4];
};

struct CNC_SERVICE_CFG
{
    /* total time that machine has performed a job */
    double stsUsageTimeHoursTotal;
    
    /* time since last service reset */
    double stsUsageTimeHoursService;

    /* total machine traveled distance in meters */
    double stsTraveledDistMetersTotal;

    /* traveled distance since last service */
    double stsTraveledDistMetersService;

    /* number of jobs that has been done */
    int stsNumJobsDoneTotal;

    /* number of jobs that has been done */
    int stsNumJobsDoneService;

    /* time of usage for next service */
    double parServiceTimeIntervalHours;

    /* travel distance for next service */
    double parServiceTravelDistInterval;

    double reserved[5];

};


struct CNC_I2CGPIO_RULE_CONFIG
{
	unsigned char cardIndex; /* card index 0-1 */
	unsigned char ruleIndex; /* 0 - 15 */
	CNC_GPIO_ACTION_TYPE action;   

	unsigned short int inputParticipantBits;
	unsigned short int inputCompareBits;

	char messageText[CNC_MAX_MESSAGE_TEXT];

};


struct CNC_I2CGPIO_CARD_CONFIG
{
	int _nrOfGpioCards;
	CNC_I2CGPIO_RULE_CONFIG ruleConfig[CNC_MAX_GPIOCARD_CARDS][CNC_MAX_GPIOCARD_RULES];
};


/* next are basically all parameters from cnc.ini file */
struct CNC_MACHINE_CONFIG
{
    /* System */
    CNC_SYSTEM_CONFIG systemCfg;

    /* configuration for each joint */
    CNC_JOINT_CFG jointCfg[CNC_MAX_JOINTS];

    /* spindle config, 3x for 3 possible spindles */
    CNC_SPINDLE_CONFIG spindleCfg[CNC_MAX_SPINDLES];

    /* IO */
    CNC_IO_CONFIG ioCfg;

    /* kinematics-Cartesian configuration */
    CNC_KIN_CFG kinCfg;

    /* interpreter */
    CNC_INTERPRETER_CONFIG interpreterCfg;

    /* trajectory generation*/
    CNC_TRAJECTORY_CFG trajCfg;

    /* plasma machine config */
    CNC_THC_CFG plasmaCfg;

    /* E Stop, safety input */
    CNC_SAFETY_CONFIG safetyCfg;

    /* probing */
    CNC_PROBING_CFG probingCfg;

    /* traffic Light */
    CNC_TRAFFIC_LIGHT_CFG trafficLightCfg;

    /* hand wheel */
    CNC_HANDWHEEL_CFG handwheelCfg;

    /* FeedSpeedOverride config */
    CNC_FEEDSPEED_CFG feedSpeedOVCfg;

	/* GPIO card configuration */
	CNC_I2CGPIO_CARD_CONFIG gpioCfg;

	/* config parameters of USB camera */
	CNC_CAMERA_CONFIG cameraCfg;

    /* config parameters related to UI */
    CNC_UI_CFG uiCfg;

    /* service parameters and status */
    CNC_SERVICE_CFG serviceCfg;

    CNC_3DPRINTING_CONFIG print3DCfg;
};


struct CNC_IO_PORT_STS
{
    /* The ID */
    CNC_IO_ID      ioId;	

    /* Actually configuration item, set by configuration while reading ini file */
	int	   invert;	

    /* the current logic value, active/on = 1, inactive/off = 0 */
    int    lvalue;	

    /* the current raw value direct from CPU */
	int      rvalue;	

    /* 1 if present on CPU */
    int      present;	
};


struct CNC_GPIO_PORT_STS
{
    /* The ID */
    CNC_GPIO_ID  ioId;	

    /* Actually configuration item, set by configuration while reading ini file */
    int	   invert;	
    
    /* the current logic value, active/on = 1, inactive/off = 0 */
    int    lvalue;	

    /* the current raw value direct from CPU */
	int      rvalue;	

    /* 1 if present on CPU */
    int      present;	

    /* IO name */
    char name[CNC_MAX_IO_NAME_LENGTH];

};


struct CNC_JOINT_STS
{    
    /* joint index starting from 0 */
    char            jointIndex;
    CNC_JOINT_STATE state;
    int             errorWord;
	double          position; 
	double          positionRaw; 
    double          maxPositionError;
    int             isHomed;
    int             homeSensorStatus;
    int             jointIsConfigured;
};


struct CNC_JOB_STATUS
{    
    /* Job file name*/
    wchar_t jobName[CNC_MAX_PATH];

	/* incremented every time a job is loaded */
	int jobLoadCounter;
	int numLinesInjob;
    int numLinesInMacro;
    int numLinesInUserMacro;
	int64_t numBytesInJob;
	int isLongJob;
	int isSuperLongJob;


    /* 0 if job needs rendering, 1 if rendered */
	int jobIsRendered;

	/* total job length in mm */
	double totalJobLength;

	/* length of job done */
	double jobProgress;

	/* actual running time */
    double jobActualRunningTime;

	/* Estimated remaining running time */
    double jobRemainingRunningTime;

    /* Estimated total Time */
    double jobEstimatedTime;

	/* job collision status */
	int TCACollision;
	int MCACollision;
	int xCollision;
	int yCollision;
	int zCollision;

    /* render progress */
    int jobRenderLine;
    double jobRenderProgressPercentage;


    /* current interpreter line number starting from 1*/
    int curIpLine;

    /* current interpreter text */
    char curIpLineText[CNC_MAX_INTERPRETER_LINE+1];

    /* current execution line number starting from 1*/
    int curExLine;

    /* last known executed interpreter line use for goto to resume where left after pause */
    int lastKnownExcutedLineNumber;

    /* last known tool change line number */
    int lastKnownToolChangeLineNumber;


};

struct CNC_SPINDLE_STS
{
    /* sync count, spindle pulse */
    int syncCount;

    /* spindle rate, rev/sec RPS */
    double actualSpindleSpeedSigned;

    /* programmed spindle speed RPS */
    double programmedSpindleSpeed;
    
    /* Speed override Factor */
    double speedOverrideFactor;

    /* 1 of on */  
    int spindleIsOn;
    
    /* 1 if ccw */  
    int spindleDirection;
       
    /* value 0-1000%% */
    int spindlePWMPrecentage;    

    /* set if CPU supports spindle/feed synchronization */
    int feedSpeedSyncAvailable;

    /* index 0-2, tells actual configuration */
    int actualSpindleConfigurationIndex;

    /* spindle is ramping up */
    int isRampingUp;
    
    CNC_SPINDLE_CONFIG spindleCfg;
};

struct CNC_PAUSE_STS
{
    /* 1 if paused for manual action, like manual tool change */
    int pauseManualActionRequired;
    
    /* position which is stored after pause */
    CNC_CART_DOUBLE pausePosition;
    int pausePositionValid;
    int pausePositionLine;
    int pauseSpindleIOValue;
    int pauseAux1IOValue;
    int pauseAux2IOValue;
    int pauseAux3IOValue;
    int pauseAux4IOValue;
    int pauseAux5IOValue;
    int pauseAux6IOValue;
    int pauseAux7IOValue;
    int pauseAux8IOValue;
    int pauseAux9IOValue;
    int pauseMistIOValue;
    int pauseFloodIOValue;

    /* So GUI knows Array indices where paused */
    int pauseArrayIndexX; 
    int pauseArrayIndexY; 
    int pauseDoArray;

    //These are monitored and updated by the server
    //GUI can show axes/IO that are in sync (at correct value for resume)
    CNC_CART_BOOL curPosInSync;
    int spindleInSync;
    int floodInSync;
    int mistInSync;
    int allAxesInSync;
};


typedef struct CNC_BASIC_INTERPRETER_STATUS
{
    CNC_CART_BOOL axesPresent;
	CNC_CART_DOUBLE position; //mm
	CNC_OFFSET_AND_PLANE activeOffsetAndPlane; 
	int spindleOn;
	int mist;
	int flood;
	double speed;
	int toolInSpindle;
	double feed;
	int inchMode;
    double lafAngle;
    double parameters0to27[27+5]; //First 27 + 101-105, otherwize this becomes too much

} CNC_BASIC_INTERPRETER_STATUS;


struct CNC_SEARCH_STATUS
{
    CNC_BASIC_INTERPRETER_STATUS basicIntStatusBeforeSearch;
    CNC_BASIC_INTERPRETER_STATUS basicIntStatusAfterSearch;

    //These are monitored and updated by the server
    //GUI can show axes/IO that are in sync (at correct value for resume)
    CNC_CART_BOOL curPosInSync;
    int spindleInSync;
    int floodInSync;
    int mistInSync;
    int toolInSync;
};

struct CNC_TRACKING_STATUS
{
    /* actual tracking status */
	CNC_TRACKING_MODE curTrackingMode; 
	CNC_TRACK_SOURCE  curTrackSource[CNC_MAX_AXES];
	CNC_CART_BOOL     curTrackingPosReached;
	CNC_CART_BOOL     curAxesIsTracking;
    double            curHandwheelMultiplicationFactor;

	/* position which is currently tracked */
	CNC_CART_DOUBLE trackingPosition;
	CNC_CART_DOUBLE trackingVelocity;

	/* hand wheel counters */
	int trackingHandwheelCounter[3];

	/* single flag protocol */
	int newTrackingDataFlag;
};

struct CNC_COMPENSATION_STATUS
{
    /* Z Height bilinear compensation */
    int    zHeightCompIsOn;
    int    outOfArea;

    /* current comp value of all compensations */
    CNC_JOINT_DOUBLE curCompValue;
    
    /* joint backlash compensation */
    CNC_JOINT_BOOL backlashCompIsOn;

    /* linear joint compensation */
    CNC_JOINT_BOOL linearJointCompIsOn;

};


struct CNC_THC_STATUS
{
    /* THC actual parameters */
    CNC_THC_PROCESS_PARAMETERS actualProcessPars;

    /* status vars */
    double voltAct;
    double controllerOut;

    int    isOn;
	int    isTracking;
	int    isActive;
    int    curveProtectActive;
	int    holeDetectActive;

	int    plasmaOnInput;
	int    isCuttingMove;
	int    isTuningMode;

};




struct CNC_NESTING_STATUS
{
    /* job array status */
    int doArray;
    int arrayNX;
    int arrayNY;
    double arrayDX;
    double arrayDY;
    double arrayStartOffsetX;
    double arrayStartOffsetY;
    double arrayMaterialSizeX;
	double arrayMaterialSizeY;
	double arrayMaterialSizeZ;
    int arrayCurXIndex;
    int arrayCurYIndex;
    double arrayCurXOffset;
    double arrayCurYOffset;
};


struct CNC_CONTROLLER_CONFIG_STATUS
{
    int  comPortListSize;
    char comPortList[CNC_MAX_COMM_PORTS][CNC_COMMPORT_NAME_LEN];
    int  connectedViaEtherNet;
    int  connectedViaUSB;
    int  connectedViaSimulation;

	/* if the USB/ETH plug is pulled out or power is switched OFF from CPU */
	int  connectionError;


    /* controller frequency table */
    double controllerFrequencies[CNC_MAX_FREQENCY_TABLE_LEN];
    int    controllerFrequencyTimerValues[CNC_MAX_FREQENCY_TABLE_LEN];
    int    controllerNumberOfFrequencies;

    /* Controller firmware version, max axes, lathe capability */
    char controllerFirmwareVersion[CNC_MAX_NAME_LENGTH];
    int  controllerMaxAxes; 
    int  controllerAvailableAxes;


    /* 1 if firmware has SW options */
    int  firmwareHasOptions;

    /* if False, a g-code program cannot be run */
    int  cpuIsEnabled; 

	/* if false function is not available */
    int  avx1GPIOIsEnabled;
    int  ediGPIOIsEnabled;
    int  plasmaIsEnabled;
    
	/* initialize counter, incremented at every init */
	int initCount;
};


struct CNC_CONTROLLER_STATUS
{
    /* physical status of joints and IO */
    CNC_JOINT_STS jointSts[CNC_MAX_JOINTS];
    CNC_IO_PORT_STS cpuIoSts[CNC_IOID_LAST];

    /* GP IO board */
    int gpioAvx2Present[CNC_MAX_GPIOCARD_CARDS];
    int gpioRLY8Present[CNC_MAX_GPIOCARD_CARDS];
    CNC_GPIO_PORT_STS gpioSts[CNC_MAX_GPIOCARD_CARDS][CNC_GPIOID_LAST];

    /* free size in CPU motion fifo */
    unsigned char cpuFifoFreeSize;

    /* filled size in CPU motion fifo */
    unsigned char cpuFifoFilledSize;

    /* motion enabled status */
    int motionEnabled;

    /* homing or probing is active */
    int capturing;

    /* position captured */
    int isCaptured;

    /* motion error word */
    int errorWord;

    /* hand wheel count */
    int handWheelCount1;
    int handWheelCount2;

    /* Aux axis */
    int auxAxPos;
    int auxAxState;
    unsigned int auxAxCi;
    unsigned int auxAxCurN;
    int auxAxDirection;
    int auxAxIsHomed;
    int auxAxisIs32bitMode;
};


struct CNC_MOTION_STATUS
{
    /* Cartesian position without offset from generator */
    CNC_CART_DOUBLE machineCartesianPosition;

    /* joint positions from generator */
    CNC_JOINT_DOUBLE machineJointPosition;
    
    /* active coordinate system offset, tool offset, rotation and plane */
    /* with this and above machine position, the program position is to be calculated */
    CNC_OFFSET_AND_PLANE activeOffsetAndPlane;

    /* actual feed in X-Y plane, traveled distance / time * 60 [mm/minute]*/
    double actualFeed;

    /* feed override set by GUI */
    double userFeedOverride;

    /* arc feedOverride set by GUI */
    double userArcFeedOverride;

    /* 1 if safe mode active */
    int  safeMode; 

    /* safety input value if present, 0 or 1, -1 if not present */
    int safetyInputValue;

    /* 0 if real mode, 1 and 2 are simulation mode, 1 normal, 2 fast */
    int simulationMode;

    /* incremented every motion update cycle */
    int heartBeat;

    /* feed and speed override status */
    int feedOvEnabled;
    int speedOvEnabled;
    int analogFeedOvEnabled;
    int feedHoldActive;
    int speedHoldHactive;


};


struct CNC_RUNNING_STATUS
{
    /* state of the interpreter/execution task */
    CNC_IE_STATE state;

    /* active G-code for each of the G-code groups, the value contains the active g-code in the group multiplied by 10  */
    int  ActiveGCodes[17];

    /* idem for M-code for each of the M groups */
    int  ActiveMCodes[11];

    /* settings lineNumber, current Feed, current Speed, current Tool (LFST)*/
    double ActiveGSettings[4];

    /* last interpreter error info*/
    CNC_LOG_MESSAGE  lastIntError;   

    /* 0 block delete is off, 1 block delete is on */
    int blockDelete;

    /* 1 when single stepping mode active */
    int stepMode;

    /* optional stop M1 */
    int optionalStopOn;

	/* 0=nothing 28=G28 home, 30=G30 home */
	char extraLineWhenEndOfJob[CNC_MAX_INTERPRETER_LINE];

    /* sub reset present in macro.cnc */
    int subResetPresent;
};



struct CNC_TRAFFIC_LIGHT_STATUS
{
    CNC_TRAFFIC_LIGHT_COLOR trafficLightColor;
    int trafficLightBlink;//True if blink
    char traficLightReason[CNC_MAX_LOGGING_TEXT];
};


struct CNC_STATUS
{
	CNC_RUNNING_STATUS runningStatus;
    CNC_MOTION_STATUS  motionStatus;
    CNC_CONTROLLER_STATUS controllerStatus;
    CNC_CONTROLLER_CONFIG_STATUS controllerConfigStatus;
    CNC_TRAFFIC_LIGHT_STATUS trafficLichtStatus;
    CNC_JOB_STATUS jobStatus;
	CNC_TRACKING_STATUS trackingStatus;
    CNC_THC_STATUS thcStatus;
    CNC_NESTING_STATUS nestingStatus;
    CNC_KIN_STATUS kinStatus;
    CNC_SPINDLE_STS spindleSts;
    CNC_PAUSE_STS pauseSts;
    CNC_SEARCH_STATUS searchSts;
    CNC_3DPRINTING_STS print3DSts;
    CNC_COMPENSATION_STATUS zHeightCompSts;
};


struct CNC_POS_FIFO_DATA
{
	CNC_CART_DOUBLE   pos;     /* x,y,z position vector  */
	CNC_MOVE_TYPE     type;    /* see GRAPH type         */
};

struct CNC_GRAPH_FIFO_DATA
{
    int             lineNumber;
	CNC_CART_DOUBLE pos;     /* end position, origin offset, start pos, tool offset, depending on type */
	CNC_MOVE_TYPE   type;    /* see GRAPH type         */
    int             msgNumber;
};


struct CNC_RENDER_DATA
{
	int             lineNr;   /* job line number */
	CNC_MOVE_TYPE   type;     /* see GRAPH type         */
	CNC_CART_DOUBLE pos;      /* end position, origin offset, start pos, tool offset, depending on type */
    CNC_VECTOR      center;   /* center for arc */
    CNC_VECTOR      normal;   /* for arc */
	int	            turn;     /* for arc direction & n of turns */
    CNC_OFFSET_AND_PLANE offsetAndPlane;
	double		    velocity;
    double		    acceleration;
	bool		    linearIsLeading;

};

/* Fool compiler to think a variable is used */
#define CNC_UNUSED( ident )  (void) (ident = ident)
    
#pragma pack()

#endif //already included
