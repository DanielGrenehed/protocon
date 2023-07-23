#include <stdint.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log.h>

#define SUCCESS 1
#define FAILURE 0
#define MS_REPORT 50

LOG_MODULE_REGISTER(main);

static const struct adc_dt_spec y_adc = ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 0);
static const struct adc_dt_spec x_adc = ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 1);

static const struct gpio_dt_spec sw = GPIO_DT_SPEC_GET(DT_NODELABEL(sw1), gpios);
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_NODELABEL(led0), gpios);

uint8_t config_io() {
	if (!adc_is_ready_dt(&y_adc) || !adc_is_ready_dt(&x_adc)) {
		LOG_ERR("ADC controller device not ready!");
		return FAILURE;
	}
	if (!adc_channel_setup_dt(&y_adc) || !adc_channel_setup_dt(&x_adc)) {
		LOG_ERR("ADC channel setup failed!");
		return FAILURE;
	}
	gpio_pin_configure_dt(&sw, GPIO_INPUT);
	gpio_pin_configure_dt(&led, GPIO_OUTPUT);
	return SUCCESS;
}

void set_led_intensity(uint8_t i) {
	gpio_pin_set_dt(&led, i);
}

static uint16_t adc_buf;
static struct adc_sequence adc_seq = {
	.buffer = &adc_buf,
	.buffer_size = sizeof(adc_buf),
};

int32_t read_adc(const struct adc_dt_spec * adc) {
	adc_sequence_init_dt(adc, &adc_seq);
	if (adc_read((*adc).dev, &adc_seq) < 0) {
		LOG_ERR("Could not read adc");
		return FAILURE;
	}
	return (*adc).channel_cfg.differential ? ((int32_t)((int16_t)adc_buf)) : ((int32_t) adc_buf);
}

void main() {

	uint8_t res = config_io();
	
	uint32_t last_report = 0;
	int32_t x = 0, y = 0;
	uint8_t intensity = 1;
	set_led_intensity(intensity);
	LOG_INF("SUCCESS!");
	while (1) {
	 	if (res == SUCCESS) {
			x += read_adc(&x_adc);
			y += read_adc(&y_adc);
		}

		if (last_report + MS_REPORT < k_uptime_get()) {
			LOG_INF("x: %i, y: %i, btn: %i", x, y, gpio_pin_get_dt(&sw));
			x = 0;
			y = 0;
			intensity = !intensity;
			set_led_intensity(intensity);
			last_report = k_uptime_get();
		}
	}
}

