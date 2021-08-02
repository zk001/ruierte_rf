#include "../../drivers.h"
#include "../../genfsk_ll/genfsk_ll.h"
#include "key.h"
#include "app.h"
#include "led.h"
#include "key.h"
#include "board.h"
#include "mac_id.h"
#include "rf.h"
#include "n_timer.h"
#include "main.h"
#include "power_saving.h"
#include "wakeup.h"

#define MAX_FUNCTION 40

typedef enum {
  TUNXI = 0,
  FUXI,
  HONGGAN,
  SHUIYA,
  PENZUI_WEIZHI,
  SHUIWEN,
  ZUOWEN,
  TINGZHI,
  QUANPAI_CHONGSHUI,
  ZIDONG_CHONGSHUI,
  ZIDONG_FANGGAI,
  ZHINENG_YEDING,
  JINGYING,
  DUIMA,
  FANGGAI_CHUSHIHUA,
  MAICHONG_ANMO,
  CHUCHOU,
  JIEDIAN_24_XIAOSHI,
  YEDENG_KAIGUAN,
  BANPAI_CHONGSHUI,
  CHUJUN,
  YUSHIRUN,
  GANYINGJULI,
  APP_PEIMA,
  SHUIXIANG_PAISHUI,
  JIAOGAN_KAIGUAN,
  DACHONG_TIAOJIE,
  XIAOCHONG_TIAOJIE,
  YUYIN_KAIGUAN,
  JIAOGAN_ZHISHIDENG,
  CHONGSHUA_MOSHI3,
  CHONGSHUA_MOSHI4,
  ZUIQUAN_KAIGUAN,
  SHANGGAI_KAIGUAN,
  PAOMODUN_CHUPAO,
  ZIDONG_CHUPAO,
  YEBENG_CHOUYE,
  PAOMODUN_BEIYONG,
  ZHANTING_MOSHI,
}function_t;

#define CHONGSHUI_PLUS 0x01
#define CHONGSHUI_DEC  0xff

_attribute_data_retention_ static u32 uid;
_attribute_data_retention_ static u32 new_id;

static u8 chongshui_para0;

const u8 function_code[MAX_FUNCTION]= {
  0x01,
  0x02,
  0x03,
  0x04,
  0x06,
  0x08,
  0x09,
  0x0C,
  0x0F,
  0x10,
  0x14,
  0x15,
  0x16,
  0x17,
  0x1C,
  0x1D,
  0x1F,
  0x21,
  0x22,
  0x23,
  0x24,
  0x26,
  0x2C,
  0x32,
  0x34,
  0x36,
  0x37,
  0x38,
  0x39,
  0x3B,
  0x3D,
  0x3E,
  0x41,
  0x42,
  0x44,
  0x45,
  0x46,
  0X47,
  0X4C
};

static u8 sum(u8 temp)
{
  u8 result = 0;

  for(u8 i = 0; i < 8; i++){
    if(temp & 0x01)
      result++;
    temp >>= 1;
  }

  return result;
}

_attribute_data_retention_ struct {
  u8 shuiya;
  u8 daogang;
  u8 fengwen;
  u8 shuiwen;
  u8 zuowen;
  u8 anmo;
}user_value __attribute__ ((aligned (4)));

typedef enum{
  SHUIYA_INDEX = 1,
  DAOGANG_INDEX,
  FENGWEN_INDEX,
  SHUIWEN_INDEX,
  ZUOWEN_INDEX,
  ANMO_INDEX
}user_en_t;

static void set_user_value(user_en_t u_val, u8 val)
{
  switch(u_val){
    case SHUIYA_INDEX:  user_value.shuiya  = val;break;
    case DAOGANG_INDEX: user_value.daogang = val;break;
    case FENGWEN_INDEX: user_value.fengwen = val;break;
    case SHUIWEN_INDEX: user_value.shuiwen = val;break;
    case ZUOWEN_INDEX:  user_value.zuowen  = val;break;
    case ANMO_INDEX:    user_value.anmo    = val;break;
    default:break;
  }
}

static u8 get_user_value(user_en_t u_val)
{
  u8 r_val;
  switch(u_val){
    case SHUIYA_INDEX:  r_val = user_value.shuiya;break;
    case DAOGANG_INDEX: r_val = user_value.daogang;break;
    case FENGWEN_INDEX: r_val = user_value.fengwen;break;
    case SHUIWEN_INDEX: r_val = user_value.shuiwen;break;
    case ZUOWEN_INDEX:  r_val = user_value.zuowen;break;
    case ANMO_INDEX:    r_val = user_value.anmo;break;
    default:r_val = 0;break;
  }

  return r_val;
}

static void set_default_user_value()
{
  user_value.shuiya    = 2;
  user_value.daogang   = 3;
  user_value.fengwen   = 3;
  user_value.shuiwen   = 3;
  user_value.zuowen    = 3;
  user_value.anmo      = 0;
}

static void fix_pack_with_user_value(rf_package_t *rf_pack, function_t fn)
{
  u8 fun = fn;
  u8 para0 = 0;
  u8 para1 = 0;
  u8 para2 = 0;

  switch(fun){
    case TUNXI:
    case FUXI:
      para0     = ((user_value.daogang << 4) & 0xf0)  | (user_value.shuiya  & 0x0f);
      para1     = ((user_value.anmo    << 4) & 0xf0)  | (user_value.shuiwen & 0x0f);
      para2     = ((user_value.zuowen  << 4) & 0xf0)  | (user_value.fengwen & 0x0f);
      break;
    case HONGGAN:
      para0     = (user_value.fengwen << 4 & 0xf0);
      para1     = 0;
      para2     = 0;
      break;
    case SHUIYA:
      para0     = (user_value.shuiya << 4 & 0xf0);
      para1     = 0;
      para2     = 0;
      break;
    case PENZUI_WEIZHI:
      para0     = (user_value.daogang << 4 & 0xf0);
      para1     = 0;
      para2     = 0;
      break;
    case SHUIWEN:
      para0     = (user_value.shuiwen << 4 & 0xf0);
      para1     = 0;
      para2     = 0;
      break;
    case ZUOWEN:
      para0     = (user_value.zuowen << 4 & 0xf0);
      para1     = 0;
      para2     = 0;break;
    case DUIMA:
      para0     = 0;
      para1     = 0;
      para2     = 0;
      break;
    case GANYINGJULI:
      para0     = 0;
      para1     = 0;
      para2     = 0;
      break;
    case DACHONG_TIAOJIE:
      para0     = chongshui_para0;
      para1     = 0;
      para2     = 0;
      break;
    case XIAOCHONG_TIAOJIE:
      para0     = chongshui_para0;
      para1     = 0;
      para2     = 0;
      break;
    default:break;
  };

  rf_pack->rf_len1     = 0;
  rf_pack->vid         = 0x5453;

  if(fn == DUIMA){
    gen_random_id(&new_id);
    rf_pack->pid           = new_id;
    rf_pack->control_key   = PAIR_KEY_VALUE;

    rf_pack->rf_seq_no     = 0;

    rf_pack->start_code    = 0;
    rf_pack->function_code = 0;
    rf_pack->para0         = 0;
    rf_pack->para1         = 0;
    rf_pack->para2         = 0;

    rf_pack->crc     = 0;

    rf_pack->unused0 = 0;
    rf_pack->unused1 = 0;
    rf_pack->unused2 = 0;
  }else{
    rf_pack->pid           = uid;
    rf_pack->control_key   = 0;

    rf_pack->rf_seq_no     = 0;

    rf_pack->start_code    = 0x50;
    rf_pack->function_code = function_code[fun];

    rf_pack->para0         = para0;
    rf_pack->para1         = para1;
    rf_pack->para2         = para2;

    rf_pack->crc = sum(0x50)+\
                   sum(rf_pack->function_code)+\
                   sum(rf_pack->para0)+\
                   sum(rf_pack->para1)+\
                   sum(rf_pack->para2);

    rf_pack->unused0 = 0;
    rf_pack->unused1 = 0;
    rf_pack->unused2 = 0;
  }
}

static void power_on_led()
{
  HalLedBlink (HAL_LED_ALL, 1, 100, MS2TICK(3000));
}

static inline void set_led_level_on(u32 led)
{
  u32 ledoff;

  ledoff = led ^ HAL_LED_ALL;

  HalLedSet (ledoff, HAL_LED_MODE_OFF);

  HalLedBlink (led, 1, 100, MS2TICK(5000));
}

static u8 set_led_level(u8 user_value)
{
  u8 leds;

  switch(user_value){
    case 0: leds = 0;  break;
    case 1: leds = HAL_LED_1;break;
    case 2:	leds = HAL_LED_1 | HAL_LED_2;break;
    case 3: leds = HAL_LED_1 | HAL_LED_2 | HAL_LED_3;break;
    case 4: leds = HAL_LED_1 | HAL_LED_2 | HAL_LED_3 | HAL_LED_4;break;
    case 5: leds = HAL_LED_1 | HAL_LED_2 | HAL_LED_3 | HAL_LED_4 | HAL_LED_5;break;
    default: leds = 0;break;
  }
  return leds;
}

static u8 set_shuiya_led_level(u8 user_value)
{
  u8 leds;

  switch(user_value){
    case 0: leds = 0;  break;
    case 1: leds = HAL_LED_1;break;
    case 2: leds = HAL_LED_1 | HAL_LED_2 | HAL_LED_3;break;
    case 3: leds = HAL_LED_1 | HAL_LED_2 | HAL_LED_3 | HAL_LED_4 | HAL_LED_5;break;
    default: leds = 0;break;
  }

  return leds;
}

//"key action" "which key" "cmd-code"
static void short_key_tunxi_yidong_tunxi()
{
  rf_package_t rf_pack;
  u8 shuiwen;
  u8 led;

  fix_pack_with_user_value(&rf_pack, TUNXI);

  shuiwen = get_user_value(SHUIWEN_INDEX);

  led = set_led_level(shuiwen);

  set_led_level_on(led);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void short_key_fuxi_yidong_fuxi()
{
  rf_package_t rf_pack;
  u8 leds;
  u8 shuiwen;

  shuiwen = get_user_value(SHUIWEN_INDEX);

  leds = set_led_level(shuiwen);

  set_led_level_on(leds);

  fix_pack_with_user_value(&rf_pack, FUXI);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void long_key_tunxi_yidong_maichong_anmo()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, MAICHONG_ANMO);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void long_key_fuxi_yidong_maichong_anmo()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, MAICHONG_ANMO);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void short_key_jia_shuiya()
{
  rf_package_t rf_pack;
  u8 leds;
  u8 shuiya;

  shuiya = get_user_value(SHUIYA_INDEX);

  shuiya++;

  if(shuiya == 4)
    shuiya = 3;

  set_user_value(SHUIYA_INDEX, shuiya);

  leds = set_shuiya_led_level(shuiya);

  set_led_level_on(leds);

  fix_pack_with_user_value(&rf_pack, SHUIYA);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void short_key_jian_shuiya()
{
  rf_package_t rf_pack;
  u8 leds;
  u8 shuiya;

  shuiya = get_user_value(SHUIYA_INDEX);

  shuiya--;

  if(shuiya == 0)
    shuiya = 1;

  set_user_value(SHUIYA_INDEX, shuiya);

  leds = set_shuiya_led_level(shuiya);

  set_led_level_on(leds);

  fix_pack_with_user_value(&rf_pack, SHUIYA);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void short_key_up_penzui_weizhi()
{
  rf_package_t rf_pack;
  u8 leds;
  u8 daogang;

  daogang = get_user_value(DAOGANG_INDEX);

  daogang++;

  if(daogang == 6)
    daogang = 5;

  set_user_value(DAOGANG_INDEX, daogang);

  leds = set_led_level(daogang);

  set_led_level_on(leds);

  fix_pack_with_user_value(&rf_pack, PENZUI_WEIZHI);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void short_key_down_penzui_weizhi()
{
  rf_package_t rf_pack;
  u8 leds;
  u8 daogang;

  daogang = get_user_value(DAOGANG_INDEX);

  daogang--;

  if(daogang == 0)
    daogang = 1;

  set_user_value(DAOGANG_INDEX, daogang);

  leds = set_led_level(daogang);

  set_led_level_on(leds);

  fix_pack_with_user_value(&rf_pack, PENZUI_WEIZHI);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void short_key_tingzhi_tingzhi()
{
  rf_package_t rf_pack;

  HalLedSet (HAL_LED_ALL, HAL_LED_MODE_OFF);

  fix_pack_with_user_value(&rf_pack, TINGZHI);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

#if (defined(RUIERTE_RF_CHUPAO) || defined(RUIERTE_RF_CHUJUN))
void long_key_tingzhi_jieneng()
{
  rf_package_t rf_pack;

  HalLedSet (HAL_LED_ALL, HAL_LED_MODE_OFF);

  fix_pack_with_user_value(&rf_pack, JIEDIAN_24_XIAOSHI);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}
#endif

static void short_key_shuiwen_shuiwen()
{
  rf_package_t rf_pack;
  u8 leds;
  u8 shuiwen;

  shuiwen = get_user_value(SHUIWEN_INDEX);

  shuiwen++;

  if(shuiwen == 6)
    shuiwen = 0;

  set_user_value(SHUIWEN_INDEX, shuiwen);

  leds = set_led_level(shuiwen);

  set_led_level_on(leds);

  fix_pack_with_user_value(&rf_pack, SHUIWEN);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void short_key_zuowen_zuowen()
{
  rf_package_t rf_pack;
  u8 leds;
  u8 zuowen;

  zuowen = get_user_value(ZUOWEN_INDEX);

  zuowen++;

  if(zuowen == 6)
    zuowen = 0;

  set_user_value(ZUOWEN_INDEX, zuowen);

  leds = set_led_level(zuowen);

  set_led_level_on(leds);

  fix_pack_with_user_value(&rf_pack, ZUOWEN);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void short_key_xiaochongshui_banpai_chongshui()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, BANPAI_CHONGSHUI);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void short_key_dachong_quanpai_chongshui()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, QUANPAI_CHONGSHUI);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

#if defined(RUIERTE_RF_JIENENG)
static void short_key_jieneng_jieneng()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, JIEDIAN_24_XIAOSHI);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}
#endif

#if defined(RUIERTE_RF_CHUPAO)
void short_key_chupao_chupao()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, PAOMODUN_CHUPAO);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}
#endif

#if defined(RUIERTE_RF_CHUPAO)
void combin_key_tingzhi_chupao_zidongchupao()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, ZIDONG_CHUPAO);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));

}
#endif

#if defined(RUIERTE_RF_CHUPAO)
void combin_key_shuiwen_chupao_paomodun_beiyong()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, PAOMODUN_BEIYONG);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}
#endif

void combin_key_shuiwen_plus_zhanting_moshi()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, ZHANTING_MOSHI);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

#if defined(RUIERTE_RF_CHUPAO)
void long_key_chupao_yebengchouye()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, YEBENG_CHOUYE);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}
#endif

#if defined(RUIERTE_RF_CHUJUN)
void short_key_chujun_chujun()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, CHUJUN);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}
#endif

static void long_key_dachongshui_zidongchongshui()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, ZIDONG_CHONGSHUI);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void combin_key_tingzhi_tunxi_yidong_yonghuduima()
{
  rf_package_t rf_pack;
  u32 send_period;
  u8 rx_buf[32] = {0};

  HalLedBlink (HAL_LED_ALL, 30, 50, MS2TICK(1000));

  fix_pack_with_user_value(&rf_pack, DUIMA);

  decrease_rf_power_tx();

  send_period = clock_time();

  while(1){
    if(!HalLedUpdate(NULL)){//if time exceed 15s, then return
      rf_8359_set_tx();
      clr_app_read_key_flag();
      reload_sys_time();
      return;
    }

    if(n_clock_time_exceed(send_period, 1000000)){//1S
      decrease_rf_power_tx();
      send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
      rf_8359_set_rx();
      send_period = clock_time();
    }

    if(receive_rf_data(rx_buf)){
      if(rx_buf[7] == 0x88){//received peer code
        rf_8359_set_tx();
        write_id(&new_id, 4);
        uid = new_id;
        clr_app_read_key_flag();
        HalLedBlink (HAL_LED_ALL, 1, 100, MS2TICK(2000));
        reload_sys_time();
        return;
      }
    }

    if(app_read_key(KEY_TINGZHI, KEY_TUNXI_YIDONG)){
      rf_8359_set_tx();
      HalLedSet (HAL_LED_ALL, HAL_LED_MODE_OFF);
      reload_sys_time();
      return;
    }
  }
}

static void long_key_shuiwen_yedeng_kaiguan()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, YEDENG_KAIGUAN);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void long_key_zuowen_zhinengyedeng()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, ZHINENG_YEDING);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void combin_key_tingzhi_fuxi_jingyin()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, JINGYING);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void short_key_nuanfeng_fengwen_honggan()
{
  rf_package_t rf_pack;
  u8 fenwen;
  u8 led;

  fenwen = get_user_value(FENGWEN_INDEX);

  if((pre_key == KEY_NUANFENG_FENGWEN) && (!is_wakeup_from_sleep())){
    fenwen++;

    if(fenwen == 6)
      fenwen = 0;
  }else if(is_wakeup_from_sleep())
    clr_wakeup_flag();

  set_user_value(FENGWEN_INDEX, fenwen);

  led = set_led_level(fenwen);

  set_led_level_on(led);

  fix_pack_with_user_value(&rf_pack, HONGGAN);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void combin_key_tingzhi_nuanfeng_chuchou()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, CHUCHOU);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

#if defined(RUIERTE_RF_JIENENG) || defined(RUIERTE_RF_CHUPAO)
static void long_key_nuanfeng_fengwen_chujun()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, CHUJUN);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}
#endif

static void combin_key_tingzhi_dachongshui_yushirun()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, YUSHIRUN);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void long_key_xiaochongshui_jiaogan_kaiguan()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, JIAOGAN_KAIGUAN);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void combin_key_shuiwen_zuowen_app_peima()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, APP_PEIMA);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void combin_key_zuowen_xiaochong_yuyin_kaiguan()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, YUYIN_KAIGUAN);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void combin_key_tingzhi_shuiwen_chongshua_moshi3()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, CHONGSHUA_MOSHI3);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void combin_key_tingzhi_zuowen_chongshua_moshi4()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, CHONGSHUA_MOSHI4);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void combin_key_tingzhi_xiaochongshui_jiaoganzhishi()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, JIAOGAN_ZHISHIDENG);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void combin_key_xiaochongshui_dachonggshui_shuixiangpaishui()
{
  rf_package_t rf_pack;

  fix_pack_with_user_value(&rf_pack, SHUIXIANG_PAISHUI);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void combin_key_dachongshui_plus_dacongtiaojie_plus()
{
  rf_package_t rf_pack;

  chongshui_para0 = CHONGSHUI_PLUS;

  fix_pack_with_user_value(&rf_pack, DACHONG_TIAOJIE);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void combin_key_dachongshui_dec_dacongtiaojie_dec()
{
  rf_package_t rf_pack;

  chongshui_para0 = CHONGSHUI_DEC;

  fix_pack_with_user_value(&rf_pack, DACHONG_TIAOJIE);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void combin_key_xiaochongshui_plus_xiaochongtiaojie_plus()
{
  rf_package_t rf_pack;

  chongshui_para0 = CHONGSHUI_PLUS;

  fix_pack_with_user_value(&rf_pack, XIAOCHONG_TIAOJIE);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

static void combin_key_xiaochongshui_dec_xiaochongtiaojie_dec()//bug
{
  rf_package_t rf_pack;

  chongshui_para0 = CHONGSHUI_DEC;

  fix_pack_with_user_value(&rf_pack, XIAOCHONG_TIAOJIE);

  send_rf_data_ruierte(&rf_pack, sizeof(rf_pack));
}

void app_init()
{
  if(!is_wakeup_from_sleep()){
    power_on_led();
    set_default_user_value();
    read_id(&uid, 4);
  }

  register_key_event(KEY_NUANFENG_FENGWEN, 0, MS2TICK(3000), 0, SHORT_KEY, short_key_nuanfeng_fengwen_honggan);
  register_key_event(KEY_TUNXI_YIDONG,     0, MS2TICK(3000), 0, SHORT_KEY, short_key_tunxi_yidong_tunxi);
  register_key_event(KEY_FUXI_YIDONG,      0, MS2TICK(3000), 0, SHORT_KEY, short_key_fuxi_yidong_fuxi);
  register_key_event(KEY_TUNXI_YIDONG,     0, MS2TICK(4000), 0, LONG_KEY,  long_key_tunxi_yidong_maichong_anmo);
  register_key_event(KEY_FUXI_YIDONG,      0, MS2TICK(4000), 0, LONG_KEY,  long_key_fuxi_yidong_maichong_anmo);

  register_key_event(KEY_DEC,     0, 0, 0, SHORT_KEY_IMMEDIATELY, short_key_jian_shuiya);
  register_key_event(KEY_PLUS,    0, 0, 0, SHORT_KEY_IMMEDIATELY, short_key_jia_shuiya);
  register_key_event(KEY_UP,      0, 0, 0, SHORT_KEY_IMMEDIATELY, short_key_up_penzui_weizhi);
  register_key_event(KEY_DOWN,    0, 0, 0, SHORT_KEY_IMMEDIATELY, short_key_down_penzui_weizhi);

  register_key_event(KEY_TINGZHI, 0, MS2TICK(3000), 0, SHORT_KEY, short_key_tingzhi_tingzhi);

#if (defined(RUIERTE_RF_CHUPAO) || defined(RUIERTE_RF_CHUJUN))
  register_key_event(KEY_TINGZHI, 0, MS2TICK(3000), 0, LONG_KEY, long_key_tingzhi_jieneng);
#endif

  register_key_event(KEY_SHUIWEN, 0, MS2TICK(3000), 0, SHORT_KEY, short_key_shuiwen_shuiwen);
  register_key_event(KEY_ZUOWEN,  0, MS2TICK(3000), 0, SHORT_KEY, short_key_zuowen_zuowen);
  register_key_event(KEY_XIAOCHONGSHUI, 0, MS2TICK(3000), 0, SHORT_KEY, short_key_xiaochongshui_banpai_chongshui);
  register_key_event(KEY_DACHONGSHUI,   0, MS2TICK(3000), 0, SHORT_KEY, short_key_dachong_quanpai_chongshui);

#if defined(RUIERTE_RF_JIENENG)
  register_key_event(KEY_JIENENG,      0, MS2TICK(3000), 0, SHORT_KEY, short_key_jieneng_jieneng);
#endif

#if defined(RUIERTE_RF_CHUJUN)
  register_key_event(KEY_CHUJUN,       0, MS2TICK(3000), 0, SHORT_KEY, short_key_chujun_chujun);
#endif

#if defined(RUIERTE_RF_CHUPAO)
  register_key_event(KEY_CHUPAO,       0, MS2TICK(3000), 0, SHORT_KEY, short_key_chupao_chupao);
  register_key_event(KEY_CHUPAO,       0, MS2TICK(3000), 0, LONG_KEY,  long_key_chupao_yebengchouye);
  register_key_event(KEY_TINGZHI, KEY_CHUPAO,  0, MS2TICK(3000), COMBIN_KEY,  combin_key_tingzhi_chupao_zidongchupao);
#endif

  register_key_event(KEY_TINGZHI, KEY_TUNXI_YIDONG, 0, MS2TICK(3000), COMBIN_KEY, combin_key_tingzhi_tunxi_yidong_yonghuduima);

#if defined(RUIERTE_RF_CHUPAO)
  register_key_event(KEY_SHUIWEN,   KEY_CHUPAO,  MS2TICK(1000), MS2TICK(3000), COMBIN_KEY_IN_TIME, combin_key_shuiwen_chupao_paomodun_beiyong);
  register_key_event(KEY_CHUPAO,    KEY_SHUIWEN, MS2TICK(1000), MS2TICK(3000), COMBIN_KEY_IN_TIME, combin_key_shuiwen_chupao_paomodun_beiyong);
#endif

  register_key_event(KEY_SHUIWEN, KEY_PLUS, MS2TICK(1000), MS2TICK(3000), COMBIN_KEY_IN_TIME, combin_key_shuiwen_plus_zhanting_moshi);

  register_key_event(KEY_DACHONGSHUI, 0,  MS2TICK(4000), 0, LONG_KEY, long_key_dachongshui_zidongchongshui);
  register_key_event(KEY_SHUIWEN,     0,  MS2TICK(4000), 0, LONG_KEY, long_key_shuiwen_yedeng_kaiguan);
  register_key_event(KEY_ZUOWEN,      0,  MS2TICK(4000), 0, LONG_KEY, long_key_zuowen_zhinengyedeng);

  register_key_event(KEY_TINGZHI, KEY_FUXI_YIDONG,      0, MS2TICK(3000), COMBIN_KEY, combin_key_tingzhi_fuxi_jingyin);

  register_key_event(KEY_TINGZHI, KEY_NUANFENG_FENGWEN, 0, MS2TICK(3000), COMBIN_KEY, combin_key_tingzhi_nuanfeng_chuchou);

#if defined(RUIERTE_RF_JIENENG) || defined(RUIERTE_RF_CHUPAO)
  register_key_event(KEY_NUANFENG_FENGWEN, 0, MS2TICK(4000), 0, LONG_KEY, long_key_nuanfeng_fengwen_chujun);
#endif

  register_key_event(KEY_TINGZHI, KEY_DACHONGSHUI, 0, MS2TICK(3000), COMBIN_KEY, combin_key_tingzhi_dachongshui_yushirun);

  register_key_event(KEY_XIAOCHONGSHUI, 0, MS2TICK(4000), 0, LONG_KEY, long_key_xiaochongshui_jiaogan_kaiguan);

  register_key_event(KEY_SHUIWEN, KEY_ZUOWEN, MS2TICK(1000), MS2TICK(3000), COMBIN_KEY_IN_TIME, combin_key_shuiwen_zuowen_app_peima);
  register_key_event(KEY_ZUOWEN, KEY_SHUIWEN, MS2TICK(1000), MS2TICK(3000), COMBIN_KEY_IN_TIME, combin_key_shuiwen_zuowen_app_peima);

  register_key_event(KEY_ZUOWEN, KEY_XIAOCHONGSHUI, MS2TICK(1000), MS2TICK(3000), COMBIN_KEY_IN_TIME, combin_key_zuowen_xiaochong_yuyin_kaiguan);
  register_key_event(KEY_XIAOCHONGSHUI, KEY_ZUOWEN, MS2TICK(1000), MS2TICK(3000), COMBIN_KEY_IN_TIME, combin_key_zuowen_xiaochong_yuyin_kaiguan);

  register_key_event(KEY_TINGZHI, KEY_SHUIWEN, MS2TICK(1000), MS2TICK(3000), COMBIN_KEY_IN_TIME, combin_key_tingzhi_shuiwen_chongshua_moshi3);
  register_key_event(KEY_SHUIWEN, KEY_TINGZHI, MS2TICK(1000), MS2TICK(3000), COMBIN_KEY_IN_TIME, combin_key_tingzhi_shuiwen_chongshua_moshi3);

  register_key_event(KEY_TINGZHI, KEY_ZUOWEN, MS2TICK(1000), MS2TICK(3000), COMBIN_KEY_IN_TIME, combin_key_tingzhi_zuowen_chongshua_moshi4);
  register_key_event(KEY_ZUOWEN, KEY_TINGZHI, MS2TICK(1000), MS2TICK(3000), COMBIN_KEY_IN_TIME, combin_key_tingzhi_zuowen_chongshua_moshi4);

  register_key_event(KEY_TINGZHI, KEY_XIAOCHONGSHUI, 0, MS2TICK(3000), COMBIN_KEY, combin_key_tingzhi_xiaochongshui_jiaoganzhishi);
  register_key_event(KEY_XIAOCHONGSHUI, KEY_TINGZHI, 0, MS2TICK(3000), COMBIN_KEY, combin_key_tingzhi_xiaochongshui_jiaoganzhishi);

  register_key_event(KEY_XIAOCHONGSHUI, KEY_DACHONGSHUI, MS2TICK(1000), MS2TICK(3000), COMBIN_KEY_IN_TIME, combin_key_xiaochongshui_dachonggshui_shuixiangpaishui);
  register_key_event(KEY_DACHONGSHUI, KEY_XIAOCHONGSHUI, MS2TICK(1000), MS2TICK(3000), COMBIN_KEY_IN_TIME, combin_key_xiaochongshui_dachonggshui_shuixiangpaishui);

  register_key_event(KEY_DACHONGSHUI,   KEY_PLUS, 0, MS2TICK(3000), COMBIN_KEY, combin_key_dachongshui_plus_dacongtiaojie_plus);
  register_key_event(KEY_DACHONGSHUI,   KEY_DEC,  0, MS2TICK(3000), COMBIN_KEY, combin_key_dachongshui_dec_dacongtiaojie_dec);
  register_key_event(KEY_XIAOCHONGSHUI, KEY_PLUS, 0, MS2TICK(3000), COMBIN_KEY, combin_key_xiaochongshui_plus_xiaochongtiaojie_plus);
  register_key_event(KEY_XIAOCHONGSHUI, KEY_DEC,  0, MS2TICK(3000), COMBIN_KEY, combin_key_xiaochongshui_dec_xiaochongtiaojie_dec);
}
