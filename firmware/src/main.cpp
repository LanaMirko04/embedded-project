/*!
 * \file            main.cpp
 * \date            2025-10-16
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [lana.mirko@icloud.com]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           CYD main program.
 */

#include "assert.h"
#include "esp_log.h"
#include "fsm.h"
#include "result.h"

static constexpr char TAG[] = "MAIN";

static Result dummy_action(void);
static Result setup_fsm(void);

extern "C" void app_main(void) {
    assert(setup_fsm() == Result::SUCCESS);

    while (true) { // main loop
    }
}

static Result dummy_action(void) {
    ESP_LOGW(TAG, "Running %s", __func__);
    return Result::SUCCESS;
}

static Result setup_fsm(void) {
    ESP_LOGW(TAG, "Running %s", __func__);
    Fsm &fsm = Fsm::get_instance();

    Result res = fsm.register_action(Fsm::State::INIT, dummy_action);
    if (res != Result::SUCCESS) {
        ESP_LOGE(TAG, "An error occurred while setting INIT action (%s) - %s", result_to_str(res), result_get_err_msg());
        return res;
    }

    res = fsm.register_action(Fsm::State::WIFI_CONNECTION, dummy_action);
    if (res != Result::SUCCESS) {
        ESP_LOGE(TAG, "An error occurred while setting WIFI_CONNECTION action (%s) - %s", result_to_str(res), result_get_err_msg());
        return res;
    }

    res = fsm.register_action(Fsm::State::FETCH_CONFIG, dummy_action);
    if (res != Result::SUCCESS) {
        ESP_LOGE(TAG, "An error occurred while setting FETCH_CONFIG action (%s) - %s", result_to_str(res), result_get_err_msg());
        return res;
    }

    res = fsm.register_action(Fsm::State::UPDATE_VIEW, dummy_action);
    if (res != Result::SUCCESS) {
        ESP_LOGE(TAG, "An error occurred while setting UPDATE_VIEW action (%s) - %s", result_to_str(res), result_get_err_msg());
        return res;
    }

    res = fsm.register_action(Fsm::State::ERROR, dummy_action);
    if (res != Result::SUCCESS) {
        ESP_LOGE(TAG, "An error occurred while setting ERROR action (%s) - %s", result_to_str(res), result_get_err_msg());
        return res;
    }

    return Result::SUCCESS;
}
