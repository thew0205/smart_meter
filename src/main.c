#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/modbus/modbus.h>
#include <zephyr/logging/log.h>

#include <zephyr/fs/fs.h>
#include <ff.h>

#include <zephyr/drivers/rtc.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor/pzem004t.h>

#define METER_TO_JSON_FORMAT ("\n=BEGIN={\"voltage\":%0.4f,\"current\": %0.4f,\"power\": %0.4f,\"energy\": %0.4f,\"freq\": %0.4f,\"pf\": %0.4f,\"timestamp\": \"%04d-%02d-%02dT%02d:%02d:%02d\"}==END==\n")

const struct device *const rtc = DEVICE_DT_GET(DT_NODELABEL(ds3231_rtc));

LOG_MODULE_REGISTER(smart_meter, LOG_LEVEL_DBG);

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct device *pzem = DEVICE_DT_GET(DT_NODELABEL(pzem));

static FATFS fat_fs;
/* mounting info */
static struct fs_mount_t mp = {
	.type = FS_FATFS,
	.fs_data = &fat_fs,
};

/*
 *  Note the fatfs library is able to mount only strings inside _VOLUME_STRS
 *  in ffconf.h
 */
static const char *disk_mount_pt = "/SD:";

static int mount_sd_card(void)
{
	/* raw disk i/o */
	static const char *disk_pdrv = "SD";
	uint64_t memory_size_mb;
	uint32_t block_count;
	uint32_t block_size;

	if (disk_access_init(disk_pdrv) != 0)
	{
		LOG_ERR("Storage init ERROR!");
		return -1;
	}

	mp.mnt_point = disk_mount_pt;

	int res = fs_mount(&mp);

	if (res == FR_OK)
	{
		LOG_INF("Disk mounted.\n");
	}
	else
	{
		LOG_ERR("Failed to mount disk \n");
		return -1;
	}
	return 0;
}

static int set_date_time(const struct device *rtc, const struct rtc_time tm)
{
	int ret = 0;

	ret = rtc_set_time(rtc, &tm);
	if (ret < 0)
	{
		LOG_ERR("Cannot write date time: %d\n", ret);
		return ret;
	}
	return ret;
}

static int get_date_time(const struct device *rtc, struct rtc_time *tm)
{
	int ret = 0;

	ret = rtc_get_time(rtc, tm);
	if (ret < 0)
	{
		LOG_WRN("Cannot read date time: %d\n", ret);
		return ret;
	}
	LOG_DBG("RTC date and time: %04d-%02d-%02d %02d:%02d:%02d\n", tm->tm_year + 1900,
			tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	return ret;
}

int main(void)
{
	int ret;
	bool led_state = true;

	if (!device_is_ready(pzem))
	{
		LOG_ERR("pzem device not ready.\n");
		return 0;
	}
	/* Check if the RTC is ready */
	if (!device_is_ready(rtc))
	{
		printk("Device is not ready\n");
		return 0;
	}
	if (!gpio_is_ready_dt(&led))
	{
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		return 0;
	}
	if (mount_sd_card())
	{
		LOG_ERR("Failed to mount SD card\n");
		return -1;
	}
	else
	{
		LOG_INF("Successfully mounted SD card\n");
	}

#ifdef SET_RTC_TIME_FROM_MACHINE
	const struct rtc_time tm = {
		.tm_year = 2024 - 1900,
		.tm_mon = 11 - 1,
		.tm_mday = 17,
		.tm_hour = 4,
		.tm_min = 19,
		.tm_sec = 0,
	};

	set_date_time(rtc, tm);
#endif

	struct sensor_value voltage;
	struct sensor_value current;
	struct sensor_value frequency;
	struct sensor_value energy;
	struct sensor_value power;
	struct sensor_value power_factor;
	struct rtc_time tm;

	// TODO: (matthew) calculate the maximum buffer required for printing.
	char logging_buffer[1000];
	while (1)
	{
		ret = sensor_sample_fetch(pzem);
		if (ret)
		{
			LOG_WRN("sensor_sample_fetch failed ret %d\n", ret);
			continue;
		}

		ret = get_date_time(rtc, &tm);
		if (ret)
		{
			LOG_WRN("Reading rtc time failed ret %d\n", ret);
			continue;
		}

		ret = sensor_channel_get(pzem, SENSOR_CHAN_VOLTAGE, &voltage);
		ret = sensor_channel_get(pzem, SENSOR_CHAN_CURRENT, &current);
		ret = sensor_channel_get(pzem, SENSOR_CHAN_POWER, &power);
		ret = sensor_channel_get(pzem, SENSOR_CHAN_FREQUENCY, &frequency);
		ret = sensor_channel_get(pzem, SENSOR_CHAN_PZEM004T_ENERGY, &power);
		ret = sensor_channel_get(pzem, SENSOR_CHAN_PZEM004T_POWER_FACTOR, &power_factor);
		int needed_size = snprintf(logging_buffer, 1000, METER_TO_JSON_FORMAT, sensor_value_to_float(&voltage), sensor_value_to_float(&current), sensor_value_to_float(&power), sensor_value_to_float(&energy), sensor_value_to_float(&frequency), sensor_value_to_float(&power_factor), tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

		char file_name_buffer[60];
		snprintf(file_name_buffer, sizeof(file_name_buffer), "/sd0/meter_data_%04d-%02d-%02d.json", tm.tm_year, tm.tm_mon, tm.tm_mday);

		struct fs_file_t data_filp;
		fs_file_t_init(&data_filp);

		ret = fs_open(&data_filp, file_name_buffer, FS_O_APPEND | FS_O_CREATE);
		if (ret)
		{
			LOG_ERR("%s -- failed to create file (err = %d)\n", __func__, ret);
			continue;
		}
		else
		{
			LOG_INF("%s - successfully created file\n", __func__);
		}

		ret = fs_write(&data_filp, logging_buffer, strlen(logging_buffer));
		if (ret < 0)
		{
			LOG_ERR("%s -- failed to write to file (err = %d)\n", __func__, ret);
			continue;
		}
		fs_close(&data_filp);
		k_sleep(K_MSEC(10000));
	}

	return 0;
}
