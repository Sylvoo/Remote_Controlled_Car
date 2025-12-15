#include "joystick.h"

#include "esp_log.h"
#include "esp_err.h"

#include "esp_adc/adc_oneshot.h"
#include "hal/adc_types.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

static const char *TAG3 = "ADC";

adc_oneshot_unit_handle_t adc1_handle;
adc_cali_handle_t cali_handle = NULL;
bool cali_enabled = false;

void adc_calibration_init(adc_unit_t unit, adc_atten_t atten )
{
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = unit,
        .atten = atten,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    if (adc_cali_create_scheme_line_fitting(&cali_config, &cali_handle) == ESP_OK) {
        cali_enabled = true;
        ESP_LOGI(TAG3, "ADC calibration enabled");
    } else {
        cali_enabled = false;
        ESP_LOGW(TAG3, "ADC calibration not available, using raw readings");
    }
}

void adc_joystick_init(void)
{
    adc_oneshot_unit_init_cfg_t init_cfg = { // tworzymy strukture poczatkowa adc
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
        // .clk_src domyslnie ok 
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &adc1_handle));

    adc_oneshot_chan_cfg_t init_chan = { // struktura konfiguracyjna channeli adc
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, JOY_X_CH ,&init_chan));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, JOY_Y_CH ,&init_chan));

    // kalibracja
    adc_calibration_init(ADC_UNIT_1, init_chan.atten);
}

void adc_deinit(void)
{
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));
}

joystick_adc_t joystick_read()
{
    joystick_adc_t xy_data = {0};
    int raw_x = 0;
    int raw_y = 0;
    xy_data.err_x = adc_oneshot_read(adc1_handle, JOY_X_CH, &raw_x);
    xy_data.err_y = adc_oneshot_read(adc1_handle, JOY_Y_CH, &raw_y);

    if(xy_data.err_x == ESP_OK && xy_data.err_y == ESP_OK)
    {
        if(cali_enabled)
        {
            adc_cali_raw_to_voltage(cali_handle, raw_x, &xy_data.x);
            adc_cali_raw_to_voltage(cali_handle, raw_y, &xy_data.y);
        }
        else
        {
            xy_data.x = raw_x;
            xy_data.y = raw_y;
        }
    }
    return xy_data;
}
