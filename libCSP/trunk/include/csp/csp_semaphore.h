/*
Cubesat Space Protocol - A small network-layer protocol designed for Cubesats
Copyright (C) 2010 Gomspace ApS (gomspace.com)
Copyright (C) 2010 AAUSAT3 Project (aausat3.space.aau.dk) 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _CSP_SEMAPHORE_H_
#define _CSP_SEMAPHORE_H_

#include <stdint.h>
#include <csp/csp.h>

/* POSIX interface */
#if defined(__CSP_POSIX__)

#include <pthread.h>
#include <semaphore.h>

#define CSP_SEMAPHORE_OK 1
#define CSP_SEMAPHORE_ERROR 2

#define CSP_ENTER_CRITICAL()
#define CSP_EXIT_CRITICAL()

typedef sem_t csp_bin_sem_handle_t;

#endif // __CSP_POSIX__

/* FreeRTOS interface */
#if defined(__CSP_FREERTOS__)

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#define CSP_SEMAPHORE_OK pdPASS
#define CSP_SEMAPHORE_ERROR pdFAIL

#define CSP_ENTER_CRITICAL() portENTER_CRITICAL()
#define CSP_EXIT_CRITICAL() portEXIT_CRITICAL()

typedef xSemaphoreHandle csp_bin_sem_handle_t;

#endif // __CSP_FREERTOS__

int csp_bin_sem_create(csp_bin_sem_handle_t *sem);
int csp_bin_sem_wait(csp_bin_sem_handle_t *sem, int timeout);
int csp_bin_sem_post(csp_bin_sem_handle_t *sem);
int csp_bin_sem_post_isr(csp_bin_sem_handle_t *sem, CSP_BASE_TYPE * task_woken);

#endif // _CSP_SEMAPHORE_H_
