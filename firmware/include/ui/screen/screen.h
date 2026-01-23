/*!
 * \file            screen.h
 * \date            2026-01-23
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [lana.mirko@icloud.com]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           Screen virtual class definition
 */

#ifndef SCREEN_H
#define SCREEN_H

#include "result.h"
#include "lvgl.h"

class Screen {
  public:
    Screen(void) : root(nullptr) {
    }
    ~Screen(void) = default;

    virtual Result create(void);
    virtual void on_enter(void);
    virtual void on_exit(void);

    lv_obj_t *get_root(void) const {
        // I'm too lazy to create a new file only for this function.
        return this->root;
    }

  protected:
    lv_obj_t *root;
};

#endif /*! SCREEN_H */
