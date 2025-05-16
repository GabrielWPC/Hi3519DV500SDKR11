/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

#ifndef DDR_LOG_H
#define DDR_LOG_H

#include "ddrtrn_training_internal_config.h"

/* ******log level ******************* */
#define DDR_LOG_INFO_STR    "info"
#define DDR_LOG_DEBUG_STR   "debug"
#define DDR_LOG_WARNING_STR "warning"
#define DDR_LOG_ERROR_STR   "error"
#define DDR_LOG_FATAL_STR   "fatal"

#define DDR_LOG_INFO    (1 << 0)
#define DDR_LOG_DEBUG   (1 << 1)
#define DDR_LOG_WARNING (1 << 2)
#define DDR_LOG_ERROR   (1 << 3)
#define DDR_LOG_FATAL   (1 << 4)

/* ******log define ********************************************** */
#if defined(DDR_TRAINING_CMD) && defined(DDR_TRAINING_LOG_CONFIG)
#define ddrtrn_info(fmt...) ddrtrn_training_log(__func__, DDR_LOG_INFO, fmt)
#define ddrtrn_debug(fmt...) ddrtrn_training_log(__func__, DDR_LOG_DEBUG, fmt)
#define ddrtrn_warning(fmt...) ddrtrn_training_log(__func__, DDR_LOG_WARNING, fmt)
#define ddrtrn_error(fmt...) ddrtrn_training_log(__func__, DDR_LOG_ERROR, fmt)
#define ddrtrn_fatal(fmt...) ddrtrn_training_log(__func__, DDR_LOG_FATAL, fmt)
#else
#define ddrtrn_info(fmt...)
#define ddrtrn_debug(fmt...)
#define ddrtrn_warning(fmt...)
#define ddrtrn_error(fmt...)
#define ddrtrn_fatal(fmt...)
#endif /* DDR_TRAINING_CMD && DDR_TRAINING_LOG_CONFIG */
/* ******trace function********************************************** */
void ddrtrn_trace(const char *func_name, const unsigned int *val);
#endif /* DDR_LOG_H */
