/*
 *
 *  zephyr_ble_th
 * 
 * 	main.c
 * 
 * 	Demonstrates usin Electronut Labs hackaBLE Nordic nRF52832 dev board 
 *  with Zephy RTOS.
 * 
 * 	This example is adapted from Zephr RTOS sensor and bluetooth examples.
 * 
 * 	Mahesh Venkitachalam
 * 	electronut.in
 * 
 */

#include <zephyr.h>
#include <misc/printk.h>
#include <device.h>
#include <gpio.h>

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <misc/byteorder.h>
#include <sensor.h>

#include <settings/settings.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#define ENVIRONMENTAL_SENSING_SERVICE 0x181A
#define TEMPERATURE_CHARACTERISTIC 0x2A6E
#define HUMIDITY_CHARACTERISTIC 0x2A6F
#define PRESSURE_CHARACTERISTIC 0x2A6D

// sensor
static struct device* dev_bme280;

static struct bt_uuid_16 vnd_uuid = BT_UUID_INIT_16(ENVIRONMENTAL_SENSING_SERVICE);

static const struct bt_uuid_16 T_uuid = BT_UUID_INIT_16(TEMPERATURE_CHARACTERISTIC);
static const struct bt_uuid_16 H_uuid = BT_UUID_INIT_16(HUMIDITY_CHARACTERISTIC);
static const struct bt_uuid_16 P_uuid = BT_UUID_INIT_16(PRESSURE_CHARACTERISTIC);

static struct bt_gatt_ccc_cfg T_ccc_cfg[BT_GATT_CCC_MAX];
static struct bt_gatt_ccc_cfg H_ccc_cfg[BT_GATT_CCC_MAX];
static struct bt_gatt_ccc_cfg P_ccc_cfg[BT_GATT_CCC_MAX];

static u8_t bTNotify;
static u8_t bHNotify;
static u8_t bPNotify;

static void T_ccc_cfg_changed(const struct bt_gatt_attr *attr, u16_t value)
{
	bTNotify = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

static void H_ccc_cfg_changed(const struct bt_gatt_attr *attr, u16_t value)
{
	bHNotify = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

static void P_ccc_cfg_changed(const struct bt_gatt_attr *attr, u16_t value)
{
	bPNotify = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

/* Vendor Primary Service Declaration */
static struct bt_gatt_attr vnd_attrs[] = {
	/* Vendor Primary Service Declaration */
	BT_GATT_PRIMARY_SERVICE(&vnd_uuid),
	BT_GATT_CHARACTERISTIC(&T_uuid.uuid, 
					BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       	BT_GATT_PERM_READ,
			       	NULL, NULL, NULL),
    BT_GATT_CCC(T_ccc_cfg, T_ccc_cfg_changed),
    BT_GATT_CHARACTERISTIC(&H_uuid.uuid, 
					BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       	BT_GATT_PERM_READ,
			       	NULL, NULL, NULL),
	BT_GATT_CCC(H_ccc_cfg, H_ccc_cfg_changed),
	BT_GATT_CHARACTERISTIC(&P_uuid.uuid, 
					BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       	BT_GATT_PERM_READ,
			       	NULL, NULL, NULL),
	BT_GATT_CCC(P_ccc_cfg, P_ccc_cfg_changed)
};

static struct bt_gatt_service vnd_svc = BT_GATT_SERVICE(vnd_attrs);

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x1a, 0x18),
};

static void connected(struct bt_conn *conn, u8_t err)
{
	if (err) {
		printk("Connection failed (err %u)\n", err);
	} else {
		printk("Connected\n");
	}
}

static void disconnected(struct bt_conn *conn, u8_t reason)
{
	printk("Disconnected (reason %u)\n", reason);
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	bt_gatt_service_register(&vnd_svc);

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Advertising successfully started\n");
}

float g_temp, g_humid, g_press;
int16_t temp_field;
uint16_t humid_field;
uint32_t press_field;

// print BME280 data
void update_sensor_data()
{
    // get sensor data
    struct sensor_value temp, press, humidity;

    sensor_sample_fetch(dev_bme280);
    sensor_channel_get(dev_bme280, SENSOR_CHAN_AMBIENT_TEMP, &temp);	
    sensor_channel_get(dev_bme280, SENSOR_CHAN_PRESS, &press);
    sensor_channel_get(dev_bme280, SENSOR_CHAN_HUMIDITY, &humidity);

    printk("temp: %d.%06d degC; press: %d.%06d hPa?; humidity: %d.%06d %%RH\n",
            temp.val1, temp.val2, press.val1, press.val2,
            humidity.val1, humidity.val2);

	g_temp = temp.val1 + (temp.val2/(float)1000000);
	g_humid = humidity.val1 + (humidity.val2/(float)1000000);
	g_press = (press.val1 + (press.val2/(float)1000000))*100; // 1 hPa = 100 Pa

	temp_field = (int16_t)(g_temp*100);
	humid_field = (uint16_t)(g_humid*100);
	press_field = (uint32_t)(g_press*10); // https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Characteristics/org.bluetooth.characteristic.pressure.xml
}


void main(void)
{
	struct device* port0 = device_get_binding("GPIO_0");
	/* Set LED pin as output */
    gpio_pin_configure(port0, 17, GPIO_DIR_OUT);

	// flash  LED
	gpio_pin_write(port0, 17, 0);
	k_sleep(500);
	gpio_pin_write(port0, 17, 1);
	k_sleep(500);

	// sensor
	dev_bme280 = device_get_binding("BME280");
	printk("dev %p name %s\n", dev_bme280, dev_bme280->config->name);

	k_sleep(500);
	update_sensor_data();

    // set up BLE
	int err;
	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	bt_conn_cb_register(&conn_callbacks);
	
	while (1) {
		k_sleep(2*MSEC_PER_SEC);

		// update 
		update_sensor_data();

		printk("temp_field: %d; humid_field: %d; press_field: %d\n", temp_field, humid_field, press_field);
		
		// notify 
		bt_gatt_notify(NULL, &vnd_attrs[2], &temp_field, sizeof(temp_field));
        bt_gatt_notify(NULL, &vnd_attrs[4], &humid_field, sizeof(humid_field));
		bt_gatt_notify(NULL, &vnd_attrs[6], &press_field, sizeof(press_field));

		// update adv data
		bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);
	}
}

