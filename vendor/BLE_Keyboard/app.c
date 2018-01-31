#include "../../proj/tl_common.h"
#include "../../proj_lib/rf_drv.h"
#include "../../proj_lib/pm.h"
#include "../../proj_lib/ble/ll/ll.h"
#include "../../proj/drivers/keyboard.h"
#include "../common/tl_audio.h"
#include "../common/blt_led.h"
#include "../../proj_lib/ble/trace.h"
#include "../../proj/mcu/pwm.h"
#include "../../proj_lib/ble/service/ble_ll_ota.h"
#include "../../proj/drivers/audio.h"
#include "../../proj/drivers/adc.h"
#include "../../proj/drivers/battery.h"
#include "../../proj_lib/ble/blt_config.h"
#include "../../proj_lib/ble/ble_smp.h"
#include "../../proj_lib/ble/ble_phy.h"
#include "../../proj/drivers/uart.h"

#include "rc_ir.h"
#include "app.h"

#if (__PROJECT_BLE_KEYBOARD__)



#define 	ADV_IDLE_ENTER_DEEP_TIME			60  //60 s
#define 	CONN_IDLE_ENTER_DEEP_TIME			60  //60 s

#define 	MY_DIRECT_ADV_TMIE					2000000


#define     MY_APP_ADV_CHANNEL					BLT_ENABLE_ADV_ALL

#define 	MY_ADV_INTERVAL_MIN					ADV_INTERVAL_30MS
#define 	MY_ADV_INTERVAL_MAX					ADV_INTERVAL_35MS




MYFIFO_INIT(blt_rxfifo, 64, 8);
MYFIFO_INIT(blt_txfifo, 40, 16);


#if (BLE_PHYTEST_MODE == PHYTEST_MODE_THROUGH_2_WIRE_UART || BLE_PHYTEST_MODE == PHYTEST_MODE_OVER_HCI_WITH_UART )
	MYFIFO_INIT(hci_rx_fifo, 64, 2);
	MYFIFO_INIT(hci_tx_fifo, 64, 2);
#endif



//////////////////////////////////////////////////////////////////////////////
//	 Adv Packet, Response Packet
//////////////////////////////////////////////////////////////////////////////
const u8	tbl_advData[] = {
	0x11, 0x09, 'W', 'o', 'o', 'd', 'y', '\'','s',' ', 'K', 'e', 'y', 'b', 'o', 'a', 'r', 'd', 
	0x02, 0x01, 0x05, 							// BLE limited discoverable mode and BR/EDR not supported
	0x03, 0x19, 0xC1, 0x03, 					// 961, Keyboard, HID subtype
	0x05, 0x02, 0x12, 0x18, 0x0F, 0x18,		// incomplete list of service class UUIDs (0x1812, 0x180F)
};

const u8	tbl_scanRsp [] = {
	0x11, 0x09, 'W', 'o', 'o', 'd', 'y', '\'','s',' ', 'K', 'e', 'y', 'b', 'o', 'a', 'r', 'd', 
};

u8  tbl_mac [] = {0xe1, 0xe1, 0xe2, 0xe3, 0xe4, 0xc7};
static const u32 pin[] = KB_DRIVE_PINS;
u32 interval_update_tick = 0;
int device_in_connection_state;


const u8 keyboard_modifier_bits[] = KEYBOARD_MODIFIER_BITS;

u32		advertise_begin_tick;

u8		ui_mic_enable = 0;
u8 		key_voice_press = 0;

int 	lowBattDet_enable = 1;
int		lowBatt_alarmFlag = 0;


int     ui_mtu_size_exchange_req = 0;


//////////////////// key type ///////////////////////
#define IDLE_KEY	   			0
#define CONSUMER_KEY   	   		1
#define KEYBOARD_KEY   	   		2
#define IR_KEY   	   			3

u8 		user_key_mode;

u8 		key_buf[8] = {0};
u16		consumer_code;

int 	key_not_released;

int 	ir_not_released;

u32 	latest_user_event_tick;

u8 		user_task_flg;
u8 		sendTerminate_before_enterDeep = 0;
u8 		ota_is_working = 0;


static STATE_MACHINE Active_State_Machine;
static u8 stuck_flag;
static u32 universal_start_tick;
static u32 active_start_tick;
static u32 pair_press_start_tick;
static u32 reset_press_start_tick;
static u32 key_stuck_start_tick;
static u32 blink_start_tick;

static int my_det_key;
static u8 my_key_p1;
static u8 my_key_p2;
static u8 my_key_cnt;
static KEY_TYPE key_type = KEY_TYPE_KEYBOARD;
static u8 pair_key_press;
static u8 reset_key_press;
static u8 pair_info;
static u8 emc_test_detect;
static u8 enter_deep_sleep;
static u8 force_pairing_enable;
static u8 reset_enable;
static u8 fast_blink_cnt;
static u16 consumer_buf = 1;
static u8 low_battery_blink_cnt;
static u8 reset_confirm;
static u8 blt_pair_start;

extern u8 my_batVal[];

#if (STUCK_KEY_PROCESS_ENABLE)
	u32 	stuckKey_keyPressTime;
#endif




#if (STUCK_KEY_PROCESS_ENABLE)
	u32 	stuckKey_keyPressTime;
#endif




#if (REMOTE_IR_ENABLE)
	//ir key
	#define TYPE_IR_SEND			1
	#define TYPE_IR_RELEASE			2

	///////////////////// key mode //////////////////////
	#define KEY_MODE_BLE	   		0    //ble key
	#define KEY_MODE_IR        		1    //ir  key


	static const u8 kb_map_ble[49] = 	KB_MAP_BLE;  //7*7
	static const u8 kb_map_ir[49] = 	KB_MAP_IR;   //7*7


	void ir_dispatch(u8 type, u8 syscode ,u8 ircode){
		if(type == TYPE_IR_SEND){
			ir_nec_send(syscode,~(syscode),ircode);
		}
		else if(type == TYPE_IR_RELEASE){
			ir_send_release();
		}
	}


#endif



#if (BLE_REMOTE_OTA_ENABLE)
	void entry_ota_mode(void)
	{
		ota_is_working = 1;
		// device_led_setup(led_cfg[LED_SHINE_OTA]);
		bls_ota_setTimeout(15 * 1000 * 1000); //set OTA timeout  15 seconds
	}



	void LED_show_ota_result(int result)
	{
		#if 0
			irq_disable();
			WATCHDOG_DISABLE;

			gpio_set_output_en(GPIO_LED, 1);

			if(result == OTA_SUCCESS){  //OTA success
				gpio_write(GPIO_LED, 1);
				sleep_us(2000000);  //led on for 2 second
				gpio_write(GPIO_LED, 0);
			}
			else{  //OTA fail

			}

			gpio_set_output_en(GPIO_LED, 0);
		#endif
	}
#endif

#if (BLE_AUDIO_ENABLE)
	u32 	key_voice_pressTick = 0;

	void		ui_enable_mic (u8 en)
	{
		ui_mic_enable = en;

		//AMIC Bias output
		gpio_set_output_en (GPIO_AMIC_BIAS, en);
		gpio_write (GPIO_AMIC_BIAS, en);

		// device_led_setup(led_cfg[en ? LED_AUDIO_ON : LED_AUDIO_OFF]);


		if(en){  //audio on
			lowBattDet_enable = 0;
			battery2audio();////switch auto mode
		}
		else{  //audio off
			audio2battery();////switch manual mode
			lowBattDet_enable = 1;
		}
	}

	void voice_press_proc(void)
	{
		key_voice_press = 0;
		ui_enable_mic (1);
		if(ui_mtu_size_exchange_req && blc_ll_getCurrentState() == BLS_LINK_STATE_CONN){
			ui_mtu_size_exchange_req = 0;
			blc_att_requestMtuSizeExchange(BLS_CONN_HANDLE, 0x009e);
		}
	}


	void	task_audio (void)
	{
		static u32 audioProcTick = 0;
		if(clock_time_exceed(audioProcTick, 5000)){
			audioProcTick = clock_time();
		}
		else{
			return;
		}

		///////////////////////////////////////////////////////////////
		log_event(TR_T_audioTask);

		proc_mic_encoder ();

		//////////////////////////////////////////////////////////////////
		if (blc_ll_getTxFifoNumber() < 10)
		{
			int *p = mic_encoder_data_buffer ();
			if (p)					//around 3.2 ms @16MHz clock
			{
				log_event (TR_T_audioData);
				bls_att_pushNotifyData (AUDIO_MIC_INPUT_DP_H, (u8*)p, ADPCM_PACKET_LEN);
			}
		}
	}



	void blc_checkConnParamUpdate(void)
	{
		if(	 interval_update_tick && clock_time_exceed(interval_update_tick,5*1000*1000) && \
			 blc_ll_getCurrentState() == BLS_LINK_STATE_CONN &&  bls_ll_getConnectionInterval()!= 8 )
		{
			interval_update_tick = clock_time() | 1;
			bls_l2cap_requestConnParamUpdate (8, 8, 99, 400);
		}
	}


#endif




void 	app_switch_to_indirect_adv(u8 e, u8 *p, int n)
{

	bls_ll_setAdvParam( MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
						ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC,
						0,  NULL,
						MY_APP_ADV_CHANNEL,
						ADV_FP_NONE);

	bls_ll_setAdvEnable(1);  //must: set adv enable
}



void 	ble_remote_terminate(u8 e,u8 *p, int n) //*p is terminate reason
{
	device_in_connection_state = 0;


	if(*p == HCI_ERR_CONN_TIMEOUT){

	}
	else if(*p == HCI_ERR_REMOTE_USER_TERM_CONN){  //0x13

	}
	else if(*p == HCI_ERR_CONN_TERM_MIC_FAILURE){

	}
	else{

	}



#if (BLE_REMOTE_PM_ENABLE)
	 //user has push terminate pkt to ble TX buffer before deepsleep
	if(sendTerminate_before_enterDeep == 1){
		sendTerminate_before_enterDeep = 2;
	}
#endif


#if (BLE_AUDIO_ENABLE)
	if(ui_mic_enable){
		ui_enable_mic (0);
	}
#endif

	advertise_begin_tick = clock_time();

}

void	task_connect (u8 e, u8 *p, int n)
{
	bls_l2cap_requestConnParamUpdate (8, 8, 99, 200);  //interval=10ms latency=99 timeout=2s

	latest_user_event_tick = clock_time();

	ui_mtu_size_exchange_req = 1;

	device_in_connection_state = 1;//

	interval_update_tick = clock_time() | 1; //none zero

	if(blt_pair_start)
	{
		rf_set_power_level_index (RF_POWER_8dBm);			//power : 8dbm
	}
}


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////


//This function process ...
void deep_wakeup_proc(void)
{
#if(DEEPBACK_FAST_KEYSCAN_ENABLE)
	//if deepsleep wakeup is wakeup by GPIO(key press), we must quickly scan this
	//press, hold this data to the cache, when connection established OK, send to master
	//deepsleep_wakeup_fast_keyscan
	if(analog_read(DEEP_ANA_REG0) == CONN_DEEP_FLG){
		if(kb_scan_key (KB_NUMLOCK_STATUS_POWERON, 1) && kb_event.cnt){
			deepback_key_state = DEEPBACK_KEY_CACHE;
			key_not_released = 1;
			memcpy(&kb_event_cache,&kb_event,sizeof(kb_event));
		}

		analog_write(DEEP_ANA_REG0, 0);
	}
#endif
}





void deepback_pre_proc(int *det_key)
{
#if (DEEPBACK_FAST_KEYSCAN_ENABLE)
	// to handle deepback key cache
	if(!(*det_key) && deepback_key_state == DEEPBACK_KEY_CACHE && blc_ll_getCurrentState() == BLS_LINK_STATE_CONN \
			&& clock_time_exceed(bls_ll_getConnectionCreateTime(), 400000)){

		memcpy(&kb_event,&kb_event_cache,sizeof(kb_event));
		*det_key = 1;

		if(key_not_released){  //no need manual release
			deepback_key_state = DEEPBACK_KEY_IDLE;
		}
		else{  //need manual release
			deepback_key_tick = clock_time();
			deepback_key_state = DEEPBACK_KEY_WAIT_RELEASE;
		}
	}
#endif
}

void deepback_post_proc(void)
{
#if (DEEPBACK_FAST_KEYSCAN_ENABLE)
	//manual key release
	if(deepback_key_state == DEEPBACK_KEY_WAIT_RELEASE && clock_time_exceed(deepback_key_tick,150000)){
		key_not_released = 0;

		send_release_code();

		deepback_key_state = DEEPBACK_KEY_IDLE;
	}
#endif
}


void key_change_proc(void)
{
	u8 i;
	u8 key_buf_index = 2;
	
	latest_user_event_tick = clock_time();  //record latest key change time

	if(key_voice_press){  //clear voice key press flg
		key_voice_press = 0;
	}

	memset(key_buf, 0, 8);

	key_not_released = 1;
	if (kb_event.cnt)  
	{
		/* Combo keys(Fn + X)
		 * F5 	PLAY/PAUSE
		 * F6 	STOP
		 * F7 	REWIND
		 * F8 	FAST FORWARD
		 * F9 	AUDIO PLAYER
		 * F10 	MUTE
		 * F11 	VOL DOWN
		 * F12 	VOL UP
		 */
		if(kb_event.cnt == 2 && \
		  (kb_event.keycode[0] == VK_FN || kb_event.keycode[1] == VK_FN))
		{
			switch ( kb_event.keycode[0] + kb_event.keycode[1])
			{
				case (VK_FN + VK_F5):
						consumer_code = 0xcd;
						break;

				case (VK_FN + VK_F6):
						consumer_code = 0xb7;
						break;

				case (VK_FN + VK_F7):
						consumer_code = 0xb4;
						break;

				case (VK_FN + VK_F8):
						consumer_code = 0xb3;
						break;

				case (VK_FN + VK_F9):
						consumer_code = 0x1c7;
						break;

				case (VK_FN + VK_F10):
						consumer_code = 0xe2;
						break;

				case (VK_FN + VK_F11):
						consumer_code = 0xea;
						break;

				case (VK_FN + VK_F12):
						consumer_code = 0xe9;
						break;

				default:
						consumer_code = 0;
					break;
			}

			if(consumer_code)
			{
				key_type = CONSUMER_KEY;
				bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_code, 2);
			}
		}
		else //not combo keys
		{
			if(key_type == CONSUMER_KEY) //combo keys just triggered
			{
				consumer_code = 0;
				bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_code, 2);
			}
			else //normal keys
			{
				for(i=0; i<kb_event.cnt; i++)
				{
					if(kb_event.keycode[i] > 0xf0 )
					{
						//modifier
						key_buf[0] |= keyboard_modifier_bits[kb_event.keycode[i] & 0x0f];
					}
					else
					{
						//normal keys
						key_buf[key_buf_index++] = kb_event.keycode[i];
					}
				}

				bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8);
			}

			key_type = KEYBOARD_KEY;
		}
	}
	else   //kb_event.cnt == 0,  key release
	{
		key_not_released = 0;

		if(key_type == CONSUMER_KEY)
		{
			consumer_code = 0;
			bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_code, 2);
		}
		else
		{
			key_buf[2] = 0;
			bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8); //release
		}

		key_type = IDLE_KEY;
	}
}



#define GPIO_WAKEUP_KEYPROC_CNT				3

void proc_keyboard (u8 e, u8 *p, int n)
{
	static int gpioWakeup_keyProc_cnt = 0;
	static u32 keyScanTick = 0;

	//when key press gpio wakeup suspend, proc keyscan at least GPIO_WAKEUP_KEYPROC_CNT times
	//regardless of 8000 us interval
	if(e == BLT_EV_FLAG_GPIO_EARLY_WAKEUP){
		gpioWakeup_keyProc_cnt = GPIO_WAKEUP_KEYPROC_CNT;
	}
	else if(gpioWakeup_keyProc_cnt){
		gpioWakeup_keyProc_cnt --;
	}


	if(gpioWakeup_keyProc_cnt || clock_time_exceed(keyScanTick, 8000)){
		keyScanTick = clock_time();
	}
	else{
		return;
	}

	memset(key_buf, 0, 8);
	memset(&kb_event, 0, sizeof(kb_data_t));
	if(!ui_mic_enable)
	{
		my_det_key = kb_scan_key (0, 1);
	}
#if(DEEPBACK_FAST_KEYSCAN_ENABLE)
	if(deepback_key_state != DEEPBACK_KEY_IDLE)
	{
		deepback_pre_proc(&my_det_key);
	}
#endif

	my_key_cnt = kb_event.cnt;
	my_key_p1 = kb_event.keycode[0];
	my_key_p2 = kb_event.keycode[1];

	Handle_Active_Mode();
}


extern u32	scan_pin_need;


//_attribute_ram_code_
void blt_pm_proc(void)
{

#if(BLE_REMOTE_PM_ENABLE)
	if(ui_mic_enable)
	{
		bls_pm_setSuspendMask (MCU_STALL);
	}
#if(REMOTE_IR_ENABLE)
	else if( ir_send_ctrl.is_sending || ir_send_ctrl.repeat_timer_enable){
		bls_pm_setSuspendMask(SUSPEND_DISABLE);
	}
#endif
#if (BLE_PHYTEST_MODE == PHYTEST_MODE_THROUGH_2_WIRE_UART)
	else if( blc_phy_isPhyTestEnable() )
	{
		bls_pm_setSuspendMask(SUSPEND_DISABLE);
	}
#endif
	else
	{
		bls_pm_setSuspendMask (SUSPEND_ADV | SUSPEND_CONN);

		user_task_flg = ota_is_working || scan_pin_need || key_not_released;

		if(user_task_flg){
			#if (LONG_PRESS_KEY_POWER_OPTIMIZE)
				extern int key_matrix_same_as_last_cnt;
				if(!ota_is_working && key_matrix_same_as_last_cnt > 5 && !low_battery_blink_cnt){  //key matrix stable can optize
					bls_pm_setManualLatency(3);
				}
				else{
					bls_pm_setManualLatency(0);  //latency off: 0
				}
			#else
				bls_pm_setManualLatency(0);
			#endif
		}


	#if 1 //deepsleep
		if(sendTerminate_before_enterDeep == 1) //sending Terminate and wait for ack before enter deepsleep
		{
			if(user_task_flg && !stuck_flag)  //detect key Press again,  can not enter deep now
			{
				sendTerminate_before_enterDeep = 0;
				bls_ll_setAdvEnable(1);   //enable adv again
				enter_deep_sleep = 0;
				Start_New_Mode(SM_ADV);
			}
		}
		else if(sendTerminate_before_enterDeep == 2)  //Terminate OK
		{
			if(pair_info)
				analog_write(DEEP_ANA_REG0,CONN_DEEP_FLG);
			else
				analog_write(DEEP_ANA_REG0,ADV_DEEP_FLG);

			cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0);  //deepsleep
		}

		if(enter_deep_sleep) //need to sleep now
		{
			if(!user_task_flg || stuck_flag)
			{
				if(blc_ll_getCurrentState() == BLS_LINK_STATE_CONN)
				{
					bls_ll_terminateConnection(HCI_ERR_REMOTE_USER_TERM_CONN); //push terminate cmd into ble TX buffer
					sendTerminate_before_enterDeep = 1;
				}
				else if(blc_ll_getCurrentState() == BLS_LINK_STATE_ADV)
				{
					if(pair_info)
						analog_write(DEEP_ANA_REG0,CONN_DEEP_FLG);
					else
						analog_write(DEEP_ANA_REG0,ADV_DEEP_FLG);

					cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0);  //deepsleep
				}
			}
		}//end if(enter_deep_sleep)
	#endif

	}

#endif  //END of  BLE_REMOTE_PM_ENABLE
}



_attribute_ram_code_ void  ble_remote_set_sleep_wakeup (u8 e, u8 *p, int n)
{
	if( blc_ll_getCurrentState() == BLS_LINK_STATE_CONN && ((u32)(bls_pm_getSystemWakeupTick() - clock_time())) > 80 * CLOCK_SYS_CLOCK_1MS){  //suspend time > 30ms.add gpio wakeup
		bls_pm_setWakeupSource(PM_WAKEUP_CORE);  //gpio CORE wakeup suspend
	}
}




void user_init()
{

	blc_app_loadCustomizedParameters();  //load customized freq_offset cap value and tp value


////////////////// BLE stack initialization ////////////////////////////////////
	u32 *pmac = (u32 *) CFG_ADR_MAC;
	if (*pmac != 0xffffffff)
	{
		memcpy (tbl_mac, pmac, 6);
	}
	else{
		tbl_mac[0] = (u8)rand();
		flash_write_page (CFG_ADR_MAC, 6, tbl_mac);
	}

	////// Controller Initialization  //////////
	blc_ll_initBasicMCU(tbl_mac);   //mandatory

	blc_ll_initAdvertising_module(tbl_mac); 	//adv module: 		 mandatory for BLE slave,
	blc_ll_initSlaveRole_module();				//slave module: 	 mandatory for BLE slave,
	blc_ll_initPowerManagement_module();        //pm module:      	 optional



	////// Host Initialization  //////////
	extern void my_att_init ();
	my_att_init (); //gatt initialization
	blc_l2cap_register_handler (blc_l2cap_packet_receive);  	//l2cap initialization


 	//// smp initialization ////
#if (BLE_REMOTE_SECURITY_ENABLE)
	blc_smp_param_setBondingDeviceMaxNumber(4);  	//default is SMP_BONDING_DEVICE_MAX_NUM, can not bigger that this value
													//and this func must call before bls_smp_enableParing
	bls_smp_enableParing (SMP_PARING_CONN_TRRIGER );
#else
	bls_smp_enableParing (SMP_PARING_DISABLE_TRRIGER );
#endif

	//HID_service_on_android7p0_init();  //hid device on android 7.0/7.1




///////////////////// USER application initialization ///////////////////
	bls_ll_setAdvData( (u8 *)tbl_advData, sizeof(tbl_advData) );
	bls_ll_setScanRspData( (u8 *)tbl_scanRsp, sizeof(tbl_scanRsp));




	////////////////// config adv packet /////////////////////
#if (BLE_REMOTE_SECURITY_ENABLE)
	u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
	smp_param_save_t  bondInfo;
	if(bond_number)   //at least 1 bonding device exist
	{
		blc_smp_param_loadByIndex( bond_number - 1, &bondInfo);  //get the latest bonding device (index: bond_number-1 )

	}

	if(bond_number)   //set direct adv
	{
		//set direct adv
		ll_whiteList_add(bondInfo.peer_addr_type, bondInfo.peer_addr);

		u8 status = bls_ll_setAdvParam( MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
										ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, OWN_ADDRESS_PUBLIC,
										bondInfo.peer_addr_type,  bondInfo.peer_addr,
										MY_APP_ADV_CHANNEL,
										ADV_FP_ALLOW_SCAN_ANY_ALLOW_CONN_WL);
		if(status != BLE_SUCCESS) { write_reg8(0x8000, 0x11); 	while(1); }  //debug: adv setting err

		//it is recommended that direct adv only last for several seconds, then switch to indirect adv
#if 0
		bls_ll_setAdvDuration(MY_DIRECT_ADV_TMIE, 1);
		bls_app_registerEventCallback (BLT_EV_FLAG_ADV_DURATION_TIMEOUT, &app_switch_to_indirect_adv);
#endif

		pair_info = 1;
	}
	else   //set indirect adv
#endif
	{
		u8 status = bls_ll_setAdvParam(  MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
										 ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC,
										 0,  NULL,
										 MY_APP_ADV_CHANNEL,
										 ADV_FP_NONE);
		if(status != BLE_SUCCESS) { write_reg8(0x8000, 0x11); 	while(1); }  //debug: adv setting err

		pair_info = 0;
	}

	bls_ll_setAdvEnable(1);  //adv enable
	rf_set_power_level_index (RF_POWER_8dBm);

	//ble event call back
	bls_app_registerEventCallback (BLT_EV_FLAG_CONNECT, &task_connect);
	bls_app_registerEventCallback (BLT_EV_FLAG_TERMINATE, &ble_remote_terminate);


	///////////////////// keyboard matrix initialization///////////////////
	for (int i=0; i<(sizeof (pin)/sizeof(*pin)); i++)
	{
		gpio_set_wakeup(pin[i],1,1);  	   //drive pin core(gpio) high wakeup suspend
		cpu_set_gpio_wakeup (pin[i],1,1);  //drive pin pad high wakeup deepsleep
	}


	bls_app_registerEventCallback (BLT_EV_FLAG_GPIO_EARLY_WAKEUP, &proc_keyboard);


	///////////////////// AUDIO initialization///////////////////
#if (BLE_AUDIO_ENABLE)
	//buffer_mic set must before audio_init !!!
	config_mic_buffer ((u32)buffer_mic, TL_MIC_BUFFER_SIZE);

	#if (BLE_DMIC_ENABLE)  //Dmic config
		/////////////// DMIC: PA0-data, PA1-clk, PA3-power ctl
		gpio_set_func(GPIO_PA0, AS_DMIC);
		gpio_set_func(GPIO_PA1, AS_DMIC);

		gpio_set_input_en(GPIO_PA0 , 1);                //PA0 as input

		gpio_set_func(GPIO_PA3, AS_GPIO);
		gpio_set_input_en(GPIO_PA3, 1);
		gpio_set_output_en(GPIO_PA3, 1);
		gpio_write(GPIO_PA3, 0);

		#if TL_MIC_32K_FIR_16K
			audio_dmic_init(1, R32, CLOCK_SYS_TYPE);  //1 indicate 1M; 32K
		#else
			audio_dmic_init(1, R64, CLOCK_SYS_TYPE);  //1 indicate 1M; 16K
		#endif
	#else  //Amic config
		//////////////// AMIC: PC3 - bias; PC4/PC5 - input
		#if TL_MIC_32K_FIR_16K
			#if (CLOCK_SYS_CLOCK_HZ == 16000000)
				audio_amic_init( DIFF_MODE, 26,  9, R2, CLOCK_SYS_TYPE);
				audio_finetune_sample_rate(2);  //reg0x30[1:0] 2 bits for fine tuning, divider for slow down sample rate
			#elif (CLOCK_SYS_CLOCK_HZ == 24000000)
				audio_amic_init( DIFF_MODE, 33, 15, R2, CLOCK_SYS_TYPE);
				audio_finetune_sample_rate(3);
			#elif (CLOCK_SYS_CLOCK_HZ == 32000000)
				audio_amic_init( DIFF_MODE, 45, 20, R2, CLOCK_SYS_TYPE); // 16 , 15
			#elif (CLOCK_SYS_CLOCK_HZ == 48000000)
				audio_amic_init( DIFF_MODE, 65, 15, R3, CLOCK_SYS_TYPE);
			#endif
		#else
			#if (CLOCK_SYS_CLOCK_HZ == 16000000)
				audio_amic_init( DIFF_MODE, 26,  9, R4, CLOCK_SYS_TYPE);
				audio_finetune_sample_rate(2);
			#elif (CLOCK_SYS_CLOCK_HZ == 24000000)
				audio_amic_init( DIFF_MODE, 33, 15, R4, CLOCK_SYS_TYPE);
				audio_finetune_sample_rate(3);
			#elif (CLOCK_SYS_CLOCK_HZ == 32000000)
				audio_amic_init( DIFF_MODE, 45, 20, R4, CLOCK_SYS_TYPE);
			#elif (CLOCK_SYS_CLOCK_HZ == 48000000)
				audio_amic_init( DIFF_MODE, 65, 15, R6, CLOCK_SYS_TYPE);
			#endif
		#endif
	audio_amic_input_set(PGA_CH);//audio input set, ignore the input parameter
	#endif//end of BLE_DMIC_ENABLE
#endif

#if(BATT_CHECK_ENABLE)
	#if((MCU_CORE_TYPE == MCU_CORE_8261)||(MCU_CORE_TYPE == MCU_CORE_8267)||(MCU_CORE_TYPE == MCU_CORE_8269))
		adc_BatteryCheckInit(ADC_CLK_4M, 1, Battery_Chn_VCC, 0, SINGLEEND, RV_1P428, RES14, S_3);
	#elif(MCU_CORE_TYPE == MCU_CORE_8266)
		adc_Init(ADC_CLK_4M, ADC_CHN_C4, SINGLEEND, ADC_REF_VOL_1V3, ADC_SAMPLING_RES_14BIT, ADC_SAMPLING_CYCLE_6);
	#endif
#endif





		///////////////////// Power Management initialization///////////////////
#if(BLE_REMOTE_PM_ENABLE)
	blc_ll_initPowerManagement_module();
	bls_pm_setSuspendMask (SUSPEND_ADV | SUSPEND_CONN);
	bls_app_registerEventCallback (BLT_EV_FLAG_SUSPEND_ENTER, &ble_remote_set_sleep_wakeup);
#else
	bls_pm_setSuspendMask (SUSPEND_DISABLE);
#endif


	////////////////LED initialization /////////////////////////
	//device_led_init(GPIO_LED, 1);

#if (BLE_REMOTE_OTA_ENABLE)
	////////////////// OTA relative ////////////////////////
	bls_ota_clearNewFwDataArea(); //must
	bls_ota_registerStartCmdCb(entry_ota_mode);
	bls_ota_registerResultIndicateCb(LED_show_ota_result);
#endif


#if (REMOTE_IR_ENABLE)
	user_key_mode = analog_read(DEEP_ANA_REG1);
	//user_key_mode = KEY_MODE_IR;  //debug

	rc_ir_init();

	analog_write(DEEP_ANA_REG1, 0);  //clear
#endif


#if (BLE_PHYTEST_MODE == PHYTEST_MODE_THROUGH_2_WIRE_UART || BLE_PHYTEST_MODE == PHYTEST_MODE_OVER_HCI_WITH_UART )
	blc_phy_initPhyTest_module();

	#if (MCU_CORE_TYPE == MCU_CORE_8267)
		gpio_set_input_en(GPIO_PB2, 1);   		//UART B2 B3
		gpio_set_input_en(GPIO_PB3, 1);
		gpio_setup_up_down_resistor(GPIO_PB2, PM_PIN_PULLUP_1M);
		gpio_setup_up_down_resistor(GPIO_PB3, PM_PIN_PULLUP_1M);
		gpio_set_func(GPIO_PB2, AS_UART);
		gpio_set_func(GPIO_PB3, AS_UART);
	#endif

	reg_dma_rx_rdy0 = FLD_DMA_UART_RX | FLD_DMA_UART_TX; //clear uart rx/tx status
	#if (CLOCK_SYS_CLOCK_HZ == 32000000)
		CLK32M_UART115200;
	#elif(CLOCK_SYS_CLOCK_HZ == 16000000)
		CLK16M_UART115200;
	#else
		need config uart clock here
	#endif

	uart_BuffInit(hci_rx_fifo_b, hci_rx_fifo.size, hci_tx_fifo_b);
	#if (BLE_PHYTEST_MODE == PHYTEST_MODE_THROUGH_2_WIRE_UART)
		blc_register_hci_handler (phy_test_2_wire_rx_from_uart, phy_test_2_wire_tx_to_uart);
	#endif



#endif


	advertise_begin_tick = clock_time();

}

void Start_Init_Mode()
{
	universal_start_tick = clock_time();
	force_pairing_enable = 1;
	reset_enable = 1;
	//LED_ON;
}

void Handle_Init_Mode()
{
	//if(clock_time_exceed(universal_start_tick,THREE_SECOND))
	{
		LED_OFF;

		if(emc_test_detect)
		{
			Start_New_Mode(SM_EMC);
			return;
		}

		//If the pairing table is empty RC will enter pairing mode when 'pressing any key'
		//If the pairing table is not empty RC will enter pairing mode by pressing combo keys.
		if(!pair_info) //no pair info
		{
			if(analog_read(DEEP_ANA_REG0)) //key press wakeup
			{
				Start_New_Mode(SM_PAIR);
			}
			else //power on
			{
				enter_deep_sleep = 1;
			}
		}
		else
		{
			Start_New_Mode(SM_ADV);
		}
	}
}


void Start_Control_Mode()
{
	active_start_tick = clock_time();
}

void Handle_Control_Mode()
{
	u8 i;
	u8 key_buf_index = 2;

	/******************************************************************************************
	*******connection down handle**************************************************************
	******************************************************************************************/
	if(!bls_ll_isConnectState())
	{
#if BLE_AUDIO_ENABLE
		if(ui_mic_enable)
		{
			audio2battery();
			ui_enable_mic(0);
		}
#endif
		LED_OFF;
		enter_deep_sleep = 1;
		return;
	}

	/******************************************************************************************
	*******Handle the keyboard*****************************************************************
	******************************************************************************************/
	if(my_det_key)
	{
		/* Combo keys(Fn + X)
		 * F5 	PLAY/PAUSE
		 * F6 	STOP
		 * F7 	REWIND
		 * F8 	FAST FORWARD
		 * F9 	AUDIO PLAYER
		 * F10 	MUTE
		 * F11 	VOL DOWN
		 * F12 	VOL UP
		 */
		if(my_key_cnt == 2 && \
		  (my_key_p1 == VK_FN || my_key_p2 == VK_FN))
		{
			switch ( my_key_p1 + my_key_p2)
			{
				case (VK_FN + VK_F5):
						consumer_code = 0xcd;
						break;

				case (VK_FN + VK_F6):
						consumer_code = 0xb7;
						break;

				case (VK_FN + VK_F7):
						consumer_code = 0xb4;
						break;

				case (VK_FN + VK_F8):
						consumer_code = 0xb3;
						break;

				case (VK_FN + VK_F9):
						consumer_code = 0x1c7;
						break;

				case (VK_FN + VK_F10):
						consumer_code = 0xe2;
						break;

				case (VK_FN + VK_F11):
						consumer_code = 0xea;
						break;

				case (VK_FN + VK_F12):
						consumer_code = 0xe9;
						break;

				default:
						consumer_code = 0;
					break;
			}

			if(consumer_code)
			{
				key_type = CONSUMER_KEY;
				bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_code, 2);
			}
		}
		else //not combo keys
		{
			if(my_key_cnt)
			{
				if(key_type == KEY_TYPE_CONSUMER) //combo keys just triggered
				{
					consumer_code = 0;
					bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_code, 2);
				}
				else //normal keys
				{
					for(i=0; i<my_key_cnt; i++)
					{
						if(kb_event.keycode[i] > 0xf0 )
						{
							//modifier
							key_buf[0] |= keyboard_modifier_bits[kb_event.keycode[i] & 0x0f];
						}
						else
						{
							//normal keys
							key_buf[key_buf_index++] = kb_event.keycode[i];
						}
					}

					bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8);
				}
				key_type = KEY_TYPE_KEYBOARD;
			}
			else
			{
				send_release_code();
			}
		}
	}

	/*****************************************************************************************
	*********low power handle*****************************************************************
	*****************************************************************************************/
	if(key_not_released)
	{
		active_start_tick = clock_time();
	}

	//no operation for more than 60s, sleep
	{
		if(clock_time_exceed(active_start_tick , ONE_MINUTE))
		{
			//enter_deep_sleep = 1;
			return;
		}
	}
}


void Start_Pair_Mode()
{
	blt_pair_start = 1;

	bls_ll_setAdvDuration (0, 0);  //duration disable
	bls_ll_setAdvParam( ADV_INTERVAL_30MS, ADV_INTERVAL_30MS, \
						ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC, \
						0,  NULL,  BLT_ENABLE_ADV_ALL, ADV_FP_NONE);
	blt_state = BLS_LINK_STATE_ADV;
	rf_set_power_level_index (RF_POWER_m10dBm);			//power : -10dbm
	blink_start_tick = universal_start_tick = clock_time();	//record the start time
	fast_blink_cnt = 0;
}

void Handle_Pair_Mode()
{

	if(bls_ll_isConnectState()) //connected
	{
		bls_pm_setManualLatency(0);  //set the latency to 0, so that LED could blink fast

		//LED fast blink 2 times if pairing success
		if(fast_blink_cnt < 5)
		{
			if(clock_time_exceed(blink_start_tick , QUARTER_SECOND))
			{
				fast_blink_cnt++;
				LED_TOGGLE;
				blink_start_tick = clock_time();
			}
		}
		else
		{
			LED_OFF;
			bls_ll_setAdvData((u8 *)tbl_advData, sizeof(tbl_advData));
			blt_pair_start = 0;
			pair_info = 1;
			Start_New_Mode(SM_CONTROL);
		}
	}
	else
	{
		// send pair adv packet for more than 30S, sleep
		if(clock_time_exceed(universal_start_tick , HALF_MIN))
		{
			LED_OFF;
			blt_pair_start = 0;
			enter_deep_sleep = 1;
			return;
		}

		//LED Blink when wait pairing
#if 0
		if(clock_time_exceed(blink_start_tick , BLINK_TIME))
		{
			blink_start_tick = clock_time();
			LED_TOGGLE;
		}
#else
		LED_ON;
#endif
	}
}


void Start_Adv_Mode()
{
	universal_start_tick = clock_time();
}

void Handle_Adv_Mode()
{
	//send adv packet for more than 1min, sleep
	if(clock_time_exceed(universal_start_tick , ONE_MINUTE))
	{
		enter_deep_sleep = 1;
	}

	if(bls_ll_isConnectState()) //connected
	{
		Start_New_Mode(SM_CONTROL);
	}
}


void Start_Reset_Mode()
{
	universal_start_tick = blink_start_tick = clock_time();
	reset_confirm = 0;
	send_release_code();
}

void Handle_Reset_Mode()
{
	if(!clock_time_exceed(universal_start_tick , THREE_SECOND))
	{
		if(!reset_confirm)
		{
			flash_erase_sector(BLT_PAIR_SECURITY_ADDR);			//erase bond info
			reset_confirm = 1;
		}
	}
	else
	{
		LED_OFF;

		if(key_not_released)
		{

		}
		else
		{
			enter_deep_sleep = 1;
		}
	}
}

#if 0
u8 emi_chn_sel;
u8 emi_mode_sel;
const EMC_KEY emc_config[] = {
		{VK_1,		RF_CHANNEL_2402,	RF_CARRY},
		{VK_2,		RF_CHANNEL_2402,	RF_CARRY_DATA},
		{VK_3,		RF_CHANNEL_2402,	RF_RX},
		{VK_4,		RF_CHANNEL_2442,	RF_CARRY},
		{VK_5,		RF_CHANNEL_2442,	RF_CARRY_DATA},
		{VK_6,		RF_CHANNEL_2442,	RF_RX},
		{VK_7,		RF_CHANNEL_2480,	RF_CARRY},
		{VK_8,		RF_CHANNEL_2480,	RF_CARRY_DATA},
		{VK_9,		RF_CHANNEL_2480,	RF_RX},
};
#define EMC_KEY_CNT		(sizeof(emc_config)/sizeof(emc_config[0]))
u8 emi_proc_keyboard (u8 e, u8 *p)
{
	u8 cmd = 0;
	u8 i;
	memset(&kb_event, 0, sizeof(kb_data_t));

	int det_key = kb_scan_key (0, 1);

	if (det_key){

		u8 key = kb_event.keycode[0];

		for(i=0;i<EMC_KEY_CNT;i++)
		{
			if(key == emc_config[i].key_index)
			{
				emi_chn_sel  = emc_config[i].emc_channel;
				emi_mode_sel = emc_config[i].emc_mode;
				cmd = 0x80;
				break;
			}
		}
	}
	return cmd;
}


void emi_test_proc()
{
	u8 cmd = 0;
	static u8 emi_flg_init;

	cmd = emi_proc_keyboard(0, 0);
	if(!emi_flg_init){
		emi_flg_init = 1;
		cmd |= emi_flg_init;

		sleep_us(2000);
	}

	emi_process(cmd, emi_chn_sel, emi_mode_sel, tbl_mac, RF_POWER_8dBm);
}

void Start_EMC_Mode()
{
	universal_start_tick = clock_time();
	LED_OFF;
	fast_blink_cnt = 6;
}

void Handle_EMC_Mode()
{
	while(1)
	{
		//handle the LED
		if(fast_blink_cnt > 0)
		{
			//when enter factory mode, LED blink twice
			if(clock_time_exceed(universal_start_tick , QUARTER_SECOND))
			{
				universal_start_tick = clock_time();
				fast_blink_cnt--;
				LED_TOGGLE;
			}
		}
		else
		{
			LED_OFF;
		}

		emi_test_proc();

#if (MODULE_WATCHDOG_ENABLE)
		wd_clear(); //clear watch dog
#endif
	}
}
#endif

void Start_New_Mode(STATE_MACHINE new_mode)
{
	Active_State_Machine = new_mode;

	switch (Active_State_Machine)
	{
	case SM_INIT:
		Start_Init_Mode();
		break;
/*

	case SM_FACTORY:
		Start_Factory_Mode();
		break;
*/

	case SM_PAIR:
		Start_Pair_Mode();
		break;

	case SM_CONTROL:
		Start_Control_Mode();
		break;
/*

	case SM_SLEEP:
		Start_Sleep_Mode();
		break;
*/

	case SM_ADV:
		Start_Adv_Mode();
		break;

	case SM_RESET:
		Start_Reset_Mode();
		break;
/*

	case SM_OTA:
		Start_OTA_Mode();
		break;


	case SM_EMC:
		Start_EMC_Mode();
		break;
*/
	default:
		break;


	}
}

void Handle_Active_Mode(void)
{
/*

	if(ota_is_working && Active_State_Machine != SM_OTA)
	{
		Start_New_Mode(SM_OTA);
	}
*/

/*
 *
 * Press <MUTE> + <V+> for 3s, force pair
 * press <V-> + <V+> then <HOME>, clear pair info
 *
 */
	if(my_det_key)
	{
		if(my_key_cnt)
		{
			key_not_released = 1;
			//if other keys pressed before the cached key restore,
			//put new keys into the cache
			if(deepback_key_state == DEEPBACK_KEY_CACHE)
			{
				memcpy(&kb_event_cache,&kb_event,sizeof(kb_event));
			}
		}
		else
		{
			key_not_released = 0;
			//prevent re-entering pair mode if user hold the force pairing keys and not release.
			//if key released this flag will be set.
			force_pairing_enable = 1;
			reset_enable = 1;
		}

		if(my_key_cnt==2)
		{
			//press Fn & P to force pair
			if( (VK_FN == my_key_p1 && VK_P == my_key_p2 ) || \
				(VK_P == my_key_p1 && VK_FN == my_key_p2))
			{
				pair_key_press = 1;
				pair_press_start_tick = clock_time();
			}
			else
			{
				pair_key_press = 0;
			}

			//press Fn & C  to reset
			if( (VK_FN == my_key_p1 && VK_C == my_key_p2 ) || \
				(VK_C == my_key_p1 && VK_FN == my_key_p2))
			{
				reset_key_press = 1;
				reset_press_start_tick = clock_time();
			}
			else
			{
				reset_key_press = 0;
			}
		}
		else
		{
			pair_key_press = 0;
			reset_key_press = 0;
		}

	}

	if(reset_key_press && \
		clock_time_exceed(reset_press_start_tick , THREE_SECOND) && \
		Active_State_Machine != SM_RESET &&\
		reset_enable)
	{
		//TODO reset factory
#if BLE_AUDIO_ENABLE
		if(ui_mic_enable)
		{
			ui_enable_mic(0);
		}
#endif
		Start_New_Mode(SM_RESET);
		reset_enable = 0;
	}

	if(pair_key_press && \
		clock_time_exceed(pair_press_start_tick , THREE_SECOND) && \
		Active_State_Machine != SM_PAIR &&\
		force_pairing_enable)
	{
#if BLE_AUDIO_ENABLE
		if(ui_mic_enable)
		{
			ui_enable_mic(0);
		}
#endif
		Start_New_Mode(SM_PAIR);

		//prevent re-entering pair mode if user hold the force pairing keys and not release.
		//if key released this flag will be set.
		force_pairing_enable = 0;
	}

/*
 *
 * if key pressed for more than 30S, enter sleep
 *
 */
#if(STUCK_KEY_PROCESS_ENABLE)
	//if the key still pressed and time exceed 30s, sleep
	if(!key_not_released)
	{
		key_stuck_start_tick = clock_time();
	}

	if(key_not_released && clock_time_exceed(key_stuck_start_tick, STUCK_KEY_SLEEP_TIME) && !my_det_key)
	{
		//key release wakeup
		{
			send_release_code();

			for(int i=0; i<sizeof(pin) / sizeof(*pin); i++)
			{
				extern u8 stuckKeyPress[];
				if(stuckKeyPress[i])
				{
					cpu_set_gpio_wakeup(pin[i], 0, 1);			//low level wakeup deepsleep
				}
				else
				{
					cpu_set_gpio_wakeup(pin[i], 0, 0);
				}
			}
#if BLE_AUDIO_ENABLE
			if(ui_mic_enable)
			{
				ui_enable_mic(0);
			}
#endif
		}

		//enter sleep
		stuck_flag = 1;
		LED_OFF;
		enter_deep_sleep = 1;
	}
#endif

	switch (Active_State_Machine)
	{
	case SM_INIT:
		Handle_Init_Mode();
		break;
/*

	case SM_FACTORY:
		Handle_Factory_Mode();
		break;
*/

	case SM_PAIR:
		Handle_Pair_Mode();
		break;

	case SM_CONTROL:
		Handle_Control_Mode();
		break;
/*

	case SM_SLEEP:
		Handle_Sleep_Mode();
		break;
*/

	case SM_ADV:
		Handle_Adv_Mode();
		break;

	case SM_RESET:
		Handle_Reset_Mode();
		break;

/*
	case SM_OTA:
		Handle_OTA_Mode();
		break;


	case SM_EMC:
		Handle_EMC_Mode();
		break;
*/
	default:
		break;
	}

#if(DEEPBACK_FAST_KEYSCAN_ENABLE)
	if(deepback_key_state != DEEPBACK_KEY_IDLE)
	{
		deepback_post_proc();
	}
#endif
}


void send_release_code(void)
{
	//send release code
	if(key_type == KEY_TYPE_CONSUMER)
	{
		consumer_buf = 0;
		bls_att_pushNotifyData(HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_buf , 2);  //release
	}
	else if(key_type == KEY_TYPE_KEYBOARD)
	{
		key_buf[2] = 0;
		bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8); //release
	}
}

/////////////////////////////////////////////////////////////////////
// main loop flow
/////////////////////////////////////////////////////////////////////
u32 tick_loop;
//unsigned short battValue[20];


void main_loop (void)
{
	tick_loop ++;


	////////////////////////////////////// BLE entry /////////////////////////////////
	blt_sdk_main_loop();



	////////////////////////////////////// UI entry /////////////////////////////////
	#if (BLE_AUDIO_ENABLE)
		//blc_checkConnParamUpdate();
		if(ui_mic_enable){
			task_audio();
		}
	#endif

	#if (BATT_CHECK_ENABLE)
		if(lowBattDet_enable){
			battery_power_check();//whether or not entry battery check
		}
	#endif
	//lowBatt_alarmFlag =  //low battery detect

	proc_keyboard (0,0, 0);

	// device_led_process();

	blt_pm_proc();
}


#endif  //end of __PROJECT_8267_BLE_REMOTE__ || __PROJECT_8261_BLE_REMOTE__ || __PROJECT_8269_BLE_REMOTE__
