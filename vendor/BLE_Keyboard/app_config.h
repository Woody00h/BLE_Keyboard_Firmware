#pragma once

/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif



#if (__PROJECT_8261_BLE_REMOTE__)
	#define CHIP_TYPE				CHIP_TYPE_8261
#elif (__PROJECT_8266_BLE_REMOTE__)
	#define CHIP_TYPE				CHIP_TYPE_8266
#elif (__PROJECT_8267_BLE_REMOTE__)
	#define CHIP_TYPE				CHIP_TYPE_8267
#elif (__PROJECT_8269_BLE_REMOTE__)
	#define CHIP_TYPE				CHIP_TYPE_8269
#elif (__PROJECT_BLE_KEYBOARD__)
	#define CHIP_TYPE				CHIP_TYPE_8269
#else

#endif



/////////////////// MODULE /////////////////////////////////
#define BLE_REMOTE_PM_ENABLE			1
#define BLE_REMOTE_SECURITY_ENABLE      1
#define BLE_REMOTE_OTA_ENABLE			1
#define REMOTE_IR_ENABLE				0
#define BATT_CHECK_ENABLE       		0   //enable or disable battery voltage detection
#define BLE_PHYTEST_MODE				PHYTEST_MODE_DISABLE

#if (__PROJECT_8267_BLE_REMOTE__ || __PROJECT_8269_BLE_REMOTE__)
	#define BLE_AUDIO_ENABLE				1
#else   //8261/8266  not support audio
	#define BLE_AUDIO_ENABLE				0
#endif




////////////////////////// AUDIO CONFIG /////////////////////////////
#if (BLE_AUDIO_ENABLE)
	#define BLE_DMIC_ENABLE					0  //0: Amic   1: Dmic
	#define	ADPCM_PACKET_LEN				128
	#define TL_MIC_ADPCM_UNIT_SIZE			248

	#define	TL_MIC_32K_FIR_16K				1

	#if TL_MIC_32K_FIR_16K
		#define	TL_MIC_BUFFER_SIZE				1984
	#else
		#define	TL_MIC_BUFFER_SIZE				992
	#endif

	#define GPIO_AMIC_BIAS					GPIO_PC6

#endif

///////////// avoid ADC module current leakage (when module on suspend status) //////////////////////////////
#define ADC_MODULE_CLOSED               BM_CLR(reg_adc_mod, FLD_ADC_CLK_EN)  // adc clk disable
#define ADC_MODULE_ENABLE               BM_SET(reg_adc_mod, FLD_ADC_CLK_EN) // adc clk open

//////////////////////////// KEYSCAN/MIC  GPIO //////////////////////////////////
#define	MATRIX_ROW_PULL					PM_PIN_PULLDOWN_100K
#define	MATRIX_COL_PULL					PM_PIN_PULLUP_10K

#define	KB_LINE_HIGH_VALID				0   //dirve pin output 0 when keyscan, scanpin read 0 is valid
#define DEEPBACK_FAST_KEYSCAN_ENABLE	1   //proc fast scan when deepsleep back trigged by key press, in case key loss
#define KEYSCAN_IRQ_TRIGGER_MODE		1
#define LONG_PRESS_KEY_POWER_OPTIMIZE	1   //lower power when pressing key without release

//stuck key
#define STUCK_KEY_PROCESS_ENABLE		0
#define STUCK_KEY_ENTERDEEP_TIME		60  //in s

//repeat key
#define KB_REPEAT_KEY_ENABLE			0
#define	KB_REPEAT_KEY_INTERVAL_MS		200
#define KB_REPEAT_KEY_NUM				1
#define KB_MAP_REPEAT					{VK_1, }

//modifier
#define VK_L_CTRL	0xF1
#define VK_L_SHIFT	0xF2
#define VK_L_ALT	0xF3
#define VK_L_WIN	0xF4
#define VK_R_CTRL	0xF5
#define VK_R_SHIFT	0xF6
#define VK_R_ALT	0xF7
#define VK_R_WIN	0xF8
#define KEYBOARD_MODIFIER_BITS	{0, 1, 2, 4, 8, 16, 32, 64, 128}

#define		KB_MAP_NORMAL	{\
				VK_ESC,		VK_F1,			VK_F2,		VK_F3,			VK_F4,			VK_F5,		VK_F6,			VK_F7,		VK_F8,		VK_F9,\
				VK_NUMBER,	VK_1,			VK_2,		VK_3,			VK_4,			VK_5,		VK_6,			VK_7,		VK_8,		VK_9,\
				VK_TAB,		VK_Q,			VK_W,		VK_E,			VK_R,			VK_T,		VK_Y, 			VK_U,		VK_I,		VK_O,\
				VK_CAPITAL,	VK_A,			VK_S,		VK_D,			VK_F,			VK_G,		VK_H, 			VK_J,		VK_K,		VK_L,\
				VK_L_SHIFT,	VK_Z,			VK_X,		VK_C,			VK_V,			VK_B,		VK_N,			VK_COMMA,	VK_M,		VK_PERIOD,\
				VK_L_CTRL,	VK_L_WIN,		VK_L_ALT,	VK_SPACE,		VK_R_ALT,		VK_R_WIN,	VK_FN,			VK_LEFT, 	VK_R_CTRL,	VK_DOWN,\
				VK_F10,		VK_F11,			VK_F12,		VK_PRINTSCREEN,	VK_SCR_LOCK,	VK_PAUSE,	VK_SLASH,		VK_ENTER,	VK_R_SHIFT,	VK_RIGHT,\
				VK_P,		VK_LBRACE,		VK_RBRACE,	VK_BACKSLASH,	VK_DELETE,		VK_END,		VK_PAGE_DOWN,	VK_UP,		VK_QUOTE,	VK_L,\
				VK_0,		VK_MINUS,		VK_PLUS,	VK_BACKSPACE,	VK_INSERT,		VK_HOME,	VK_PAGE_UP,		VK_NONE,	VK_NONE,	VK_NONE,\
}

#endif  //end of REMOTE_IR_ENABLE

//col
#define  KB_DRIVE_PINS  {GPIO_PB4, GPIO_PB5, GPIO_PB6, GPIO_PB7, GPIO_PC4, GPIO_PA2, GPIO_PA3, GPIO_PA0, GPIO_PA4, GPIO_PD7}
//row
#define  KB_SCAN_PINS   {GPIO_PC3, GPIO_PB3, GPIO_PB2, GPIO_PB1, GPIO_PA5, GPIO_PD6, GPIO_PD5, GPIO_PA1, GPIO_PC5}

#define	PB4_FUNC				AS_GPIO
#define	PB5_FUNC				AS_GPIO
#define	PB6_FUNC				AS_GPIO
#define	PB7_FUNC				AS_GPIO
#define	PC4_FUNC				AS_GPIO
#define	PA2_FUNC				AS_GPIO
#define	PA3_FUNC				AS_GPIO
#define	PA0_FUNC				AS_GPIO
#define	PA4_FUNC				AS_GPIO
#define	PD7_FUNC				AS_GPIO
//drive pin need 100K pulldown
#define	PULL_WAKEUP_SRC_PB4		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PB5		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PB6		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PB7		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PC4		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PA2		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PA3		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PA0		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PA4		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PD7		MATRIX_ROW_PULL
//drive pin open input to read gpio wakeup level
#define PB4_INPUT_ENABLE		1
#define PB5_INPUT_ENABLE		1
#define PB6_INPUT_ENABLE		1
#define PB7_INPUT_ENABLE		1
#define PC4_INPUT_ENABLE		1
#define PA2_INPUT_ENABLE		1
#define PA3_INPUT_ENABLE		1
#define PA0_INPUT_ENABLE		1
#define PA4_INPUT_ENABLE		1
#define PD7_INPUT_ENABLE		1

#define	PC3_FUNC				AS_GPIO
#define	PB3_FUNC				AS_GPIO
#define	PB2_FUNC				AS_GPIO
#define	PB1_FUNC				AS_GPIO
#define	PA5_FUNC				AS_GPIO
#define	PD6_FUNC				AS_GPIO
#define	PD5_FUNC				AS_GPIO
#define	PA1_FUNC				AS_GPIO
#define	PC5_FUNC				AS_GPIO
//scan  pin need 10K pullup
#define	PULL_WAKEUP_SRC_PC3		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PB3		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PB2		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PB1		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PA5		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PD6		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PD5		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PA1		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PC5		MATRIX_COL_PULL
//scan pin open input to read gpio level
#define PC3_INPUT_ENABLE		1
#define PB3_INPUT_ENABLE		1
#define PB2_INPUT_ENABLE		1
#define PB1_INPUT_ENABLE		1
#define PA5_INPUT_ENABLE		1
#define PD6_INPUT_ENABLE		1
#define PD5_INPUT_ENABLE		1
#define PA1_INPUT_ENABLE		1
#define PC5_INPUT_ENABLE		1





#define		KB_MAP_NUM		KB_MAP_NORMAL
#define		KB_MAP_FN		KB_MAP_NORMAL


/////////////////// Clock  /////////////////////////////////
#define CLOCK_SYS_TYPE  		CLOCK_TYPE_PLL	//  one of the following:  CLOCK_TYPE_PLL, CLOCK_TYPE_OSC, CLOCK_TYPE_PAD, CLOCK_TYPE_ADC
#define CLOCK_SYS_CLOCK_HZ  	16000000

//////////////////Extern Crystal Type///////////////////////
#define CRYSTAL_TYPE			XTAL_12M		//  extern 12M crystal


/////////////////// watchdog  //////////////////////////////
#define MODULE_WATCHDOG_ENABLE		0
#define WATCHDOG_INIT_TIMEOUT		500  //ms








///////////////////////////////////// ATT  HANDLER define ///////////////////////////////////////
typedef enum
{
	ATT_H_START = 0,


	//// Gap ////
	/**********************************************************************************************/
	GenericAccess_PS_H, 					//UUID: 2800, 	VALUE: uuid 1800
	GenericAccess_DeviceName_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	GenericAccess_DeviceName_DP_H,			//UUID: 2A00,   VALUE: device name
	GenericAccess_Appearance_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read
	GenericAccess_Appearance_DP_H,			//UUID: 2A01,	VALUE: appearance
	CONN_PARAM_CD_H,						//UUID: 2803, 	VALUE:  			Prop: Read
	CONN_PARAM_DP_H,						//UUID: 2A04,   VALUE: connParameter

	
	//// gatt ////
	/**********************************************************************************************/
	GenericAttribute_PS_H,					//UUID: 2800, 	VALUE: uuid 1801
	GenericAttribute_ServiceChanged_CD_H,	//UUID: 2803, 	VALUE:  			Prop: Indicate
	GenericAttribute_ServiceChanged_DP_H,   //UUID:	2A05,	VALUE: service change
	GenericAttribute_ServiceChanged_CCB_H,	//UUID: 2902,	VALUE: serviceChangeCCC
	

	//// device information ////
	/**********************************************************************************************/
	DeviceInformation_PS_H,					//UUID: 2800, 	VALUE: uuid 180A
	DeviceInformation_pnpID_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read
	DeviceInformation_pnpID_DP_H,			//UUID: 2A50,	VALUE: PnPtrs
	

	//// HID ////
	/**********************************************************************************************/
	HID_PS_H, 								//UUID: 2800, 	VALUE: uuid 1812

	//include
	HID_INCLUDE_H,							//UUID: 2802, 	VALUE: include

	//protocol
	HID_PROTOCOL_MODE_CD_H,					//UUID: 2803, 	VALUE:  			Prop: read | write_without_rsp	
	HID_PROTOCOL_MODE_DP_H,					//UUID: 2A4E,	VALUE: protocolMode
	
	//boot keyboard input report
	HID_BOOT_KB_REPORT_INPUT_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	HID_BOOT_KB_REPORT_INPUT_DP_H,			//UUID: 2A22, 	VALUE: bootKeyInReport
	HID_BOOT_KB_REPORT_INPUT_CCB_H,			//UUID: 2902, 	VALUE: bootKeyInReportCCC

	//boot keyboard output report
	HID_BOOT_KB_REPORT_OUTPUT_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read | write| write_without_rsp
	HID_BOOT_KB_REPORT_OUTPUT_CCB_H,		//UUID: 2A32, 	VALUE: bootKeyOutReport

	//consume report in
	HID_CONSUME_REPORT_INPUT_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	HID_CONSUME_REPORT_INPUT_DP_H,			//UUID: 2A4D, 	VALUE: reportConsumerIn
	HID_CONSUME_REPORT_INPUT_CCB_H,			//UUID: 2902, 	VALUE: reportConsumerInCCC
	HID_CONSUME_REPORT_INPUT_REF_H, 		//UUID: 2908    VALUE: REPORT_ID_CONSUMER, TYPE_INPUT
	
	//keyboard report in
	HID_NORMAL_KB_REPORT_INPUT_CD_H,		//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	HID_NORMAL_KB_REPORT_INPUT_DP_H,		//UUID: 2A4D, 	VALUE: reportKeyIn
	HID_NORMAL_KB_REPORT_INPUT_CCB_H,		//UUID: 2902, 	VALUE: reportKeyInInCCC
	HID_NORMAL_KB_REPORT_INPUT_REF_H, 		//UUID: 2908    VALUE: REPORT_ID_KEYBOARD, TYPE_INPUT

	//keyboard report out
	HID_NORMAL_KB_REPORT_OUTPUT_CD_H,		//UUID: 2803, 	VALUE:  			Prop: Read | write| write_without_rsp
	HID_NORMAL_KB_REPORT_OUTPUT_DP_H,  		//UUID: 2A4D, 	VALUE: reportKeyOut
	HID_NORMAL_KB_REPORT_OUTPUT_REF_H, 		//UUID: 2908    VALUE: REPORT_ID_KEYBOARD, TYPE_OUTPUT
	
	// report map
	HID_REPORT_MAP_CD_H,					//UUID: 2803, 	VALUE:  			Prop: Read
	HID_REPORT_MAP_DP_H,					//UUID: 2A4B, 	VALUE: reportKeyIn
	HID_REPORT_MAP_EXT_REF_H,				//UUID: 2907 	VALUE: extService

	//hid information
	HID_INFORMATION_CD_H,					//UUID: 2803, 	VALUE:  			Prop: read
	HID_INFORMATION_DP_H,					//UUID: 2A4A 	VALUE: hidInformation
	
	//control point
	HID_CONTROL_POINT_CD_H,					//UUID: 2803, 	VALUE:  			Prop: write_without_rsp
	HID_CONTROL_POINT_DP_H,					//UUID: 2A4C 	VALUE: controlPoint


	//// battery service ////
	/**********************************************************************************************/
	BATT_PS_H, 								//UUID: 2800, 	VALUE: uuid 180f
	BATT_LEVEL_INPUT_CD_H,					//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	BATT_LEVEL_INPUT_DP_H,					//UUID: 2A19 	VALUE: batVal
	BATT_LEVEL_INPUT_CCB_H,					//UUID: 2902, 	VALUE: batValCCC


	//// Ota ////
	/**********************************************************************************************/
	OTA_PS_H, 								//UUID: 2800, 	VALUE: telink ota service uuid
	OTA_CMD_OUT_CD_H,						//UUID: 2803, 	VALUE:  			Prop: read | write_without_rsp	
	OTA_CMD_OUT_DP_H,						//UUID: telink ota uuid,  VALUE: otaData
	OTA_CMD_OUT_DESC_H,						//UUID: 2901, 	VALUE: otaName


	
#if (BLE_AUDIO_ENABLE)
	//// Audio ////
	/**********************************************************************************************/
	AUDIO_PS_H, 							//UUID: 2800, 	VALUE: telink audio service uuid
	
	//mic
	AUDIO_MIC_INPUT_CD_H,					//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	AUDIO_MIC_INPUT_DP_H,					//UUID: telink mic uuid,  VALUE: micData
	AUDIO_MIC_INPUT_CCB_H,					//UUID: 2A19 	VALUE: micDataCCC
	AUDIO_MIC_INPUT_DESC_H,					//UUID: 2901, 	VALUE: micName

	//speaker
	AUDIO_SPEAKER_OUT_CD_H,					//UUID: 2803, 	VALUE:  			Prop: write_without_rsp
	AUDIO_SPEAKER_OUT_DP_H,					//UUID: telink speaker uuid,  VALUE: speakerData
	AUDIO_SPEAKEROUT_DESC_H,				//UUID: 2901, 	VALUE: speakerName
#endif



	ATT_END_H,

}ATT_HANDLE;





















//////////////////////// DEBUG GPIO ///////////////////////////
#define  DEBUG_GPIO_ENABLE					0

#if(DEBUG_GPIO_ENABLE)

	//8261/8267/8269 hardware: C59T80A5_V1.1
	#if (__PROJECT_8261_BLE_REMOTE__ || __PROJECT_8267_BLE_REMOTE__ || __PROJECT_8269_BLE_REMOTE__)
			#define PA2_FUNC				AS_GPIO //debug gpio chn0 : A2
			#define PA3_FUNC				AS_GPIO //debug gpio chn1 : A3
			#define PA4_FUNC				AS_GPIO //debug gpio chn2 : A4
			#define PA5_FUNC				AS_GPIO //debug gpio chn3 : A5
			#define PB2_FUNC				AS_GPIO //debug gpio chn4 : B2
			#define PB3_FUNC				AS_GPIO //debug gpio chn5 : B3

			#define PA2_OUTPUT_ENABLE					1
			#define PA3_OUTPUT_ENABLE					1
			#define PA4_OUTPUT_ENABLE					1
			#define PA5_OUTPUT_ENABLE					1
			#define PB2_OUTPUT_ENABLE					1
			#define PB3_OUTPUT_ENABLE					1

			#define DBG_CHN0_LOW		( *(unsigned char *)0x800583 &= (~0x04) )
			#define DBG_CHN0_HIGH		( *(unsigned char *)0x800583 |= 0x04 )
			#define DBG_CHN0_TOGGLE		( *(unsigned char *)0x800583 ^= 0x04 )
			#define DBG_CHN1_LOW		( *(unsigned char *)0x800583 &= (~0x08) )
			#define DBG_CHN1_HIGH		( *(unsigned char *)0x800583 |= 0x08 )
			#define DBG_CHN1_TOGGLE		( *(unsigned char *)0x800583 ^= 0x08 )
			#define DBG_CHN2_LOW		( *(unsigned char *)0x800583 &= (~0x10) )
			#define DBG_CHN2_HIGH		( *(unsigned char *)0x800583 |= 0x10 )
			#define DBG_CHN2_TOGGLE		( *(unsigned char *)0x800583 ^= 0x10 )
			#define DBG_CHN3_LOW		( *(unsigned char *)0x800583 &= (~0x20) )
			#define DBG_CHN3_HIGH		( *(unsigned char *)0x800583 |= 0x20 )
			#define DBG_CHN3_TOGGLE		( *(unsigned char *)0x800583 ^= 0x20 )
			#define DBG_CHN4_LOW		( *(unsigned char *)0x80058b &= (~0x04) )
			#define DBG_CHN4_HIGH		( *(unsigned char *)0x80058b |= 0x04 )
			#define DBG_CHN4_TOGGLE		( *(unsigned char *)0x80058b ^= 0x04 )
			#define DBG_CHN5_LOW		( *(unsigned char *)0x80058b &= (~0x08) )
			#define DBG_CHN5_HIGH		( *(unsigned char *)0x80058b |= 0x08 )
			#define DBG_CHN5_TOGGLE		( *(unsigned char *)0x80058b ^= 0x08 )
	#else //8266 hardware: C43T53A5_V1.0
			#define PB0_FUNC				AS_GPIO //debug gpio chn0 : PB0
			#define PC3_FUNC				AS_GPIO //debug gpio chn1 : PC3
			#define PE7_FUNC				AS_GPIO //debug gpio chn2 : PE7
			#define PF1_FUNC				AS_GPIO //debug gpio chn3 : PF1

			#define PB0_INPUT_ENABLE					0
			#define PC3_INPUT_ENABLE					0
			#define PE7_INPUT_ENABLE					0
			#define PF1_INPUT_ENABLE					0
			#define PB0_OUTPUT_ENABLE					1
			#define PC3_OUTPUT_ENABLE					1
			#define PE7_OUTPUT_ENABLE					1
			#define PF1_OUTPUT_ENABLE					1

			#define DBG_CHN0_LOW		( *(unsigned char *)0x80058b &= (~0x01) )
			#define DBG_CHN0_HIGH		( *(unsigned char *)0x80058b |= 0x01 )
			#define DBG_CHN0_TOGGLE		( *(unsigned char *)0x80058b ^= 0x01 )
			#define DBG_CHN1_LOW		( *(unsigned char *)0x800593 &= (~0x08) )
			#define DBG_CHN1_HIGH		( *(unsigned char *)0x800593 |= 0x08 )
			#define DBG_CHN1_TOGGLE		( *(unsigned char *)0x800593 ^= 0x08 )
			#define DBG_CHN2_LOW		( *(unsigned char *)0x8005a3 &= (~0x80) )
			#define DBG_CHN2_HIGH		( *(unsigned char *)0x8005a3 |= 0x80 )
			#define DBG_CHN2_TOGGLE		( *(unsigned char *)0x8005a3 ^= 0x80 )
			#define DBG_CHN3_LOW		( *(unsigned char *)0x8005ab &= (~0x02) )
			#define DBG_CHN3_HIGH		( *(unsigned char *)0x8005ab |= 0x02 )
			#define DBG_CHN3_TOGGLE		( *(unsigned char *)0x8005ab ^= 0x02 )
	#endif


#else
#define DBG_CHN0_LOW
#define DBG_CHN0_HIGH
#define DBG_CHN0_TOGGLE
#define DBG_CHN1_LOW
#define DBG_CHN1_HIGH
#define DBG_CHN1_TOGGLE
#define DBG_CHN2_LOW
#define DBG_CHN2_HIGH
#define DBG_CHN2_TOGGLE
#define DBG_CHN3_LOW
#define DBG_CHN3_HIGH
#define DBG_CHN3_TOGGLE
#define DBG_CHN4_LOW
#define DBG_CHN4_HIGH
#define DBG_CHN4_TOGGLE
#define DBG_CHN5_LOW
#define DBG_CHN5_HIGH
#define DBG_CHN5_TOGGLE

#endif  //end of DEBUG_GPIO_ENABLE



/////////////////// set default   ////////////////

#include "../common/default_config.h"

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif
