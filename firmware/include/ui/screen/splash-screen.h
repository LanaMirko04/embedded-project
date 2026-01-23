/*!
 * \file            splash-screen.h
 * \date            2026-01-23
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [lana.mirko@icloud.com]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           Splash screen class definition.
 */

#ifndef SPLASH_SCREEN_H
#define SPLASH_SCREEN_H

#include <cstddef>

#include "result.h"
#include "ui/screen/screen.h"

class SplashScreen : public Screen {
  public:
    Result create(void) override;
    void on_enter(void) override;
    void on_exit(void) override;

  private:
    static constexpr char TITLE[] = "Sdrumo";
    static constexpr std::size_t LETTERS = sizeof(TITLE) - 1U;

    lv_obj_t *labels[LETTERS]{};

    void start_anim();
    static void anim_y_cb(void *obj, int32_t v);
    static void anim_opa_cb(void *obj, int32_t v);
};

#endif /*! SPLASH_SCREEN_H */
