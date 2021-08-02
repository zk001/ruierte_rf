#ifndef __APP_H__
#define __APP_H__

typedef struct {
  u8  rf_len1;
  u16 vid;					// 5~6 vendor ID
  u32 pid;					// 7~10 product ID
  u8  control_key;			// 11 function control key
  u8  rf_seq_no; 			// 12 rf sequence total number, save this value in 3.3v analog register.

  u8 start_code;
  u8 function_code;
  u8 para0;
  u8 para1;
  u8 para2;
  u8 crc;

  u8 unused0;
  u8 unused1;
  u8 unused2;
}__attribute__((packed))rf_package_t;

#define KEY_NUANFENG_FENGWEN   KEY4
#if defined(RUIERTE_RF_JIENENG)
#define KEY_JIENENG            KEY12
#elif defined(RUIERTE_RF_CHUPAO)
#define KEY_CHUPAO             KEY12
#elif defined(RUIERTE_RF_CHUJUN)
#define KEY_CHUJUN             KEY12
#endif
#define KEY_FUXI_YIDONG         KEY8
#define KEY_TUNXI_YIDONG        KEY0
#define KEY_UP                  KEY10
#define KEY_DOWN                KEY6
#define KEY_DEC                 KEY1
#define KEY_PLUS                KEY3
#define KEY_TINGZHI             KEY2
#define KEY_SHUIWEN             KEY5
#define KEY_ZUOWEN              KEY7
#define KEY_DACHONGSHUI         KEY9
#define KEY_XIAOCHONGSHUI       KEY11

#define COMBIN_KEY_SETUP_TIME  (1000*16*1000)
#define COMBIN_TIME            (3000*16*1000)//ms2tick(3000)
#define COMBIN_KEY_LAST_TIME   (3000*16*1000)

#define SHORT_TIME  (3000*16*1000)//ms2tick(3000)
#define LONG_TIME   (3000*16*1000)//ms2tick(3000)
#define STUCK_TIME  (30000*16*1000)//ms2tick(30000)

#define KEY_PROCESS_TIME        5000//US
#define LED_UPDATE_PROCESS_TIME 10000//US
#define ID_Flash_Addr    0x020000 //address store id

#define SLEEP_WAIT_TIME  5000
#define LED_BRIGHT_LEVEL 80

#define PWM_PERIOD  1000 //US
#define PWM_ON_DUTY 500  //US50

#define PAIR_KEY_VALUE	0x55
#define MAX_EVENT 50//the number is the register_key_event call times
extern void app_init();

#endif
