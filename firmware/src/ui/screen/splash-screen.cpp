/*!
 * \file            splash-screen.h
 * \date            2026-01-23
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [lana.mirko@icloud.com]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           Splash screen class definition.
 */

#include "ui/screen/splash-screen.h"
#include "lv_conf_internal.h"
#include "result.h"
#include "lvgl.h"

Result SplashScreen::create(void) {
    if (this->root) {
        return Result::ERROR_ALREADY_CREATED;
    }

    root = lv_obj_create(NULL);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    constexpr int start_x = 40;
    constexpr int base_y = 120;

    for (int i = 0; i < LETTERS; i++) {
        char buf[2] = { TITLE[i], '\0' };

        labels[i] = lv_label_create(root);
        lv_label_set_text(labels[i], buf);
        lv_obj_set_style_text_color(labels[i], lv_color_white(), 0);
        lv_obj_set_style_text_font(labels[i], LV_FONT_MONTSERRAT_28, 0);

        lv_obj_set_pos(labels[i], start_x + i * 24, base_y + 20);
        lv_obj_set_style_opa(labels[i], LV_OPA_0, 0);
    }

    return Result::SUCCESS;
}

void SplashScreen::on_enter(void) {
    start_anim();
    lv_scr_load(root);
}

void SplashScreen::on_exit(void) {
    lv_anim_del(NULL, anim_y_cb);
    lv_anim_del(NULL, anim_opa_cb);
}

void SplashScreen::start_anim() {
    constexpr int IN_TIME = 300;
    constexpr int OUT_TIME = 250;
    constexpr int WAIT_TIME = 600;
    constexpr int CASCADE_MS = 80;

    constexpr int LOOP_TIME =
        (LETTERS - 1) * CASCADE_MS +
        IN_TIME +
        WAIT_TIME +
        (LETTERS - 1) * CASCADE_MS +
        OUT_TIME;

    for (int i = 0; i < LETTERS; i++) {
        lv_obj_t *lbl = labels[i];
        int base_y = lv_obj_get_y(lbl) - 20;

        lv_anim_t a;

        /* Y animation (IN → OUT) */
        lv_anim_init(&a);
        lv_anim_set_var(&a, lbl);
        lv_anim_set_exec_cb(&a, anim_y_cb);
        lv_anim_set_values(&a, base_y + 20, base_y);
        lv_anim_set_time(&a, IN_TIME);
        lv_anim_set_delay(&a, i * CASCADE_MS);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
        lv_anim_set_playback_time(&a, OUT_TIME);
        lv_anim_set_playback_delay(&a, WAIT_TIME + (LETTERS - i - 1) * CASCADE_MS);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_repeat_delay(&a, LOOP_TIME);
        lv_anim_start(&a);

        /* Opacity animation */
        lv_anim_init(&a);
        lv_anim_set_var(&a, lbl);
        lv_anim_set_exec_cb(&a, anim_opa_cb);
        lv_anim_set_values(&a, LV_OPA_0, LV_OPA_COVER);
        lv_anim_set_time(&a, IN_TIME);
        lv_anim_set_delay(&a, i * CASCADE_MS);
        lv_anim_set_playback_time(&a, OUT_TIME);
        lv_anim_set_playback_delay(&a, WAIT_TIME + (LETTERS - i - 1) * CASCADE_MS);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_repeat_delay(&a, LOOP_TIME);
        lv_anim_start(&a);
    }
}

void SplashScreen::anim_y_cb(void *obj, int32_t v) {
    lv_obj_set_y(static_cast<lv_obj_t *>(obj), v);
}

void SplashScreen::anim_opa_cb(void *obj, int32_t v) {
    lv_obj_set_style_opa(static_cast<lv_obj_t *>(obj), v, 0);
}
