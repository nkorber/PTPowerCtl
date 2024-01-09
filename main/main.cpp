#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "esp_ota_ops.h"
#include "esp_app_desc.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "filter.h"


static const char *TAG = "Main";
// esp_adc_cal_characteristics_t adc1_chars;
adc_oneshot_unit_handle_t adc1_handle;
adc_cali_handle_t adc1_cali_chan0_handle = NULL;
bool do_calibration1_chan0=false;

BWFilter_LP_2_05_50 Temp1Filter;
int raw_value, filtered_value;
int mV = 9999;
TaskHandle_t xHandle = NULL;

/*---------------------------------------------------------------
        ADC Calibration
---------------------------------------------------------------*/
static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
            .default_vref = 0,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

/* ------------------------------
 *  Analog read and filter task
 * ------------------------------*/
void vAnalogRead( void * pvParameters )
{
    while (1) {
		ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &raw_value));
        filtered_value = Temp1Filter.execute(static_cast<uint16_t>(raw_value));
        if (do_calibration1_chan0) {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan0_handle, filtered_value, &mV));
        }
        vTaskDelay(20 / portTICK_PERIOD_MS);
   }
}


void setup (void){
	esp_log_level_set("*", ESP_LOG_DEBUG);
    //-------------ADC1 Init---------------//
	adc_oneshot_unit_init_cfg_t init_config1 = {
		.unit_id = ADC_UNIT_1,
        .clk_src = static_cast<adc_oneshot_clk_src_t>(0),
		.ulp_mode = ADC_ULP_MODE_DISABLE,
	};
	ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
    //-------------ADC1 Config---------------//
	adc_oneshot_chan_cfg_t config = {
		.atten = ADC_ATTEN_DB_11,
		.bitwidth = ADC_BITWIDTH_12,
	};
	ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config));
    //-------------ADC1 Calibration Init---------------//
    do_calibration1_chan0 = adc_calibration_init(ADC_UNIT_1, ADC_CHANNEL_0, ADC_ATTEN_DB_11, &adc1_cali_chan0_handle);

    xTaskCreate( vAnalogRead, "Analog Read", 1024, NULL, tskIDLE_PRIORITY, &xHandle );
  configASSERT( xHandle );

//	pc_setup();
}

extern "C" void app_main(void)
{
	const esp_app_desc_t *app_desc = esp_app_get_description();
	ESP_LOGI(TAG, "Project %s, version %s compiled: %s %s",
			app_desc->project_name, app_desc->version, app_desc->date, app_desc->time);
	setup ();
//    int i = 0;
//    bool status = false;
    while (1) {
//        gpio_set_level(LED_PIN, status);
//        status = !status;
		// int raw_value;
		// int mV = 9999;
		// ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &raw_value));
  //       if (do_calibration1_chan0) {
  //           ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan0_handle, raw_value, &mV));
  //           // ESP_LOGI(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, EXAMPLE_ADC1_CHAN0, voltage[0][0]);
  //       }

        printf("Hello from %s : raw %d, mV %d.\n", app_desc->project_name, filtered_value, mV);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
//        gpio_set_level(LED_PIN, status);
//        status = !status;
//        vTaskDelay(1000 / portTICK_PERIOD_MS);
//        printf("[%d] Hello world!\n", i);
//        i++;
    }
}

