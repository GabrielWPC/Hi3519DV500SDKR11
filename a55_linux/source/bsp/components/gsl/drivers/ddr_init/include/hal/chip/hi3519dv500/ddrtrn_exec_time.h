/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef DDR_EXEC_TIME_H
#define DDR_EXEC_TIME_H

#define REG_BASE_TIMER01 0xF8002000
#define REG_TIMER_LOAD 0x0
#define REG_TIMER_VALUE 0x4
#define REG_TIMER_CONTROL 0x8
#define TIMER_EN_VAL   (0x1 << 7) /* [7]timeren */
#define TIMER_MODE_VAL (0x1 << 6) /* [6]timermode */
#define TIMER_PRE_VAL  (0x0 << 2) /* [3:2]timerpre */
#define TIMER_SIZE_VAL (0x1 << 1) /* [1]timersize */

#define INITIAL_COUNT_VAL 0xffffffff
#define CLOCK_FREQ   24 /* 24MHz */

#endif /* DDR_EXEC_TIME_H */