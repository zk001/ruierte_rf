#include "../../drivers.h"
#include "../../genfsk_ll/genfsk_ll.h"
#include "../../common/mempool.h"
#include "key.h"
#include "rf.h"
#include "app.h"
#include "led.h"
#include "dc.h"
#include "n_timer.h"
#include "board.h"
#include "mac_id.h"
#include "gpio_key.h"
#include "gpio_led.h"
#include "power_saving.h"
#include "timer_clock.h"
#include "wakeup.h"

MEMPOOL_DECLARE(KEY_EVENT_POOL, KEY_EVENT_POOL_MEM, sizeof(mem_block_t) + sizeof(event_handler_t), MAX_EVENT);
_attribute_data_retention_ key_map_t key_arry[MAX_GPIO_KEYS] = {//117
  {ROW0, COL0, IS_WAKE_UP},//KEY_TUNXI_YIDONG
  {ROW0, COL1, IS_WAKE_UP},//KEY_DEC
  {ROW0, COL2, IS_WAKE_UP},//KEY_TINGZHI
  {ROW0, COL3, IS_WAKE_UP},//KEY_PLUS
  {ROW1, COL0, IS_WAKE_UP},//KEY_NUANFENG_FENGWEN
  {ROW1, COL1, IS_WAKE_UP},//KEY_SHUIWEN
  {ROW1, COL2, IS_WAKE_UP},//KEY_DOWN
  {ROW1, COL3, IS_WAKE_UP},//KEY_ZUOWEN
  {ROW2, COL0, IS_WAKE_UP},//KEY_FUXI_YIDONG
  {ROW2, COL1, IS_WAKE_UP},//KEY_DACHONGSHUI
  {ROW2, COL2, IS_WAKE_UP},//KEY_UP
  {ROW2, COL3, IS_WAKE_UP},//KEY_XIAOCHONGSHUI
  {ROW3, COL0, IS_WAKE_UP}//KEY_JIENENG
};

const u32 led_arry[MAX_GPIO_LEDS] = {
  LED5, LED4, LED3, LED2, LED1
};

const hal_led_t led_enum_arry[MAX_LEDS] = {
  {HAL_LED_1, gpio_led_on_off, NULL},
  {HAL_LED_2, gpio_led_on_off, NULL},
  {HAL_LED_3, gpio_led_on_off, NULL},
  {HAL_LED_4, gpio_led_on_off, NULL},
  {HAL_LED_5, gpio_led_on_off, NULL},
};

const key_type_t key_enum_arry[MAX_KEYS] = {
  {KEY0,  MECHANICAL_KEY, gpio_key_init, gpio_key_low_scan, gpio_stuck_key_low_scan},
  {KEY1,  MECHANICAL_KEY, gpio_key_init, gpio_key_low_scan, gpio_stuck_key_low_scan},
  {KEY2,  MECHANICAL_KEY, gpio_key_init, gpio_key_low_scan, gpio_stuck_key_low_scan},
  {KEY3,  MECHANICAL_KEY, gpio_key_init, gpio_key_low_scan, gpio_stuck_key_low_scan},
  {KEY4,  MECHANICAL_KEY, gpio_key_init, gpio_key_low_scan, gpio_stuck_key_low_scan},
  {KEY5,  MECHANICAL_KEY, gpio_key_init, gpio_key_low_scan, gpio_stuck_key_low_scan},
  {KEY6,  MECHANICAL_KEY, gpio_key_init, gpio_key_low_scan, gpio_stuck_key_low_scan},
  {KEY7,  MECHANICAL_KEY, gpio_key_init, gpio_key_low_scan, gpio_stuck_key_low_scan},
  {KEY8,  MECHANICAL_KEY, gpio_key_init, gpio_key_low_scan, gpio_stuck_key_low_scan},
  {KEY9,  MECHANICAL_KEY, gpio_key_init, gpio_key_low_scan, gpio_stuck_key_low_scan},
  {KEY10, MECHANICAL_KEY, gpio_key_init, gpio_key_low_scan, gpio_stuck_key_low_scan},
  {KEY11, MECHANICAL_KEY, gpio_key_init, gpio_key_low_scan, gpio_stuck_key_low_scan},
  {KEY12, MECHANICAL_KEY, gpio_key_init, gpio_key_low_scan, gpio_stuck_key_low_scan},
};

int main(void)
{
  blc_pm_select_internal_32k_crystal();

  cpu_wakeup_init();

  clock_init(SYS_CLK_24M_Crystal);

  gpio_init(1);

  dc_power_on();

  pwm_set_clk(CLOCK_SYS_CLOCK_HZ, CLOCK_SYS_CLOCK_HZ);

  if(!is_wakeup_from_sleep()){
    gpio_key_alloc(key_arry, MAX_GPIO_KEYS);
    register_key(key_enum_arry, MAX_KEYS);

    gpio_led_alloc(led_arry, MAX_GPIO_LEDS);
    register_led(led_enum_arry, MAX_LEDS);
  }

  if(!is_wakeup_from_sleep()){
    key_init();
    HalLedInit();
  }else
    key_wakeup_init();

  key_process(NULL);

  rf_8359_set_tx();

  if(!is_wakeup_from_sleep())
    id_init();

  mempool_init(&KEY_EVENT_POOL ,&KEY_EVENT_POOL_MEM[0], sizeof(mem_block_t) + sizeof(event_handler_t), MAX_EVENT);

  app_init();

  ev_on_timer(key_process, NULL, KEY_PROCESS_TIME);

  ev_on_timer(HalLedUpdate, NULL, LED_UPDATE_PROCESS_TIME);

  if(!is_wakeup_from_sleep())
    idle_time_for_sleep(SLEEP_WAIT_TIME);

  reload_sys_time();

  while(1){
    poll_key_event();

    ev_process_timer();

    if(poll_idle_time()){
      HalLedSet(HAL_LED_ALL, HAL_LED_MODE_OFF);
      gpio_key_sleep_setup();
      set_wakeup_flag();
      dc_shutdown();
      cpu_sleep_wakeup(DEEPSLEEP_MODE_RET_SRAM_LOW32K, PM_WAKEUP_PAD, 0);
    }
  }
}
