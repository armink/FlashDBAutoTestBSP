/*
 * Copyright (c) 2020, Armink, <armink.ztl@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief configuration file
 */

#ifndef _FDB_CFG_H_
#define _FDB_CFG_H_

/* more configuration macro is in rtconfig.h */
#include <rtconfig.h>

extern void rt_kprintf(const char *fmt, ...);
#define FDB_PRINT(...)             rt_kprintf(__VA_ARGS__)

#endif /* _FDB_CFG_H_ */
