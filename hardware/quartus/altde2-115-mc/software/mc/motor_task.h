/*
 * motor_task.h
 *
 *  Created on: 05/03/2015
 *      Author: f07112
 */
// File added 2015-03-05 km

#ifndef __MOTOR_TASK_H_
#define __MOTOR_TASK_H_

#include "includes.h"

// For litle endian
//#define SYSID_VERSION_MAJOR(s) ((s & 0x00F00000)>>20)      //!< DOC hardware Major version from QSYS SYSID
//#define SYSID_VERSION_MINOR(s) ((s & 0x000F0000)>>16)      //!< DOC hardware Minor version from QSYS SYSID
//#define SYSID_POWERBOARD_ID(s) ((s & 0x0000F000)>>12)      //!< DOC power board identifier from QSYS SYSID
//#define SYSID_DEVICE_FAMILY(s) ((s & 0x00000F00)>>8)       //!< DOC device family identifier from QSYS SYSID
//#define SYSID_DESIGN_ID(s)     ((s & 0x000000FF))          //!< DOC identifier from QSYS SYSID

// For big endian
#define SYSID_VERSION_MAJOR(s) ((s & 0x0000F000)>>12)      //!< DOC hardware Major version from QSYS SYSID
#define SYSID_VERSION_MINOR(s) ((s & 0x00000F00)>>8)      //!< DOC hardware Minor version from QSYS SYSID
#define SYSID_POWERBOARD_ID(s) ((s & 0x00F00000)>>20)      //!< DOC power board identifier from QSYS SYSID
#define SYSID_DEVICE_FAMILY(s) ((s & 0x000F0000)>>16)       //!< DOC device family identifier from QSYS SYSID
#define SYSID_DESIGN_ID(s)     ((s & 0xFF000000)>>24)          //!< DOC identifier from QSYS SYSID


#define SYSID_UNKNOWN    -1

#define	SYSID_CIV_DE2115 0		//!< Cyclone IV On DE2-115 INK
#define	SYSID_CVE        1 		//!< Cyclone VE dev kit
#define	SYSID_CVSX       2 		//!< Cyclone VSX dev kit

//km #define	SYSID_PB_FALCONEYE_1      0	//!< FalconEye 1
//km #define	SYSID_PB_FALCONEYE_2_HSMC 1	//!< FalconEye 2 with HSMC adapter
//km #define	SYSID_PB_FALCONEYE_EBV    2	//!< FalconEye 2 on MercuryCode
#define	SYSID_PB_ALT12_MULTIAXIS  3	//!< Altera ALT12 multi axis power board

//km #define	SYSID_ENCODER_ENDAT		 0	//!< EnDat encoder
#define	SYSID_ENCODER_BISS		 1	//!< BiSS encoder
//km #define	SYSID_ENCODER_QUADRATURE 2	//!< Quadrature encoder
//km #define	SYSID_ENCODER_HALL		 3	//!< Hall effect encoder
//km #define	SYSID_ENCODER_RESOLVER	 4	//!< Resolver

typedef struct {
	int    sysid;				//!< Identifier form Qsys SYSID
	char * name;				//!< Descriptive name
} device_family_t;

typedef struct {
	int          sysid;			//!< Identifier form Qsys SYSID
	char *       name;			//!< Descriptive name
	unsigned int axes;			//!< Number of axes supported
	unsigned int first_axis;	//!< Lowest operational axis
	unsigned int last_axis;		//!< Highest operational axis
	unsigned int encoder;		//!< Default encoder type
	unsigned int req_family;	//!< Bit vector of required device family
	unsigned int undervoltage;	//!< DC link undervoltage limit (V)
	unsigned int overvoltage;	//!< DC link overvoltage limit (V)
} powerboard_t;

typedef struct {
	int    sysid;				//!< Identifier form Qsys SYSID
	char * name;				//!< Descriptive name
	int (*encoder_init_fn)(drive_params * dp);				//!< Pointer to encoder initialisation function
	void (*encoder_service_fn)(drive_params * dp);			//!< Pointer to encoder service function
	void (*encoder_read_position_fn)(drive_params * dp);	//!< Pointer to encoder position read function
} encoder_t;

typedef struct {
	device_family_t * device_family;		//!< Device family decoded from Qsys SYSID
	powerboard_t    * powerboard;			//!< Power board decoded from Qsys SYSID
	encoder_t       * encoder;				//!< Encoder type decoded from Qsys SYSID

	unsigned int      version_major;		//!< Qsys project major version number
	unsigned int      version_minor;		//!< Qsys project minor version number

	unsigned int      design_id;			//!< Magic number from Qsys SYSID
	unsigned int      first_drive;			//!< First active axis <= last_drive <= max_channel
	unsigned int      last_drive;			//!< Last active axis <= max_channel
} platform_t;

extern platform_t platform;

#endif /* __MOTOR_TASK_H_ */
