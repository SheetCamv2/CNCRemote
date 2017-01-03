#ifndef CNC_TYPES_MANAGED_H_INCLUDED
#define CNC_TYPES_MANAGED_H_INCLUDED



#ifndef USBCNC_VERSION
#define USBCNC_VERSION "USBCNC V4.01.B12"
#endif

#ifndef CNC_EPSILON
#define CNC_EPSILON (0.00001)
#endif

#ifndef CNC_DOUBLES_ARE_EQUAL
#define CNC_DOUBLES_ARE_EQUAL(x,y)  (int)(fabs( (double)((x)-(y)) ) < CNC_EPSILON )
#endif

#ifndef CNC_DOUBLES_ARE_EQUAL_E
#define CNC_DOUBLES_ARE_EQUAL_E(x,y,e)  (int)(fabs( (double)((x)-(y)) ) < e )
#endif

#include <string.h>

#include "..\pub\cncapi.h"



//shared Enums
/// <summary>
/// Enum eCncMachineType
/// </summary>
public enum class eCncMachineType
{
	CNC_MACHINE_TYPE_MILLING = 0,
	CNC_MACHINE_TYPE_TURNING = 1,
	CNC_MACHINE_TYPE_TURNING_BACK_X = 2,
	CNC_MACHINETYPE_4AXES_FOAMCUTTER = 3
};


/// <summary>
/// Enum CNC_RC
/// </summary>
/// <remarks>
/// Most CNCAPI functions return this result code
/// </remarks>
public enum class eCncConstants
{
	//Max joints (motors) and max kinematic axes (x,y,z,a,b)
	CNC_MAX_JOINTS =  6,
	CNC_MAX_AXES   =  6,
	CNC_MAX_NAME_LENGTH = 32,
	CNC_MAX_PATH = 260,
	CNC_MAX_FNAME_LENGTH = 32,
	CNC_MAX_LOGGING_TEXT = 120,
	CNC_MAX_SOURCE_INFO_TEXT = 30,
	CNC_MAX_FUNCTION_NAME_TEXT = 30,
	CNC_MAX_MESSAGE_TEXT = 150,
	CNC_MAX_VARS = 5800,
	CNC_MAX_TOOLS = 16,
	CNC_MAX_CURRENT_GCODE_TEXT = 60,
	CNC_MAX_CURRENT_MCODE_TEXT = 20,
	CNC_MAX_EXPRESSION_TEXT = 80,
	CNC_MAX_INTERPRETER_LINE = 255,
	CNC_POS_FIFO_SIZE = 100,
	CNC_POS_FIFO_MARGIN = 2,
	CNC_GRAPH_FIFO_SIZE = 100,
	CNC_GRAPH_FIFO_MARGIN  = 2,
	CNC_LOG_FIFO_SIZE = 25,
	CNC_LOG_FIFO_MARGIN = 2,
	CNC_MAX_INTERPRETER_LOOKAHEAD = 500,
	CNC_MAX_VERSION_TEXT = 50,
	CNC_COMMPORT_NAME_LEN = 20,
	CNC_MAX_COMM_PORTS = 50,
	CNC_MAX_OUTPUT_PORTS = 18,
	CNC_MAX_INPUT_PORTS  = 32,
	CNC_MAX_GPIO_OUTPUT_PORTS = 16,
	CNC_MAX_GPIO_INPUT_PORTS = 16,
    CNC_MAX_GPIO_ANIN_PORTS = 8,
    CNC_MAX_GPIO_PWM_OUT_PORTS = 4,
	CNC_MAX_IO_PORTS = (CNC_MAX_OUTPUT_PORTS + 
                        CNC_MAX_INPUT_PORTS + 
                        CNC_MAX_GPIO_OUTPUT_PORTS + 
                        CNC_MAX_GPIO_INPUT_PORTS + 
                        CNC_MAX_GPIO_PWM_OUT_PORTS +
                        CNC_MAX_GPIO_ANIN_PORTS + 1), 
	CNC_MAX_PRECISION_TEXT = 32,
	CNC_TOOL_DIAMETER_INDEX = 5400, //First var index of tool diameter
	CNC_TOOL_ZOFFSET_INDEX = 5500,    //First var index of tool lenght
	CNC_TOOL_XOFFSET_INDEX = 5600,    //First var index of tool lenght
   	CNC_TOOL_ORIENTATION_INDEX = 5701    //First var index of tool orientation

};


/// <summary>
/// Enum CNC_RC
/// </summary>
/// <remarks>
/// Most CNCMA functions return this result code
/// </remarks>
public enum class eCncRC
{
    CNC_RC_OK           =  0,       // success 
    CNC_RC_BUF_EMPTY    =  1,       // buffer empty 
    CNC_RC_TRACE        =  2,       // trace info 
    CNC_RC_USER_INFO    =  3,       // User message 
    CNC_RC_SHUTDOWN     =  4,       // returned by process request after CMD_CLOSE 
	CNC_RC_EXISTING     =  5,       // if shared mem already exists 
	CNC_RC_ALREADY_RUNS =  6,       // if server already running 
    CNC_RC_ERR          = -1,       // no data in fifo 
    CNC_RC_ERR_PAR      = -2,       // wrong parameter or parameter value, see text 
    CNC_RC_ERR_STATE    = -3,       // wrong state, not allowed, see text 
    CNC_RC_ERR_INT      = -4,       // interpreter error, see interpreter error status 
    CNC_RC_ERR_CE       = -5,       // command envelope error 
    CNC_RC_ERR_EXE      = -6,       // execution error 
    CNC_RC_ERR_PIC      = -7,       // PIC error, see sub code, text 
    CNC_RC_ERR_MOT      = -8,       // trajectory generator error, see sub code, text 
    CNC_RC_ERR_SYS      = -9,       // server system error, see text 
    CNC_RC_ERR_TIMEOUT  = -10,      // general timeout error 
    CNC_RC_EXE_CE       = -11,      // Error executing command envelope 
	CNC_RC_ERR_FILEOPEN = -12,      // file open error 
	CNC_RC_ERR_COLLISION= -13       // Collision
};


/// <summary>
/// Enum CNC_ERROR_CLASS
/// </summary>
public enum class eCncErrorClass
{
    CNC_EC_INFO = 0,  //Info, no user action
	CNC_EC_DIALOG,    //For user interaction with partprogram
	CNC_EC_USERACTION,//User action request
    CNC_EC_WARNING,   //Warning eventual user action
    CNC_EC_STOP,      //Stopped on path
    CNC_EC_QSTOP,     //Stop immediate, path not maintained
    CNC_EC_ABORT,     //Emergency stop
    CNC_EC_BUG,       //SW bug report to supplier
    CNC_EC_FATAL      //unrecoverable system failure

};

/// <summary>
/// Enum CNC_IO_ID
/// </summary>
public enum class eCncIOID
{
    CNC_IOID_NONE = 0,           //No IO defined
    CNC_IOID_MACHINE_ON_OUT,     //Machine ON output
    CNC_IOID_DRIVE_ENABLE_OUT,   //Amplifier enable
    CNC_IOID_TOOL_OUT,           //Tool on
    CNC_IOID_COOLANT1_OUT,       //Flood
    CNC_IOID_COOLANT2_OUT,       //Mist
	CNC_IOID_TOOLDIR_OUT,        //Tool direction output
	CNC_IOID_AUX1_OUT,           //Aux1 output
	CNC_IOID_AUX2_OUT,           //Aux1 output
	CNC_IOID_AUX3_OUT,           //Aux1 output
	CNC_IOID_AUX4_OUT,           //Aux1 output
	CNC_IOID_AUX5_OUT,           //Aux1 output
	CNC_IOID_AUX6_OUT,           //Aux1 output
	CNC_IOID_AUX7_OUT,           //Aux1 output
	CNC_IOID_AUX8_OUT,           //Aux1 output
	CNC_IOID_AUX9_OUT,           //Aux1 output
	CNC_IOID_PWM_VAL1_OUT,       //PWM value output 0..100%
	CNC_IOID_PWM_VAL2_OUT,       //PWM value output 0..100%
	CNC_IOID_PWM_VAL3_OUT,       //PWM value output 0..100%

    CNC_IOID_EMSTOP1_IN,         //Emergency stop input
    CNC_IOID_EMSTOP2_IN,         //Emergency stop input
    CNC_IOID_EXTERR_IN,          //Emergency stop input
	CNC_IOID_PROBE_IN,           //Probe
	CNC_IOID_SYNC_IN,            //Spindle Sync Pulse
	CNC_IOID_RUN_IN,             //Run button
	CNC_IOID_PAUSE_IN,           //Pause button
	CNC_IOID_HOME1_IN,           //1 for all end of stroke on V1
	CNC_IOID_HOME2_IN,           //idem
	CNC_IOID_HOME3_IN,           //idem
	CNC_IOID_HOME4_IN,           //idem
	CNC_IOID_HOME5_IN,           //idem
	CNC_IOID_HOME6_IN,           //idem
    CNC_IOID_HW1A_IN,            //Handwheel 1
    CNC_IOID_HW1B_IN,            //Handwheel 1
    CNC_IOID_HW2A_IN,            //Handwheel 2
    CNC_IOID_HW2B_IN,            //Handwheel 2
    CNC_IOID_ANA1_IN,            //Analog1
    CNC_IOID_ANA2_IN,            //Analog2
    CNC_IOID_ANA3_IN,            //Analog3
	CNC_IOID_AUX1_IN,            //Aux1 input
	CNC_IOID_AUX2_IN,            //Aux1 input
	CNC_IOID_AUX3_IN,            //Aux1 input
	CNC_IOID_AUX4_IN,            //Aux1 input
	CNC_IOID_AUX5_IN,            //Aux1 input
	CNC_IOID_AUX6_IN,            //Aux1 input

	CNC_IOID_GPIO_OUT_0,         //General purpose io output
	CNC_IOID_GPIO_OUT_1,         //General purpose io output
	CNC_IOID_GPIO_OUT_2,         //General purpose io output
	CNC_IOID_GPIO_OUT_3,         //General purpose io output
	CNC_IOID_GPIO_OUT_4,         //General purpose io output
	CNC_IOID_GPIO_OUT_5,         //General purpose io output
	CNC_IOID_GPIO_OUT_6,         //General purpose io output
	CNC_IOID_GPIO_OUT_7,         //General purpose io output
	CNC_IOID_GPIO_OUT_8,         //General purpose io output
	CNC_IOID_GPIO_OUT_9,         //General purpose io output
	CNC_IOID_GPIO_OUT_10,        //General purpose io output
	CNC_IOID_GPIO_OUT_11,        //General purpose io output
	CNC_IOID_GPIO_OUT_12,        //General purpose io output
	CNC_IOID_GPIO_OUT_13,        //General purpose io output
	CNC_IOID_GPIO_OUT_14,        //General purpose io output
	CNC_IOID_GPIO_OUT_15,        //General purpose io output

	CNC_IOID_GPIO_IN_0,            //General purpose io input
	CNC_IOID_GPIO_IN_1,            //General purpose io input
	CNC_IOID_GPIO_IN_2,            //General purpose io input
	CNC_IOID_GPIO_IN_3,            //General purpose io input
	CNC_IOID_GPIO_IN_4,            //General purpose io input
	CNC_IOID_GPIO_IN_5,            //General purpose io input
	CNC_IOID_GPIO_IN_6,            //General purpose io input
	CNC_IOID_GPIO_IN_7,            //General purpose io input
	CNC_IOID_GPIO_IN_8,            //General purpose io input
	CNC_IOID_GPIO_IN_9,            //General purpose io input
	CNC_IOID_GPIO_IN_10,           //General purpose io input
	CNC_IOID_GPIO_IN_11,           //General purpose io input
	CNC_IOID_GPIO_IN_12,           //General purpose io input
	CNC_IOID_GPIO_IN_13,           //General purpose io input
	CNC_IOID_GPIO_IN_14,           //General purpose io input
	CNC_IOID_GPIO_IN_15,           //General purpose io input

    CNC_IOID_GPIO_PWMOUT_0,        //General purpose io output
	CNC_IOID_GPIO_PWMOUT_1,        //General purpose io output
	CNC_IOID_GPIO_PWMOUT_2,        //General purpose io output
	CNC_IOID_GPIO_PWMOUT_3,        //General purpose io output

    CNC_IOID_GPIO_ANIN_0,          //General purpose io analogue input
	CNC_IOID_GPIO_ANIN_1,          //General purpose io analogue input
	CNC_IOID_GPIO_ANIN_2,          //General purpose io analogue input
	CNC_IOID_GPIO_ANIN_3,          //General purpose io analogue input
	CNC_IOID_GPIO_ANIN_4,          //General purpose io analogue input
	CNC_IOID_GPIO_ANIN_5,          //General purpose io analogue input
	CNC_IOID_GPIO_ANIN_6,          //General purpose io analogue input
	CNC_IOID_GPIO_ANIN_7,          //General purpose io analogue input

};

public enum class eCncState
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
	CNC_IE_RUNNING_HANDWHEEL_STATE,  /* handwheel operation */
	CNC_IE_RUNNING_LINE_HANDWHEEL_STATE,/* single line running from handwheel mode, can only be G92... */
	CNC_IE_RUNNING_JNTJOG_STATE,     /* running joint jog */
	CNC_IE_RUNNING_LINJOG_STATE,     /* running joint jog */

	/* Rendering and searching states */
	CNC_IE_RENDERING_GRAPH_STATE,    /* running interpreter for graph view only */
	CNC_IE_SEARCHING_STATE,          /* searching line */
	CNC_IE_SEARCHED_DONE_STATE,      /* searched line found */

	CNC_IE_LAST_STATE                /* keep last */
};


/// <summary>
/// Enum CNC_JOINT_STATE
/// </summary>
public enum class eCncJointState
{
    CNC_JOINT_POWER_UP            = 0,
    CNC_JOINT_MOVING_STATE        = 1,
    CNC_JOINT_READY_STATE         = 2,
    CNC_JOINT_READY_STOPPED_STATE = 3,
    CNC_JOINT_FREE_STATE          = 4,
    CNC_JOINT_ERROR_STATE         = 5, 
    CNC_JOINT_LAST_STATE          = 6
};

/// <summary>
/// Enum CNC_LANG_T
/// </summary>
public enum class eCncLangType
{
	CNC_LANG_ENGLISH   = 0,
	CNC_LANG_GERMAN    = 1,
	CNC_LANG_DUTCH     = 2,
	CNC_LANG_ITALIAN   = 3,
	CNC_LANG_FRENCH    = 4,
	CNC_LANG_SPANISH   = 5,
	CNC_LANG_PORTUGESE = 6,
	CNC_LANG_TURKISH   = 7,
	CNC_LANG_JAPANESE  = 8,
	CNC_LANG_NEW       = 9,
	CNC_LANG_LAST      = 10 	//Keep last

};

public enum class eCncMoveType
{
	CNC_MOVE_TYPE_UNKNOWN  = 0,
	CNC_MOVE_TYPE_G0  = 1,
    CNC_MOVE_TYPE_G1  = 2,
	CNC_MOVE_TYPE_ARC = 3,
	CNC_MOVE_TYPE_PROBE = 4,
	CNC_MOVE_TYPE_JOG = 5,
	CNC_MOVE_TYPE_HOME = 6,
    CNC_MOVE_TYPE_ORIGIN_OFFSET = 7,
    CNC_MOVE_TYPE_START_POSITION = 9,
    CNC_MOVE_TYPE_END = 10
};

public ref class CncPosition
{
public:
	double x,y,z,a,b,c;

public:
	CncPosition(void)
	{
		x = y = z = a = b = c = 0.0;
	}

	CncPosition(CNC_CART_DOUBLE &p)
	{
		x = p.x;
		y = p.y;
		z = p.z;
		a = p.a;
		b = p.b;
		c = p.c;
	}
};


public ref class CncVector
{
public:
	double x,y,z;

public:
	CncVector(void)
	{
		x = y = z = 0.0;
	}

	CncVector(CNC_CART_DOUBLE &p)
	{
		x = p.x;
		y = p.y;
		z = p.z;
	}
};


public ref class CncJointPosition
{
public:
	double m0, m1, m2, m3, m4;

public:
	CncJointPosition(void)
	{
		m0 = m1 = m2 = m3 = m4 = 0.0;
	}

	CncJointPosition(double p[])
	{
		m0 = p[0];
		m1 = p[1];
		m2 = p[2];
		m3 = p[3];
		m4 = p[4];
	}
};

/// <summary>
/// Log message structure for CNC events stored in FIFO buffer by CNC server
/// </summary>
public ref class CncLogMessage
{
public:	
	/// <summary>
	/// code where the error or what kind of message did occur, see CNC_RC
	/// </summary>
	eCncRC Code;			
	/// <summary>
	/// subcode is only relevant when code specifies a subcode
	/// </summary>
	int SubCode;
	/// <summary>
	/// the error class
	/// </summary>
	eCncErrorClass ErrorClass;
	/// <summary>
	/// textual error description
	/// </summary>
	System::String^ Text;	
	/// <summary>
	/// textual description of c-source and line number for error reporting
	/// </summary>
	System::String^ SourceInfo;
	/// <summary>
	/// textual description of function name for error reporting
	/// </summary>
	System::String^ FunctionName;
	/// <summary>
	/// time of log message
	/// </summary>
	System::DateTime^ Time;
	/// <summary>
	/// number
	/// </summary>
	int Number;

	/// <summary>
	/// parameter names for interpreter dialog
	/// </summary>
	System::String^ Par1Name;
	System::String^ Par2Name;
	System::String^ Par3Name;
	System::String^ Par4Name;
	System::String^ Par5Name;
	System::String^ Par6Name;
	System::String^ Par7Name;
	/// <summary>
	/// parameter numbers for interpreter dialog
	/// </summary>
	int Par1Number;
	int Par2Number;
	int Par3Number;
	int Par4Number;
	int Par5Number;
	int Par6Number;
	int Par7Number;



public:
	/// <summary>
	/// Initializes a new instance of CncLogMessage
	/// </summary>
	CncLogMessage::CncLogMessage(void)
	{
		Code			= eCncRC::CNC_RC_BUF_EMPTY;
		SubCode			= 0;
		ErrorClass		= eCncErrorClass::CNC_EC_INFO;
		Text			= System::String::Empty;
		SourceInfo		= System::String::Empty;
		FunctionName	= System::String::Empty;
		Time			= System::DateTime::Now;
		Number			= 0;
		Par1Name        = System::String::Empty;
		Par2Name        = System::String::Empty;
		Par3Name        = System::String::Empty;
		Par4Name        = System::String::Empty;
		Par5Name        = System::String::Empty;
		Par6Name        = System::String::Empty;
		Par7Name        = System::String::Empty;
		Par1Number		= 0;
		Par2Number		= 0;
		Par3Number		= 0;
		Par4Number		= 0;
		Par5Number		= 0;
		Par6Number		= 0;
		Par7Number		= 0;
	}  
public:
	/// <summary>
	/// Initializes a new instance of CncLogMessage
	/// </summary>
	/// <param name="logMsg">CNC server internal structure</param>
	CncLogMessage::CncLogMessage(CNC_LOG_MESSAGE &logMsg)
	{
		Code			= static_cast<eCncRC>( (int) (logMsg.code) );
		SubCode			= logMsg.subCode;
		ErrorClass		= static_cast<eCncErrorClass>( (int) (logMsg.errorClass) );
		Text			= gcnew System::String( logMsg.text );
		SourceInfo		= gcnew System::String( logMsg.sourceInfo );
		FunctionName	= gcnew System::String( logMsg.functionName );
		Time			= System::DateTime(1970, 1, 1).AddSeconds( (double) logMsg.timeStamp );
		Number			= logMsg.n;
		Par1Name        = gcnew System::String( logMsg.par1Name );
		Par2Name        = gcnew System::String( logMsg.par2Name );
		Par3Name        = gcnew System::String( logMsg.par3Name );
		Par4Name        = gcnew System::String( logMsg.par4Name );
		Par5Name        = gcnew System::String( logMsg.par5Name );
		Par6Name        = gcnew System::String( logMsg.par6Name );
		Par7Name        = gcnew System::String( logMsg.par7Name );
		Par1Number		= logMsg.par1Number;
		Par2Number		= logMsg.par2Number;
		Par3Number		= logMsg.par3Number;
		Par4Number		= logMsg.par4Number;
		Par5Number		= logMsg.par5Number;
		Par6Number		= logMsg.par6Number;
		Par7Number		= logMsg.par7Number;
	}
public:
	/// <summary>
	/// Initializes a new instance of CncLogMessage
	/// </summary>
	/// <param name="code">code where the error or what kind of message did occur, see CNC_RC</param>
	/// <param name="subCode">subcode is only relevant when code specifies a subcode</param>
	/// <param name="errorClass">the error class</param>
	/// <param name="text">textual error description</param>
	/// <param name="sourceInfo">textual description of c-source and line number for error reporting</param>
	/// <param name="functionName">textual description of function name for error reporting</param>
	/// <param name="time">time of log message</param>
	/// <param name="number">number</param>
	CncLogMessage::CncLogMessage(
		eCncRC code, int subCode, 
		eCncErrorClass errorClass,
		System::String^ text, 
		System::String^ sourceInfo, 
		System::String^ functionName,
		System::DateTime^ time, 
		int number,
		System::String^ par1Name,       
		System::String^ par2Name,       
		System::String^ par3Name,       
		System::String^ par4Name,       
		System::String^ par5Name,       
		System::String^ par6Name,       
		System::String^ par7Name,       
		int par1Number,	
		int par2Number,	
		int par3Number,	
		int par4Number,	
		int par5Number,	
		int par6Number,	
		int par7Number	
		)
	{
		Code			= code;
		SubCode			= subCode;
		ErrorClass		= errorClass;
		Text			= text;
		SourceInfo		= sourceInfo;
		FunctionName	= functionName;
		Time			= time;
		Number			= number;
		Par1Name		= par1Name;
		Par2Name		= par2Name;
		Par3Name		= par3Name;
		Par4Name		= par4Name;
		Par5Name		= par5Name;
		Par6Name		= par6Name;
		Par7Name		= par7Name;
		Par1Number		= par1Number;
		Par2Number		= par2Number;
		Par3Number		= par3Number;
		Par4Number		= par4Number;
		Par5Number		= par5Number;
		Par6Number		= par6Number;
		Par7Number		= par7Number;
	}
};

/// <summary>
/// Joint (axis)configuration definition
/// </summary>
public ref class CncJointConfig
{
public:
	/// <summary>
	/// logical name of the joint, used in GUI, one character
	/// </summary>
	char Name;
	/// <summary>
	/// Axis resolution - number of increments for one application unit
	/// </summary>
	double Resolution;
	/// <summary>
	/// Axis positive limit
	/// </summary>
	double PositiveLimit;
	/// <summary>
	/// Axis negative limit
	/// </summary>
	double NegativeLimit;
	/// <summary>
	/// max values for velocity in AU
	/// </summary>
	double MaxVelocity;
	/// <summary>
	/// max values for acceleration in AU/s^2
	/// </summary>
	double MaxAcceleration;
	/// <summary>
	/// homing parameters, AU - sign is direction
	/// </summary>
	double HomeVelocity; 
	/// <summary>
	/// position at home sensor
	/// </summary>
	double HomePosition; 
	/// <summary>
	/// backlash parameters
	/// </summary>
	double BackLash;

	/// <summary>
	// jogspeed percentages 
	/// </summary>
	double LowSpeedJogPercent;
	double MedSpeedJogPercent;
	double HighSpeedJogPercent;

public:
	/// <summary>
	/// Initializes a new instance of CncJointConfig
	/// </summary>
	CncJointConfig::CncJointConfig(void)
	{
		Name = 0;
		Resolution = 0.0;
		PositiveLimit = 0.0;
		NegativeLimit = 0.0;
		MaxVelocity = 0.0;
		MaxAcceleration = 0.0;
		HomeVelocity = 0.0; 
		HomePosition = 0.0; 
		BackLash = 0.0;
		LowSpeedJogPercent = 10.0;
		MedSpeedJogPercent = 50.0;
		HighSpeedJogPercent =100.0;
	}

public:
	/// <summary>
	/// Gets CncJointConfig
	/// </summary>
	/// <param name="joint">Internal struct used by CNC Server</param>
	eCncRC CncJointConfig::Get(int joint)
	{
		CNC_JOINT_CFG *jointConfig = CncGetJointCfg(joint);
		if (jointConfig == 0) return(eCncRC::CNC_RC_ERR);

		Name = jointConfig->name;
		Resolution = jointConfig->resolution;
		PositiveLimit = jointConfig->positiveLimit;
		NegativeLimit = jointConfig->negativeLimit;
		MaxVelocity = jointConfig->maxVelocity;
		MaxAcceleration = jointConfig->maxAcceleration;
		HomeVelocity = jointConfig->homeVelocity; 
		HomePosition = jointConfig->homePosition; 
		BackLash = jointConfig->backLash;
		LowSpeedJogPercent = jointConfig->lowSpeedJogPercent;
		MedSpeedJogPercent = jointConfig->medSpeedJogPercent;
		HighSpeedJogPercent = jointConfig->highSpeedJogPercent;

		return(eCncRC::CNC_RC_OK);
	}

	/// <summary>
	/// Set CncJointConfig
	/// </summary>
	/// <param name="cfg"> motor configuration </param>
	void CncJointConfig::Set(CNC_JOINT_CFG *cfg)
	{
		cfg->name = Name;
		cfg->resolution = Resolution;
		cfg->positiveLimit = PositiveLimit;
		cfg->negativeLimit = NegativeLimit;
		cfg->maxVelocity = MaxVelocity;
		cfg->maxAcceleration = MaxAcceleration;
		cfg->homeVelocity = HomeVelocity; 
		cfg->homePosition = HomePosition; 
		cfg->backLash = BackLash;
		cfg->lowSpeedJogPercent = LowSpeedJogPercent;
		cfg->medSpeedJogPercent = MedSpeedJogPercent;
		cfg->highSpeedJogPercent = HighSpeedJogPercent;
	}
};


/// <summary>
/// Tool description
/// </summary>
public ref class CncTool
{
public:
	/// <summary>
	/// Identifier
	/// </summary>
	int				id;
	/// <summary>
	/// Length or zOffset in application units
	/// </summary>
	double			zOffset;
	/// <summary>
	/// width or xOffset in application units
	/// </summary>
	double			xOffset;
	/// <summary>
	/// Diameter in application units
	/// </summary>
	double			diameter;
	/// <summary>
	/// Description of tool
	/// </summary>
	int 			orientation;
	/// <summary>
	/// Description of tool
	/// </summary>
	System::String^	description;
public:
	/// <summary>
	/// Initializes a new instance of CncTool (overloaded)
	/// </summary>
	CncTool(void)
	{
		id = 0;
		zOffset = 0.0;
		xOffset = 0.0;
		diameter = 0.0;
        orientation = 0;
		description = System::String::Empty;
	}

	/// <summary>
	/// Initializes a new instance of CncTool (overloaded)
	/// </summary>
	/// <param name="data">pointer to CNC_TOOL_TABLE from server</param>
	void Set(CNC_TOOL_DATA &data)
	{
		data.id = id;
		data.zOffset = zOffset;
		data.xOffset = xOffset;
		data.orientation = orientation;
		data.diameter = diameter;

		System::IntPtr p = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(description);
		char* pAnsi = static_cast<char*>(p.ToPointer());
		strncpy_s(data.description, sizeof(data.description), pAnsi, _TRUNCATE);
		System::Runtime::InteropServices::Marshal::FreeHGlobal(p);
	}

	
	/// <summary>
	/// Initializes a new instance of CncTool (overloaded)
	/// </summary>
	/// <param name="data">pointer to CNC_TOOL_TABLE from server</param>
	CncTool::CncTool(CNC_TOOL_DATA &data)
	{
		id = data.id;
		zOffset = data.zOffset;
		xOffset = data.xOffset;
        orientation = data.orientation;
		diameter = data.diameter;
		description = gcnew System::String(data.description);
	}

};

/// <summary>
/// Struct CNC_MACHINE_CONFIG
/// </summary>
/// <remarks>
/// Machine configuration data
/// </remarks>
public ref class CncMachineConfig
{
public:              
	/// <summary>                            
	/// Password for setup screen
	/// </summary>
	System::String^ setupPassword;

	/// <summary>                                                                        
	/// The language -1=eng, 0=duits. 1=ned, 2=italie, 3=fr 4=spanje 5=portugal
	/// </summary>                           
	eCncLangType language;                   

	/// <summary>                                                                        
	/// machineType, 0=milling, 1=turning
	/// </summary>                           
    eCncMachineType machineType;                   

	/// <summary>                                                                        
	/// diameterProgramming for turning if 1, 0 for radius programming or milling
	/// </summary>                           
    int diameterProgramming;          

	/// <summary>                                                                        
	/// center coordinates absolute if 1, relative if 0
	/// </summary>                           
    int absoluteCenterCoords;

	/// <summary>                                                                        
	/// SimpleZeroing without showing dialog for value.
	/// </summary>                           
    int simpleZeroing;


	/// <summary>                            
	//Name of com port e.g. "COM3"           
	/// </summary>
	System::String^ comPortName;	

	/// <summary>                            
	/// If 1, Ethernet is scanned for Ethernet CPU's 
	/// </summary>
	int scanEthernet;

	/// <summary>
	//Editor use for edit .cnc files
	/// </summary>
	System::String^ favoriteEditor;	

	/// <summary>
    //end of stroke sense level, used for homing 
	/// </summary>
    int endOfStrokeInputSenseLevel;

	/// <summary>
    //use only one home switch for all axes, the first - X-Axis
	/// </summary>
    int useXHomeinputForAllAxes;

	/// <summary>
    //switch on estop function on home sensors if all axes are homed */
	/// </summary>
    int homeIsEstopAfterHomingAllAxes;


	/// <summary>
    //emergency stop sense level
	/// </summary>
	int EStopInputSenseLevel1;
	int EStopInputSenseLevel2;

	/// <summary>
    //ext error input sense level
	/// </summary>
    int extErrorInputSenseLevel;

	/// <summary>
	//TRUE if axis is linear, only for A,B,C 
	/// </summary>
	array<int>^ abcAxisOption;
    
	/// <summary>
    //configuration for each joint
	/// </summary>
	array<CncJointConfig^>^ jointCfg;

	/// <summary>
    //trajectory interpolation time in seconds e.g. 0.01 second
	/// </summary>
    double interpolationTime;

	/// <summary>
	//The GUI automatically loads and runs this job file
	/// </summary>
	System::String^ watchLoadFileName;	
	
	/// <summary>
	//Tell the GUI to watch for file changed
	/// </summary>	int  watchFileChanged;
	int  watchFileChanged;
		
	/// <summary>
	//Tell the GUI to automatically load the changed file without dialog if file changed
	/// </summary>	int  watchAutoRun;
	int  watchAutoLoad;

	/// <summary>
	//Tell the GUI to automatically run the changed file if changed on disk without dialog
	///
	/// </summary>	int  watchAutoRun;
	int  watchAutoRun;

		/// <summary>
	/// Name of macro file, this file is always loaded together
	/// with job file
	/// </summary>
	System::String^ macroFile;	

	/// <summary>
	/// Initial executed interpreter string
	/// </summary>
	System::String^ initialString;	

	/// <summary>
	/// Invert X jog keys for moving machine bed
	/// </summary>
	int invertJogKeysX;

	/// <summary>
	/// Invert Y jog keys for moving machine bed
	/// </summary>
	int invertJogKeysY;

	/// <summary>
	/// Invert Z jog keys for moving machine bed
	/// </summary>
	int invertJogKeysZ;

	/// <summary>
	/// do not halt program on tool change request 
	/// </summary>
	int noHaltForToolChange;

	/// <summary>
	/// time to wait after spindle on, before moving 
	/// </summary>
	double spindleRampUpTime;
	int proportionalRampUpTime;

	/// <summary>
	/// max and min spindle revolutions/sec five 100% PWM output 
	/// </summary>
	double spindleNmax;
	double spindleNmin;

	/// <summary>
	/// if true, pwm value is programmed according to spindle speed 
	/// </summary>
	int spindleUseRPMSensor;

	/// <summary>
	/// if true, RPM value is showed in GUI
	/// </summary>
	int spindleShowRPM;

	/// <summary>
    /// show terms if not already shown 
	/// </summary>
    int showTerms;

	/// <summary>
    /// Homing mandatory, if 1, jog speed will be low and running program prohibited if not homed.
	/// </summary>
    int mandatoryHoming;

	/// <summary>
    /// show startup screen 
	/// </summary>
    int showStartupScreen;

	/// <summary>
	/// max step rate is controller baseFreq/controllerTimerDivider 
	/// </summary>
	int controllerTimerDividerIndex;

	/// <summary>
	/// 1 if height probe is present, 0 if not 
	/// </summary>
	int heightProbePresent;

	/// <summary>
	/// Offset for height probe 
	/// </summary>
	double heightProbeOffset;

	/// <summary>
    /// handwheel encoder count for one revolution 
	/// </summary>
    int handwheelCountPerRev;

	/// <summary>
	/// 1 for velocity mode, 0 for position mode 
	/// </summary>
	int handwheelX1VelMode;

	/// <summary>
	/// 1 for velocity mode, 0 for position mode
	/// </summary>
	int handwheelX10VelMode;

	/// <summary>
	/// 1 for velocity mode, 0 for position mode
	/// </summary>
	int handwheelX100VelMode;

	/// <summary>
	/// if TRUE, use handwheel for feedOV control
	/// </summary>
	int useHandWheelForFeedOverride; 

	/// <summary>
	/// if 1, probe touch points are stored in digitize file name 
	/// </summary>
	int storeProbeTouchPoints;

	/// <summary>
	/// file name to store probe touch points 
	/// </summary>
	System::String^ probeTouchPointFileName;	

	/// <summary>
	/// beep if probe triggers 
	/// </summary>
	int probeBeep;

	/// <summary>
    /// if TRUE, the lookahead feed trajectory generator is used 
	/// </summary>
    int useLAF;

	/// <summary>
    /// look ahead feed angle in degrees, connecting lines with 
    /// segments are considered straight if angle between them is less  
	/// </summary>
    double lafAngle;

   	/// <summary>
	/// file name of kinematics dll
	/// </summary>
	System::String^ kinematicsDLLName;	


public:
	CncMachineConfig()
	{
		setupPassword   			  = System::String::Empty;
		language                      = eCncLangType::CNC_LANG_ENGLISH;
        machineType                   = eCncMachineType::CNC_MACHINE_TYPE_MILLING;
        diameterProgramming           = 0;
        absoluteCenterCoords          = 0;
        simpleZeroing                 = 0;
		comPortName                   = System::String::Empty;
		favoriteEditor                = System::String::Empty;
		endOfStrokeInputSenseLevel    = 2;
		useXHomeinputForAllAxes       = 0;
        homeIsEstopAfterHomingAllAxes = 0;
		EStopInputSenseLevel1         = 2;
		EStopInputSenseLevel2         = 2;
		extErrorInputSenseLevel       = 2;
		abcAxisOption	              = gcnew array<int>(3);
		interpolationTime             = 0.01;
		watchLoadFileName             = System::String::Empty;
		watchFileChanged			  = 0;
		watchAutoLoad                 = 0;
		watchAutoRun                  = 0;
		macroFile		              = System::String::Empty;
		initialString			      = System::String::Empty;
		invertJogKeysX                = 0;
		invertJogKeysY                = 0;
		invertJogKeysZ                = 0;
		noHaltForToolChange           = 0;
		spindleRampUpTime             = 1.0;
		proportionalRampUpTime        = 0;
		spindleNmax                   = 10000;
		spindleNmin                   = 0;
		spindleUseRPMSensor           = 0;
		spindleShowRPM				  = 0;
		showTerms                     = 1;
        mandatoryHoming               = 1;
		showStartupScreen             = 1;
		controllerTimerDividerIndex   = 0;
		heightProbePresent            = 0;
		heightProbeOffset             = 0;
		handwheelCountPerRev          = 400;
		handwheelX1VelMode            = 1;
		handwheelX10VelMode           = 1;
		handwheelX100VelMode          = 1;
		useHandWheelForFeedOverride   = 1;
		storeProbeTouchPoints         = 0;
		probeTouchPointFileName       = System::String::Empty;
		probeBeep                     = 0;
		useLAF                        = 1;
		lafAngle                      = 1.0;
	}

public:
	CncMachineConfig(CNC_MACHINE_CONFIG &d)
	{
		setupPassword   			  = gcnew System::String(d.setupPassword);
		language                      = static_cast<eCncLangType>( (int) (d.language) );
        machineType                   = static_cast<eCncMachineType>(d.machineType);
        diameterProgramming           = d.diameterProgramming;
        absoluteCenterCoords          = d.absoluteCenterCoords;
        simpleZeroing                 = d.simpleZeroing;
		comPortName                   = gcnew System::String(d.comPortName);
		scanEthernet				  = d.scanEthernet;
		favoriteEditor                = gcnew System::String(d.favoriteEditor);                
		endOfStrokeInputSenseLevel    = d.endOfStrokeInputSenseLevel;    
		useXHomeinputForAllAxes       = d.useXHomeinputForAllAxes; 
        homeIsEstopAfterHomingAllAxes = d.homeIsEstopAfterHomingAllAxes;
		EStopInputSenseLevel1         = d.EStopInputSenseLevel1;          
		EStopInputSenseLevel2         = d.EStopInputSenseLevel2;          
		extErrorInputSenseLevel       = d.extErrorInputSenseLevel;       
		abcAxisOption	              = gcnew array<int>(3);;	              
		for (int i = 0; i < 3; i++)
			abcAxisOption[i] = d.abcAxisOption[i];
		interpolationTime             = d.interpolationTime;             
		watchLoadFileName	          = gcnew System::String(d.watchLoadFileName);	              
		watchFileChanged			  = d.watchFileChanged;
		watchAutoLoad                 = d.watchAutoLoad;
		watchAutoRun                  = d.watchAutoRun;
		macroFile		              = gcnew System::String(d.macroFile);		              
		initialString			      = gcnew System::String(d.initialString);			      
		invertJogKeysX                = d.invertJogKeysX;                
		invertJogKeysY                = d.invertJogKeysY;                
		invertJogKeysZ                = d.invertJogKeysZ;                
		noHaltForToolChange           = d.noHaltForToolChange;           
		spindleRampUpTime             = d.spindleCfg.rampUpTime;     
		proportionalRampUpTime        = d.spindleCfg.proportionalRampUpTime;
		spindleNmax                   = d.spindleCfg.Nmax;                   
		spindleNmin                   = d.spindleCfg.Nmin;                   
		spindleUseRPMSensor           = d.spindleCfg.useRPMSensor;    
		spindleShowRPM				  = d.spindleCfg.showRPM;
		showTerms                     = d.showTerms;                     
        mandatoryHoming               = d.mandatoryHoming;
		showStartupScreen             = d.showStartupScreen;             
		controllerTimerDividerIndex   = d.controllerTimerDividerIndex;        
		heightProbePresent            = d.heightProbePresent;            
		heightProbeOffset             = d.heightProbeOffset;             
		handwheelCountPerRev          = d.handwheelCountPerRev;          
		handwheelX1VelMode            = d.handwheelX1VelMode;              
		handwheelX10VelMode           = d.handwheelX10VelMode;             
		handwheelX100VelMode          = d.handwheelX100VelMode;             
		useHandWheelForFeedOverride   = d.useHandWheelForFeedOverride;
		storeProbeTouchPoints         = d.storeProbeTouchPoints;         
		probeTouchPointFileName       = gcnew System::String(d.probeTouchPointFileName);       
		probeBeep                     = d.probeBeep;                     
		useLAF                        = d.useLAF;                        
		lafAngle                      = d.lafAngle;    
        kinematicsDLLName             = gcnew System::String(d.kinCfg.kinematicsDLLName);  
	}

	void Set(CNC_MACHINE_CONFIG &d)
	{
		int i = 0;


		d.language = static_cast<CNC_LANG_T>( (int) (language) );

		System::IntPtr p = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(comPortName);
		char* pAnsi = static_cast<char*>(p.ToPointer());
		strncpy_s(d.comPortName, sizeof(d.comPortName), pAnsi, _TRUNCATE);
		System::Runtime::InteropServices::Marshal::FreeHGlobal(p);

		d.scanEthernet = scanEthernet;

		p = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(setupPassword);
		pAnsi = static_cast<char*>(p.ToPointer());
		strncpy_s(d.setupPassword, sizeof(d.setupPassword), pAnsi, _TRUNCATE);
		System::Runtime::InteropServices::Marshal::FreeHGlobal(p);

		p = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(favoriteEditor);
		pAnsi = static_cast<char*>(p.ToPointer());
		strncpy_s(d.favoriteEditor, sizeof(d.favoriteEditor), pAnsi, _TRUNCATE);
		System::Runtime::InteropServices::Marshal::FreeHGlobal(p);

        d.machineType                   = static_cast<CNC_MACHINE_TYPE>(machineType);
        d.diameterProgramming           = diameterProgramming;
        d.simpleZeroing                 = simpleZeroing;
        d.absoluteCenterCoords          = absoluteCenterCoords;
		d.useXHomeinputForAllAxes       = useXHomeinputForAllAxes;
		d.homeIsEstopAfterHomingAllAxes = homeIsEstopAfterHomingAllAxes;
		d.EStopInputSenseLevel2         = EStopInputSenseLevel2;
		d.EStopInputSenseLevel2         = EStopInputSenseLevel2;
		d.extErrorInputSenseLevel       = extErrorInputSenseLevel;

		for (i = 0; i < 3; i++)
			d.abcAxisOption[i] = static_cast<CNC_ROT_AXIS_OPTION>(abcAxisOption[i]);

		d.interpolationTime             = 0.01;

		p = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(watchLoadFileName);
		pAnsi = static_cast<char*>(p.ToPointer());
		strncpy_s(d.watchLoadFileName, sizeof(d.watchLoadFileName), pAnsi, _TRUNCATE);
		System::Runtime::InteropServices::Marshal::FreeHGlobal(p);
		d.watchAutoLoad = watchAutoLoad;
		d.watchAutoRun = watchAutoRun;
		d.watchFileChanged = watchFileChanged;

		p = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(macroFile);
		pAnsi = static_cast<char*>(p.ToPointer());
		strncpy_s(d.macroFile, sizeof(d.macroFile), pAnsi, _TRUNCATE);
		System::Runtime::InteropServices::Marshal::FreeHGlobal(p);

		p = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(initialString);
		pAnsi = static_cast<char*>(p.ToPointer());
		strncpy_s(d.initialString, sizeof(d.initialString), pAnsi, _TRUNCATE);
		System::Runtime::InteropServices::Marshal::FreeHGlobal(p);

		d.invertJogKeysX                = invertJogKeysX;
		d.invertJogKeysY                = invertJogKeysY;
		d.invertJogKeysZ                = invertJogKeysZ;


		d.noHaltForToolChange           = noHaltForToolChange;
		d.spindleCfg.rampUpTime             = spindleRampUpTime;
		d.spindleCfg.proportionalRampUpTime        = proportionalRampUpTime;
		d.spindleCfg.Nmax                   = spindleNmax;
		d.spindleCfg.Nmin					= spindleNmin;
		d.spindleCfg.useRPMSensor           = spindleUseRPMSensor;
		d.spindleCfg.showRPM				= spindleShowRPM;
		d.showTerms                     = showTerms;
        d.mandatoryHoming               = mandatoryHoming;
		d.showStartupScreen             = showStartupScreen;
		d.controllerTimerDividerIndex   = controllerTimerDividerIndex;
		d.heightProbePresent            = heightProbePresent;
		d.heightProbeOffset             = heightProbeOffset;
		d.handwheelCountPerRev          = handwheelCountPerRev;
		d.handwheelX1VelMode             = handwheelX1VelMode;
		d.handwheelX10VelMode           = handwheelX10VelMode;
		d.handwheelX100VelMode          = handwheelX100VelMode;
		d.useHandWheelForFeedOverride   = useHandWheelForFeedOverride;
		d.storeProbeTouchPoints         = storeProbeTouchPoints;

		p = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(probeTouchPointFileName);
		pAnsi = static_cast<char*>(p.ToPointer());
		strncpy_s(d.probeTouchPointFileName, sizeof(d.probeTouchPointFileName), pAnsi, _TRUNCATE);
		System::Runtime::InteropServices::Marshal::FreeHGlobal(p);

		p = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(kinematicsDLLName);
		pAnsi = static_cast<char*>(p.ToPointer());
		strncpy_s(d.kinCfg.kinematicsDLLName, sizeof(d.kinCfg.kinematicsDLLName), pAnsi, _TRUNCATE);
		System::Runtime::InteropServices::Marshal::FreeHGlobal(p);

		d.probeBeep                     = probeBeep;
		d.useLAF                        = useLAF;
		d.lafAngle                      = lafAngle;
	}    
};       
         
         
         
/// <summary>
/// CncIOPort
/// </summary>
/// <remarks>
/// IO Port status
/// </remarks>
public ref class CncIOPort
{
public:
    eCncIOID       ioId;	//The ID
	int			   invert;  //Actually configuration item
    int            value;   //the current value

public:
	CncIOPort()
	{
		ioId = eCncIOID::CNC_IOID_NONE;
		invert = 0;
		value = 0;
	}

	CncIOPort(CNC_IO_PORT_STS &d)
	{
		ioId = static_cast<eCncIOID>( (int) (d.ioId) );
		invert = d.invert;
		value = d.lvalue;
	}
};


/// <summary>
/// Structure holding joint status data
/// </summary>
public ref class CncJointStatus
{    
public:
	/// <summary>
	/// joint index starting from 0
	/// </summary>
	int   jointIndex;
	/// <summary>
	/// joint state
	/// </summary>
	eCncJointState state;
	/// <summary>
	/// error word
	/// </summary>
	int            errorWord;
	/// <summary>
	/// position
	/// </summary>
	double          position; 
	/// <summary>
	/// maximum position error
	/// </summary>
	double          maxPositionError ;
	/// <summary>
	/// is joint homed
	/// </summary>
	bool isHomed;
	/// <summary>
	/// is joint homed
	/// </summary>
    bool homeSensorStatus;
public:
	/// <summary>
	/// Initializes a new instance of CncJointStatus
	/// </summary>
	CncJointStatus::CncJointStatus(void)
	{
		jointIndex = 0;
		state = static_cast<eCncJointState>( (int) CNC_JOINT_ERROR_STATE );
		errorWord = 0;
		position = 0.0;
		maxPositionError = 0.0;
		isHomed = false;
        homeSensorStatus = false;
	}

	/// <summary>
	/// Initializes a new instance of CncJointStatus
	/// </summary>
	CncJointStatus::CncJointStatus(CNC_JOINT_STS &d)
	{
		jointIndex = d.jointIndex;
		state = static_cast<eCncJointState>( (int) (d.state) );
		errorWord = d.errorWord;
		position = d.position;
		maxPositionError = d.maxPositionError;
		isHomed = d.isHomed != 0;
        homeSensorStatus = d.homeSensorStatus != 0;
	}
};




/// <summary>
/// Struct CNC_TIME_STAMP
/// </summary>
/// <remarks>
/// Time stamp for hi res time measurements
/// </remarks>
public ref class CncTimeStamp
{
public:
    long long startTime; 
    long long timeStamp;
};

/// <summary>
/// Struct CNC_POS_FIFO_DATA
/// </summary>
/// <remarks>
/// Real time position data
/// </remarks>
public ref class CncPosFifoData
{
public:
	double x,y,z,a,b,c;     // x,y,z position vector 
	eCncMoveType type;

public:
	CncPosFifoData()
	{
		x = y = z = a = b = c = 0.0;
	}

	CncPosFifoData(CNC_POS_FIFO_DATA &d)
	{
		x = d.pos.x;
		y = d.pos.y;
		z = d.pos.z;
		a = d.pos.a;
		b = d.pos.b;
		c = d.pos.c;
		type = static_cast<eCncMoveType>( (int) (d.type) );
        
	}
};

/// <summary>
/// Struct CNC_GRAPH_FIFO_DATA
/// </summary>
/// <remarks>
/// Non Real time position data
/// </remarks>
public ref class CncGraphFifoData
{
public:
   	eCncMoveType  type;    /* see GRAPH type         */
	CncPosition   pos;     /* end position, origin offset, start pos, tool offset, depending on type */
	int	turn;				/* for arc direction & n of turns */


public:
	CncGraphFifoData()
	{
        type = eCncMoveType::CNC_MOVE_TYPE_END;
        pos.x = pos.y = pos.z = pos.a = pos.b = pos.c = 0.0;
	}

	CncGraphFifoData(CNC_GRAPH_FIFO_DATA &d)
	{
        type = static_cast<eCncMoveType>( (int) (d.type) );
		pos.x = d.pos.x;
		pos.y = d.pos.y;
		pos.z = d.pos.z;
		pos.a = d.pos.a;
		pos.b = 0;
		pos.c = 0;
	}
};

#endif
