/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef __FRAME_CACHE_H__
#define __FRAME_CACHE_H__

#include <pthread.h>

#define DEFALT_PAYLOAD_BUF_SIZE 16588800
#define DEFALT_PAYLOAD_CACHE_CNT 6

typedef struct frame_node_t {
    unsigned char *mem;
    unsigned int length;
    unsigned int used;
    unsigned int index;
    struct frame_node_t *next;
} frame_node_t;

typedef struct frame_cache_t {
    struct frame_node_t *head;
    struct frame_node_t *tail;
    unsigned int count;
} frame_cache_t;

typedef struct frame_queue_t {
    struct frame_cache_t *cache;
    pthread_mutex_t locker;
    pthread_cond_t waiter;
} frame_queue_t;

typedef struct uvc_cache_t {
    frame_queue_t *ok_queue;
    frame_queue_t *free_queue;

    unsigned char debug_print;
} uvc_cache_t;

typedef struct uac_cache_t {
    frame_queue_t *ok_queue;
    frame_queue_t *free_queue;
} uac_cache_t;


int create_uvc_cache(void);
void destroy_uvc_cache(void);
uvc_cache_t *uvc_cache_get(void);

int create_uac_cache(void);
void destroy_uac_cache(void);
void clear_uac_cache(void);
uac_cache_t *uac_cache_get(void);

int put_node_to_queue(frame_queue_t *q, frame_node_t *node);
int get_node_from_queue(frame_queue_t *q, frame_node_t **node);
void clear_uvc_cache(void);

#endif // __FRAME_CACHE_H__
