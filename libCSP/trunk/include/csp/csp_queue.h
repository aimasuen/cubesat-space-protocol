#include <stdint.h>

#ifndef _CSP_QUEUE_H_
#define _CSP_QUEUE_H_

/* Blackfin/x86 on Linux */
#if defined(__i386__) || defined(__BFIN__)

#include <pthread.h>
#include "csp_pthread_queue.h"

#define CSP_QUEUE_EMPTY PTHREAD_QUEUE_EMPTY
#define CSP_QUEUE_FULL PTHREAD_QUEUE_FULL
#define CSP_QUEUE_OK PTHREAD_QUEUE_OK

typedef pthread_queue_t * csp_queue_handle_t;

#endif // __i386__ or __BFIN__

/* AVR/ARM on FreeRTOS */
#if defined(__AVR__) || defined(__ARM__)

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#define CSP_QUEUE_EMPTY errQUEUE_EMPTY
#define CSP_QUEUE_FULL errQUEUE_FULL
#define CSP_QUEUE_OK pdPASS

typedef xQueueHandle csp_queue_handle_t;

#endif // __AVR__ or __ARM__

csp_queue_handle_t csp_queue_create(int length, size_t item_size);
int csp_queue_enqueue(csp_queue_handle_t handle, void *value, int timeout);
int csp_queue_enqueue_isr(csp_queue_handle_t handle, void * value, signed char * task_woken);
int csp_queue_dequeue(csp_queue_handle_t handle, void *buf, int timeout);
int csp_queue_dequeue_isr(csp_queue_handle_t handle, void *buf, signed char * task_woken);
int csp_queue_size(csp_queue_handle_t handle);
int csp_queue_size_isr(csp_queue_handle_t handle);

#endif // _CSP_QUEUE_H_
