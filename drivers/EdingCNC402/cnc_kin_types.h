#ifndef _CNC_KIN_TYPES_H_
#define _CNC_KIN_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

	/* universal type to be used with
	 * KinControl Function
	 */
	typedef union _KIN_CONTROL_DATA
	{
		double dData[12];
		int    iData[12];
		char   cData[64];

	} KIN_CONTROLDATA;

	typedef enum _KIN_CONTROL_ID
	{
		CNC_KIN_CONTROL_ID_OPEN  = 1,    /* Used during initialization of the CNC */
		CNC_KIN_CONTROL_ID_CLOSE = 2,    /* Used during shutdown of the CNC       */
		CNC_KIN_CONTROL_ID_OFF   = 3,    /* Kinematics switched off			    */
		CNC_KIN_CONTROL_ID_ON    = 4,    /* Kinematics switched on				*/
		KIN_CONTROL_ID_RES1  = 5,        /* System reserved        				*/
		KIN_CONTROL_ID_RES2  = 6,        /* System reserved        				*/
		KIN_CONTROL_ID_RES3  = 7,        /* System reserved        				*/
		KIN_CONTROL_ID_RES4  = 8,        /* System reserved        				*/
		KIN_CONTROL_ID_JOINT_LIMITS  = 9,/* System reserved        				*/
		KIN_CONTROL_ID_KIN_LIMITS  = 10, /* System reserved        				*/
		KIN_CONTROL_ID_USER1 = 11,       /* Kinematics specific control ID		*/
		KIN_CONTROL_ID_USER2 = 12,       /* Kinematics specific control ID		*/
		KIN_CONTROL_ID_USER3 = 13,       /* Kinematics specific control ID		*/
		KIN_CONTROL_ID_USER4 = 14,       /* Kinematics specific control ID		*/
		KIN_CONTROL_ID_USER5 = 15,       /* Kinematics specific control ID		*/
		KIN_CONTROL_ID_USER6 = 16,       /* Kinematics specific control ID		*/
	} KIN_CONTROL_ID;


    /* the forward flags are passed to the forward kinematics so that they
       can resolve ambiguities in the world coordinates for a given joint set,
       e.g., for hexpods, this would be platform-below-base, platform-above-base.

       The flags are also passed to the inverse kinematics and are set by them,
       which is how they are changed from their initial value. For example, for
       hexapods you could do a coordinated move that brings the platform up from
       below the base to above the base. The forward flags would be set to
       indicate this. */
    typedef unsigned int CNC_KINEMATICS_FORWARD_FLAGS;

    /* the inverse flags are passed to the inverse kinematics so thay they
       can resolve ambiguities in the joint angles for a given world coordinate,
       e.g., for robots, this would be elbow-up, elbow-down, etc.

       The flags are also passed to the forward kinematics and are set by them,
       which is how they are changed from their initial value. For example, for
       robots you could do a joint move that brings the elbow from a down
       configuration to an up configuration. The inverse flags would be set to
       indicate this. */
    typedef unsigned int CNC_KINEMATICS_INVERSE_FLAGS;


    /* the type of kinematics used, these are common for milling machines */
    typedef enum CNC_KINEMATICS_TYPE
	{
		CNC_KINEMATICS_TYPE_UNDEFINED       = 0,
        CNC_KINEMATICS_TYPE_TRIVIAL			= 1,  /* 1:1 kins                                */	
		CNC_KINEMATICS_TYPE_4AX_A_CILINDER  = 2,  /* Y mapped to rotate A                    */ 
		CNC_KINEMATICS_TYPE_VIRTUAL_C       = 3,  /* Virtual C axis rotates XY               */
		CNC_KINEMATICS_TYPE_LIN_DELTA    	= 4,  /* Linear delta robot                      */
		CNC_KINEMATICS_TYPE_RESERVED_13    	= 5,  /* Reserved for build in kins              */
		CNC_KINEMATICS_TYPE_RESERVED_12    	= 6,  /* Reserved for build in kins              */	
		CNC_KINEMATICS_TYPE_RESERVED_11    	= 7,  /* Reserved for build in kins              */
		CNC_KINEMATICS_TYPE_RESERVED_10    	= 8,  /* Reserved for build in kins              */
		CNC_KINEMATICS_TYPE_RESERVED_9    	= 9,  /* Reserved for build in kins              */
		CNC_KINEMATICS_TYPE_RESERVED_8    	= 10, /* Reserved for build in kins              */
		CNC_KINEMATICS_TYPE_RESERVED_7    	= 11, /* Reserved for build in kins              */
		CNC_KINEMATICS_TYPE_RESERVED_6    	= 12, /* Reserved for build in kins              */
		CNC_KINEMATICS_TYPE_RESERVED_5    	= 13, /* Reserved for build in kins              */
		CNC_KINEMATICS_TYPE_RESERVED_4    	= 14, /* Reserved for build in kins              */
		CNC_KINEMATICS_TYPE_RESERVED_3    	= 15, /* Reserved for build in kins              */
		CNC_KINEMATICS_TYPE_RESERVED_2    	= 16, /* Reserved for build in kins              */
		CNC_KINEMATICS_TYPE_RESERVED_1    	= 17, /* Reserved for build in kins              */
		CNC_KINEMATICS_CUSTOM_1		        = 18, /* reservation for custom kinematics types */
		CNC_KINEMATICS_CUSTOM_2		        = 19, /* reservation for custom kinematics types */
		CNC_KINEMATICS_CUSTOM_3		        = 20, /* reservation for custom kinematics types */
		CNC_KINEMATICS_CUSTOM_4		        = 21, /* reservation for custom kinematics types */
		CNC_KINEMATICS_CUSTOM_5		        = 22, /* reservation for custom kinematics types */
		CNC_KINEMATICS_CUSTOM_6		        = 23, /* reservation for custom kinematics types */
		CNC_KINEMATICS_CUSTOM_7		        = 24, /* reservation for custom kinematics types */
		CNC_KINEMATICS_CUSTOM_8		        = 25, /* reservation for custom kinematics types */
		CNC_KINEMATICS_CUSTOM_9		        = 26, /* reservation for custom kinematics types */
		CNC_KINEMATICS_CUSTOM_10	        = 27, /* reservation for custom kinematics types */
		CNC_KINEMATICS_CUSTOM_11	        = 28, /* reservation for custom kinematics types */
		CNC_KINEMATICS_CUSTOM_12	        = 30  /* reservation for custom kinematics types */
    } CNC_KINEMATICS_TYPE;

	/* Forward kins, convert joint positions to Cartesian position */
    typedef int (__stdcall* CncKinematicsForwardType)(CNC_JOINT_DOUBLE jointPos,
                                                      CNC_CART_DOUBLE *cartPos, 
                                                      const CNC_KINEMATICS_FORWARD_FLAGS *fflags,
                                                      CNC_KINEMATICS_INVERSE_FLAGS *iflags,
                                                      double tl);

	/* Inverse kins, convert Cartesian position to joint positions */
	typedef int (__stdcall* CncKinematicsInverseType) (CNC_CART_DOUBLE cartPos, 
													CNC_JOINT_DOUBLE *jointPos,
													const CNC_KINEMATICS_INVERSE_FLAGS *iflags,
													CNC_KINEMATICS_FORWARD_FLAGS *fflags,
													double tl);

	/* Inverse Kins, calculates max allowable Cartesian velocity and acceleration given start/end pos */
    typedef int  (_stdcall* CncKinematicsInverseMaxFeedType) (  CNC_CART_DOUBLE startPos,
                                                        CNC_CART_DOUBLE endPos, 
                                                        CNC_JOINT_DOUBLE *joints,
                                                        double *maxTrajectVelocity, 
                                                        double *maxTrajectAcceleration, 
                                                        double toolLength);

	/* Communication with kinematics, required with some particular kinematics type */
	typedef int (__stdcall* CncKinematicsControlType)(KIN_CONTROL_ID controlID, KIN_CONTROLDATA *controlData);

	/* Get the kinematics type */
	typedef CNC_KINEMATICS_TYPE (__stdcall* CncKinematicsTypeType)(void);

	/* Get kinematics version string */
	typedef char * (__stdcall* CncKinematicsVersionType)(void);

#pragma pack()


#ifdef __cplusplus
}
#endif

#endif










