/*!
 * \file            clock-screen.h
 * \date            2026-01-23
 * \author          Giulia Cristofolini [giulia.cristofolini@studenti.unitn.it]
 *                  Mirko Lana [lana.mirko@icloud.com]
 *                  Mattia Zagatti [mattia.zagatti@studenti.unitn.it]
 *
 * \brief           Clock screen class definition.
 */

#ifndef CLOCK_SCREEN_H
#define CLOCK_SCREEN_H

#include "ui/screen/screen.h"

class ClockScreen : public Screen {
  public:
    ClockScreen(void);
    ~ClockScreen(void) = default;

    void on_enter(void) override;
    void on_exit(void) override;

  private:
    // TODO: Implement screen callbacks and private members
};

#endif /*! CLOCK_SCREEN_H */
