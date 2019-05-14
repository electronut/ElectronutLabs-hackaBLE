#include "sdk_common.h"
#include "ble_cus.h"
#include <string.h>
#include "ble_srv_common.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "nrf_log.h"

#include "epaper.h"

unsigned char epaper_bwData[EPD_WIDTH * EPD_HEIGHT / 8];

#define RED_LED 19
#define GREEN_LED 18
#define BLUE_LED 17

void leds_off()
{
	nrf_gpio_pin_set(RED_LED);
    nrf_gpio_pin_set(GREEN_LED);
    nrf_gpio_pin_set(BLUE_LED);
}

/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_cus_t * p_cus, ble_evt_t const * p_ble_evt)
{
    p_cus->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

    ble_cus_evt_t evt;

    evt.evt_type = BLE_CUS_EVT_CONNECTED;

    p_cus->evt_handler(p_cus, &evt);
}

/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_cus_t * p_cus, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_cus->conn_handle = BLE_CONN_HANDLE_INVALID;
    
    ble_cus_evt_t evt;

    evt.evt_type = BLE_CUS_EVT_DISCONNECTED;

    p_cus->evt_handler(p_cus, &evt);
}

/**@brief Function for handling the Write event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_write(ble_cus_t * p_cus, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    
    // Custom Value Characteristic Written to.
    for(int i=0; i<NO_OF_CHARACTERISTCS; i++)
    {
        uint8_t value[EPAPER_CHARACTERISTC_VALUE_LENGTH];
        if (p_evt_write->handle == p_cus->custom_value_handles[i].value_handle)
        {
            for(int i=0; i<p_evt_write->len; i++)
            {
                // NRF_LOG_RAW_INFO("%x ", p_evt_write->data[i]);
                value[i] = p_evt_write->data[i];
            }

            int data_length = (p_evt_write->len)-2;
            static int frame_no;
            int tmp_frame_no = ((value[0]<<8) | value[1]);
            bool last_frame = false;

            static uint32_t green_led_state = 1;

            if(tmp_frame_no==0) // end frame
            {
                frame_no++;
                last_frame = true;
                NRF_LOG_RAW_INFO("last frame\r\n");

                NRF_LOG_RAW_INFO("%d : ", frame_no);
                for (int i = 0; i < 8; i++) { //last buffer size = 8
                    epaper_bwData[((frame_no-1)*data_length)+i] = value[i+2];
                    NRF_LOG_RAW_INFO("%02x", value[i]);
                }
                NRF_LOG_RAW_INFO("\r\n");

                leds_off();
                nrf_gpio_pin_clear(BLUE_LED);
            }
            else
            {
                frame_no = tmp_frame_no;
                NRF_LOG_RAW_INFO("%d : ", frame_no);
                for (int i = 0; i < data_length; i++) {
                    epaper_bwData[((frame_no-1)*data_length)+i] = value[i+2];
                    NRF_LOG_RAW_INFO("%02x", value[i]);
                }
                NRF_LOG_RAW_INFO("\r\n");

                leds_off();
                green_led_state = green_led_state ? 0:1;
                nrf_gpio_pin_write(GREEN_LED, green_led_state);
            }

            if(last_frame)
            {
                Paint_DrawBitmap(&paint_black, 0, 0, epaper_bwData, EPD_WIDTH, EPD_HEIGHT, COLORED);
                EPD_DisplayFrame(&epd, frame_buffer_black, frame_buffer_red);
            }
        }
    }

    for(int i=0; i<NO_OF_CHARACTERISTCS; i++)
    {
        // Check if the Custom value CCCD is written to and that the value is the appropriate length, i.e 2 bytes.
        if ((p_evt_write->handle == p_cus->custom_value_handles[i].cccd_handle)
            && (p_evt_write->len == 2)
        )
        {
            // CCCD written, call application event handler
            if (p_cus->evt_handler != NULL)
            {
                ble_cus_evt_t evt;

                if (ble_srv_is_notification_enabled(p_evt_write->data))
                {
                    evt.evt_type = BLE_CUS_EVT_NOTIFICATION_ENABLED;
                }
                else
                {
                    evt.evt_type = BLE_CUS_EVT_NOTIFICATION_DISABLED;
                }
                // Call the application event handler.
                p_cus->evt_handler(p_cus, &evt);
            }
        }
    }
}

void ble_cus_on_ble_evt( ble_evt_t const * p_ble_evt, void * p_context)
{
    ble_cus_t * p_cus = (ble_cus_t *) p_context;
    
    // NRF_LOG_INFO("BLE event received. Event type = %d\r\n", p_ble_evt->header.evt_id); 
    if (p_cus == NULL || p_ble_evt == NULL)
    {
        return;
    }
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_cus, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_cus, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_cus, p_ble_evt);
            break;
/* Handling this event is not necessary
        case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
            NRF_LOG_INFO("EXCHANGE_MTU_REQUEST event received.\r\n");
            break;
*/
        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for adding the Custom Value characteristic.
 *
 * @param[in]   p_cus        Battery Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t custom_value_char_add(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init, uint16_t UUID_chararc_no,
                                uint8_t read_status, uint8_t write_status, uint8_t characteristics_no, uint8_t size_of_charac)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    // Add Custom Value characteristic
    memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    
    cccd_md.write_perm = p_cus_init->custom_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = read_status;
    char_md.char_props.write  = write_status;
    char_md.char_props.notify = 1; 
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md; 
    char_md.p_sccd_md         = NULL;
		
    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = UUID_chararc_no;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->custom_value_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = size_of_charac;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = size_of_charac;

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->custom_value_handles[characteristics_no]);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

uint32_t ble_cus_init(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    if (p_cus == NULL || p_cus_init == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure
    p_cus->evt_handler               = p_cus_init->evt_handler;
    p_cus->conn_handle               = BLE_CONN_HANDLE_INVALID;

    // Add Custom Service UUID
    ble_uuid128_t base_uuid = {CUSTOM_SERVICE_UUID_BASE};
    err_code =  sd_ble_uuid_vs_add(&base_uuid, &p_cus->uuid_type);
    VERIFY_SUCCESS(err_code);
    
    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = CUSTOM_SERVICE_UUID;

    // Add the Custom Service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_cus->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Custom Value characteristic
    custom_value_char_add(p_cus, p_cus_init, EPAPER_CHARACTERISTC_UUID, 1, 1, 0, 18);
    return 0;
}

uint32_t ble_cus_custom_value_update(ble_cus_t * p_cus, uint8_t charac_no, uint8_t *charac_value, uint16_t * charac_len)
{
    NRF_LOG_INFO("In ble_cus_custom_value_update. \r\n");
    // if (p_cus == NULL)
    // {
    //     return NRF_ERROR_NULL;
    // }

    uint32_t err_code = NRF_SUCCESS;

    // Send value if connected and notifying.
    if ((p_cus->conn_handle != BLE_CONN_HANDLE_INVALID)) 
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_cus->custom_value_handles[charac_no].value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        // hvx_params.offset = gatts_value.offset;
        // hvx_params.p_len  = &gatts_value.len;
        // hvx_params.p_data = gatts_value.p_value;
        
        hvx_params.p_data = charac_value;
        hvx_params.p_len  = charac_len;

        err_code = sd_ble_gatts_hvx(p_cus->conn_handle, &hvx_params);
        NRF_LOG_INFO("sd_ble_gatts_hvx result: %x. \r\n", err_code);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
        NRF_LOG_INFO("sd_ble_gatts_hvx result: NRF_ERROR_INVALID_STATE. \r\n");
    }


    return err_code;
}
