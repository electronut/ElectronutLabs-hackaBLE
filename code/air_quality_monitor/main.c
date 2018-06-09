/*
  bluey beacon
  This puts bluey in non-connectable advertising-only mode with no scan response.
  The ad packets send out temperature, humidity and ambient light levels.
  Electronut Labs
  electronut.in
*/

#include <stdbool.h>
#include <stdint.h>
#include "ble_advdata.h"
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "bsp.h"
#include "app_timer.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "app_uart.h"
#include "nrf_delay.h"

#include <stdlib.h>

#include "nrf_delay.h"
#include "nrf_drv_twi.h"
#include "bluey_twi.h"
#include "nrf_drv_clock.h"

#define CENTRAL_LINK_COUNT              0                                 /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT           0                                 /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define IS_SRVC_CHANGED_CHARACT_PRESENT 0                                 /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

#define APP_CFG_NON_CONN_ADV_TIMEOUT    0                                 /**< Time for which the device must be advertising in non-connectable mode (in seconds). 0 disables timeout. */
#define NON_CONNECTABLE_ADV_INTERVAL    MSEC_TO_UNITS(2000, UNIT_0_625_MS) /**< The advertising interval for non-connectable advertisement (100 ms). This value can vary between 100ms to 10.24s). */
#define BEACON_INTERVAL      APP_TIMER_TICKS(2000, APP_TIMER_PRESCALER)

#define DEAD_BEEF                       0xDEADBEEF                        /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256

#define APP_TIMER_PRESCALER             0                                 /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE         4                                 /**< Size of timer operation queues. */

#define DEVICE_NAME "AirQM"

static ble_gap_adv_params_t m_adv_params;

// temperature & humidty sensor initialization
uint8_t temperature = 0;
uint8_t humidity = 0;
// dust sensor values initialisation
static uint16_t pm0_1=0, pm2_5=0, pm10=0;
//define dht11 functions and variables
void DHT11_DIR_INPUT(void);
void DHT11_DIR_OUTPUT(void);
void DHT11_read(void);
uint8_t data[5]={0,0,0,0,0};
uint32_t DHT11_ONE_WIRE_INPUT_PIN = 15;
// declare advertisement data
void set_adv_data(bool init);


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


APP_TIMER_DEF(m_beacon_timer_id);

//APP_TIMER_DEF(timer_ms);

// one-shot flag for setting adv data
static volatile bool g_setAdvData = false;

/**@brief Function for handling the Battery measurement timer timeout.
 *
 * @details This function will be called each time the battery level measurement timer expires.
 *
 * @param[in] p_context  Pointer used for passing some arbitrary information (context) from the
 *                       app_start_timer() call to the timeout handler.
 */
static void beacon_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    // set update data flag
    g_setAdvData = true;
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
    uint32_t err_code;// err_code1, err_code2;

    // Initialize timer module.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);

    // Create timers.
    err_code = app_timer_create(&m_beacon_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                beacon_timeout_handler);
    
   // err_code1 = app_timer_create(&timer_ms, APP_TIMER_MODE_SINGLE_SHOT, timer_handler);
    APP_ERROR_CHECK(err_code);
}
// handler for timer in ms

/*static void timer_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
}*/

/**@brief Function for starting application timers.
 */
static void application_timers_start(void)
{
    uint32_t err_code;

    // Start application timers.
    err_code = app_timer_start(m_beacon_timer_id, BEACON_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}
// set sensor values
uint8_t checkValue(uint8_t *databuf, uint8_t index)
{
    uint8_t flag=0;
    uint16_t sum=0;
    //uint16_t recSum;

    for( uint8_t i = 0; i < 21; i++)
    {
        sum=sum+databuf[i];
    }

    uint16_t check = (databuf[22]<<8)+databuf[23];
    if(sum == check)
    {
        flag=1;
    }

    return flag;
}

uint16_t transmitPM2_5(uint8_t *thebuf)
{
  uint16_t PM2_5Val;
  PM2_5Val=(thebuf[6]<<8) + thebuf[7];//count PM2.5 value of the air detector module
  return PM2_5Val;
}

uint16_t transmitPM0_1(uint8_t *thebuf)
{
  uint16_t  PM0_1Val;
  PM0_1Val=(thebuf[4]<<8) + thebuf[5];//count PM2.5 value of the air detector module
  return PM0_1Val;
}

uint16_t transmitPM10(uint8_t *thebuf)
{
  uint16_t PM10Val;
  PM10Val=(thebuf[8]<<8) + thebuf[9];//count PM2.5 value of the air detector module
  return PM10Val;
}


// uart event handler for reading sensor data from buffer

void uart_event_handle(app_uart_evt_t * p_event)
{
    static uint8_t data_array[24];
    uint8_t ch;
    static uint8_t index = 0;
    //uint32_t       err_code;
    static int state = 0;
    if(p_event->evt_type == APP_UART_DATA_READY)
    {
        UNUSED_VARIABLE(app_uart_get(&ch));
        switch(state)
        {
            case 0:
            if(ch == 0x42)
            {
                state = 1;
                index = 0;
                data_array[index++] = ch;
            }
            break;
            case 1:
            if(ch == 0x4d)
            {
                state = 2;
                data_array[index++] = ch;
            }
            break;
            case 2:
            data_array[index++] = ch;
            if (index == 24)
            {
                 state = 0;
                if(checkValue(data_array,index)==1)
                {
                 pm0_1=transmitPM0_1(data_array);
                 pm2_5=transmitPM2_5(data_array);
                 pm10=transmitPM10(data_array);
                 NRF_LOG_INFO("pm0_1=%d pm2_5=%d pm10=%d\r\n",pm0_1,pm2_5,pm10);
                }
                 
            }
            break;
        }
    }

    switch (p_event->evt_type)
    {
        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);
            app_uart_flush();
            break;

        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
}

// initialise the uart
static void uart_init(void)
{
    uint32_t                     err_code;
    app_uart_comm_params_t const comm_params =
    {
        .rx_pin_no    = 27,
        .tx_pin_no    = 26,
        .rts_pin_no   = RTS_PIN_NUMBER,
        .cts_pin_no   = CTS_PIN_NUMBER,
        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
        .use_parity   = false,
        .baud_rate    = UART_BAUDRATE_BAUDRATE_Baud9600
    };

    APP_UART_FIFO_INIT(&comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handle,
                       APP_IRQ_PRIORITY_LOWEST,
                       err_code);
    APP_ERROR_CHECK(err_code);
}

// -----------------get data from dht11-------------------------------
//config pin as input
void DHT11_DIR_INPUT(void)
{
				nrf_gpio_cfg_input(DHT11_ONE_WIRE_INPUT_PIN, NRF_GPIO_PIN_PULLUP);
}
//config pin as output
void DHT11_DIR_OUTPUT(void)
{
				nrf_gpio_cfg_output(DHT11_ONE_WIRE_INPUT_PIN);
				//DHT11_ONE_WIRE_DIR = 1;
}

uint32_t expectPulse(bool level) 
{
  uint32_t count = 0;
  // Otherwise fall back to using digitalRead (this seems to be necessary on ESP8266
  // right now, perhaps bugs in direct port access functions?).
  
    while (nrf_gpio_pin_read(DHT11_ONE_WIRE_INPUT_PIN) == level) {
      if (count++ >= 2555) {
        return 0; // Exceeded timeout, fail.
      }
    }

  return count;
}

//initiate dht11 to giveout data
void DHT11_read(void) 
{
  // Check if sensor was read less than two seconds ago and return early
  // to use last reading.
  // Reset 40 bits of received data to zero.
  data[0] = data[1] = data[2] = data[3] = data[4] = 0;

  // Go into high impedence state to let pull-up raise data line level and
  // start the reading process.
  DHT11_DIR_OUTPUT();
  nrf_gpio_pin_write(DHT11_ONE_WIRE_INPUT_PIN, 1);
  nrf_delay_ms(250);

  // First set data line low for 20 milliseconds.
  nrf_gpio_pin_write(DHT11_ONE_WIRE_INPUT_PIN, 0);
  nrf_delay_ms(20);

  uint32_t cycles[80];
  {

    // End the start signal by setting data line high for 40 microseconds.
    nrf_gpio_pin_write(DHT11_ONE_WIRE_INPUT_PIN,1);
    nrf_delay_us(40);

    // Now start reading the data line to get the value from the DHT sensor.
    DHT11_DIR_INPUT();
    nrf_delay_us(10);  // Delay a bit to let sensor pull data line low.

    // First expect a low signal for ~80 microseconds followed by a high signal
    // for ~80 microseconds again.
    if (expectPulse(0) == 0) {
        printf("80_low wrong!\n");
      //_lastresult = false;
      //return _lastresult;
    }
    
    if (expectPulse(1) == 0) {
         printf("80_high wrong!\n");
       //_lastresult = false;
       //return _lastresult;
    }

    // Now read the 40 bits sent by the sensor.  Each bit is sent as a 50
    // microsecond low pulse followed by a variable length high pulse.  If the
    // high pulse is ~28 microseconds then it's a 0 and if it's ~70 microseconds
    // then it's a 1.  We measure the cycle count of the initial 50us low pulse
    // and use that to compare to the cycle count of the high pulse to determine
    // if the bit is a 0 (high state cycle count < low state cycle count), or a
    // 1 (high state cycle count > low state cycle count). Note that for speed all
    // the pulses are read into a array and then examined in a later step.
    for (int i=0; i<80; i+=2) 
    {
      cycles[i]   = expectPulse(0);
      cycles[i+1] = expectPulse(1);
    }
  } // Timing critical code is now complete.

  // Inspect pulses and determine which ones are 0 (high state cycle count < low
  // state cycle count), or 1 (high state cycle count > low state cycle count).
    for (int i=0; i<40; ++i)  
    {
        uint32_t lowCycles  = cycles[2*i];
        uint32_t highCycles = cycles[2*i+1];
        if ((lowCycles == 0) || (highCycles == 0))  
        {
            printf("Timeout waiting for pulse.\n\n");
             printf("data_levs wrong!\n");
           // _lastresult = false;
            //return _lastresult;
         }
    data[i/8] <<= 1;
    // Now compare the low and high cycle times to see if the bit is a 0 or 1.
    if (highCycles > lowCycles) {
      // High cycles are greater than 50us low cycle count, must be a 1.
      data[i/8] |= 1;
    }
    // Else high cycles are less than (or equal to, a weird case) the 50us low
    // cycle count so this must be a zero.  Nothing needs to be changed in the
    // stored data.
  }

  printf("Received:\r\n");
  printf("%d",data[0]); printf(", ");
  printf("%d",data[1]); printf(", ");
  printf("%d",data[2]); printf(", ");
  printf("%d",data[3]); printf(", ");
  printf("%d",data[4]); printf(" =? ");
  printf("%d\r\n",(data[0] + data[1] + data[2] + data[3]) & 0xFF);

  // Check we read 40 bits and that the checksum matches.
  if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
    humidity = data[0];
    temperature = data[2];
  }
  else {
         printf("Checksum failure!\n");
             // _lastresult = false;
              // return _lastresult;
       }
}

// set adv data
void set_adv_data(bool init)
{
  uint32_t      err_code;

  ble_advdata_t advdata;
  uint8_t       flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;

  ble_advdata_manuf_data_t        manuf_data; // Variable to hold manufacturer specific data
  // Initialize with easily identifiable data
  uint8_t data[]                      = {
    0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8};

  // get sensor data
  if (!init)
  {
      DHT11_read();
    data[0] = pm0_1>>8;
    data[1] = pm0_1;
    data[2] = pm2_5>> 8;
    data[3] = pm2_5;
    data[4] = pm10 >> 8;
    data[5] = pm10;
    data[6] = temperature;
    data[7] = humidity;
    
  }


  manuf_data.company_identifier       = 0xFFFF;
  manuf_data.data.p_data              = data;
  manuf_data.data.size                = sizeof(data);

  // Build advertising data struct to pass into @ref ble_advertising_init.
  memset(&advdata, 0, sizeof(advdata));

  advdata.name_type               = BLE_ADVDATA_SHORT_NAME;
  advdata.short_name_len          = 5;
  advdata.flags                   = flags; //BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
  advdata.p_manuf_specific_data   = &manuf_data;
  // if you don't set tx power, you get 3 extra bytes for manuf data
  //int8_t tx_power                 = -4;
  //advdata.p_tx_power_level        = &tx_power;
  advdata.include_appearance      = true;

  err_code = ble_advdata_set(&advdata, 0);
  APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
static void advertising_init(void)
{
    uint32_t      err_code;


    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    set_adv_data(true);

    // Initialize advertising parameters (used when starting advertising).
    memset(&m_adv_params, 0, sizeof(m_adv_params));

    m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_NONCONN_IND;
    m_adv_params.p_peer_addr = NULL;                             // Undirected advertisement.
    m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval    = NON_CONNECTABLE_ADV_INTERVAL;
    m_adv_params.timeout     = APP_CFG_NON_CONN_ADV_TIMEOUT;
}

/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    uint32_t err_code;

    err_code = sd_ble_gap_adv_start(&m_adv_params);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;

    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
    APP_ERROR_CHECK(err_code);

    //Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,PERIPHERAL_LINK_COUNT);

    // Enable BLE stack.
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for doing power management.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

const nrf_drv_twi_t p_twi_sensors = NRF_DRV_TWI_INSTANCE(0);


// main loop begins 
int main(void)
{
    uint32_t err_code;
    // Initialize.
    err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);
    err_code = bsp_init(BSP_INIT_LED, APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), NULL);
    APP_ERROR_CHECK(err_code);
    ble_stack_init();
    advertising_init();
    timers_init();
    uart_init();

    // Start execution.
    NRF_LOG_INFO("BLE Beacon started\r\n");
    advertising_start();

    application_timers_start();

    // Enter main loop.
    for (;; )
    {
      // one-shot flag to set adv data
     if(g_setAdvData) {
        set_adv_data(false);
        g_setAdvData = false;
      }
      power_manage();
    }
}


/**
 * @}
 */
