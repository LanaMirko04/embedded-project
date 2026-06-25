/*!
 * \file            mutex_lock.h
 * \brief           RAII scoped lock for FreeRTOS semaphores.
 */

#ifndef MUTEX_LOCK_H
#define MUTEX_LOCK_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class MutexLock {
  public:
    explicit MutexLock(SemaphoreHandle_t m) : m_(m) {
        if (m_) xSemaphoreTake(m_, portMAX_DELAY);
    }
    ~MutexLock() {
        if (m_) xSemaphoreGive(m_);
    }
    MutexLock(const MutexLock &) = delete;
    MutexLock &operator=(const MutexLock &) = delete;

  private:
    SemaphoreHandle_t m_;
};

#endif /*! MUTEX_LOCK_H */
