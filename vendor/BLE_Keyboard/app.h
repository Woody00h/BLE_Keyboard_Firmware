/*
 * app.h
 *
 *  Created on: 2018/1/27
 *      Author: Woody Chen
 */

#ifndef BLE_KEYBOARD_APP_H_
#define BLE_KEYBOARD_APP_H_

typedef enum
{
	SM_INIT,
	SM_FACTORY,
	SM_PAIR,
	SM_CONTROL,
	SM_SLEEP,
	SM_ADV,
	SM_RESET,	//clear the pairing info
	SM_OTA,
	SM_EMC,
}STATE_MACHINE;

typedef enum
{
	KEY_TYPE_KEYBOARD,			//HID Keyboard keys
	KEY_TYPE_CONSUMER,			//HID Consumer control keys
	KEY_TYPE_RC_MODE,			//keys to change the RC mode
	KEY_TYPE_CUSTOMIZED,		//customized keys
	KEY_TYPE_INVALID,			//invalid keys
}KEY_TYPE;

typedef struct
{
	u8 key_index;
	u8 emc_channel;
	u8 emc_mode;
}EMC_KEY;

#define RF_CHANNEL_2402			0
#define RF_CHANNEL_2442			1
#define RF_CHANNEL_2480			2
#define RF_CARRY				0
#define RF_CARRY_DATA			1
#define RF_RX					2

#define KEY_DEBOUNCE_TIME		(50 *1000)
#define	FIFTY_MS				(50 *1000)
#define QUARTER_SECOND			(250*1000)
#define BLINK_TIME				(500*1000)
#define HALF_SECOND 			(500*1000)
#define ONE_SECOND				(1000*1000)
#define TWO_SECOND				(2 *ONE_SECOND)
#define THREE_SECOND			(3 *ONE_SECOND)
#define FIVE_SECOND				(5 *ONE_SECOND)
#define TEN_SECOND				(10*ONE_SECOND)
#define TWENTY_SECOND			(20*ONE_SECOND)
#define HALF_MIN				(30*ONE_SECOND)
#define ONE_MINUTE				(60*ONE_SECOND)
#define STUCK_KEY_SLEEP_TIME	HALF_MIN
#define FIRST_KEY_DELAY			(400*1000)

#define	LED_ON					
#define	LED_OFF					
#define	LED_TOGGLE				

#define LOW_BATTERY_THRESHOLD	2400
#define LOW_VOLTAGE_BLINK_COUNT	4

#define IS_VOICE_KEY_PRESS		voice_key_press
#define IS_VOICE_KEY_RELEASE	!voice_key_press

#define BLT_PAIR_SECURITY_ADDR	0x74000
#define BLT_MASTER_ADDR			0x40000	//store the master addr by APP

KEY_TYPE type_of_key(u8 key_index);
void Start_New_Mode(STATE_MACHINE new_mode);
u16 batt_value_get_and_filter(void);
void send_release_code(void);
void Handle_Active_Mode(void);

#endif /* BLE_KEYBOARD_APP_H_ */