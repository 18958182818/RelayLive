/*!
 * \file ring_buff.h
 *
 * \author wlla
 * \date ʮһ�� 2018
 *
 * �����Ļ��λ�������ͬһʱ��һ���̲߳��룬һ���̶߳�ȡ
 */


#ifndef UTIL_RING_BUFF
#define UTIL_RING_BUFF

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct ring_buff ring_buff_t;

/**
 * ����ringbuff
 * @param element_len ����Ԫ�صĴ�С
 * @param count Ԫ�ظ���
 * @param destroy_element Ԫ��ɾ���ķ���
 */
ring_buff_t* create_ring_buff(size_t element_len, size_t count, void(*destroy_element)(void *));

/**
 * ����ringbuff
 */
void destroy_ring_buff(struct ring_buff *ring);

/** ��ȡringbuff�п���λ�õĸ��� */
size_t ring_get_count_free_elements(struct ring_buff *ring);

/** ��ȡringbuff���Ѿ�������ݵ�λ�ø��� */
size_t ring_get_count_waiting_elements(struct ring_buff *ring, uint32_t *tail);

/** ��src����max_count��Ԫ�ؾ����ܵĲ��뵽ring�У�֪��ȫ�������ring����������ʵ�ʲ���ĸ��� */
size_t ring_insert(struct ring_buff *ring, const void *src, size_t max_count);

/**
 * ��ring����tailָ����λ�þ����ܵ��Ƴ�max_count��Ԫ�ؿ�����dest��
 * destΪ��ʱ�����п���������ֱ����ring���Ƴ�
 * tail����Ƴ����λ�ã����tailΪ�գ����ring��¼��oldest_tail����ʼ
 */
size_t ring_consume(struct ring_buff *ring, uint32_t *tail, void *dest, size_t max_count);

/** ��ring����tailλ��ָ����λ���ҵ���һ�����ڵ�Ԫ��
 * ���ú�ʹ��lws_ring_consume(ring, &tail, NULL, 1)����ring�����߸�Ԫ��
 */
const void* ring_get_element(struct ring_buff *ring, uint32_t *tail);

/**
 * �ͷŲ�����Ҫ��Ԫ�ء�
 * ���ж���ط�ʹ�����ringbuffʱ��ÿ�λ�ȡԪ�غ󲢲������ͷţ�����ͨ���������ͳһ�ͷŲ�����Ҫ��Ԫ��
 * @param tail �ͷŵ�Ԫ��λ��
 */
void ring_update_oldest_tail(struct ring_buff *ring, uint32_t tail);

/** ��ȡringbuff�е�һ��Ԫ�ص�λ�� */
uint32_t ring_get_oldest_tail(struct ring_buff *ring);

int ring_next_linear_insert_range(struct ring_buff *ring, void **start, size_t *bytes);

void ring_bump_head(struct ring_buff *ring, size_t bytes);

void ring_dump(struct ring_buff *ring, uint32_t *tail);

#define ring_consume_and_update_oldest_tail(\
		___ring,    /* the ring_buff object */ \
		___type,    /* type of objects with tails */ \
		___ptail,   /* ptr to tail of obj with tail doing consuming */ \
		___count,   /* count of payload objects being consumed */ \
		___list_head,	/* head of list of objects with tails */ \
		___mtail,   /* member name of tail in ___type */ \
		___mlist  /* member name of next list member ptr in ___type */ \
	) { \
		int ___n, ___m; \
	\
	___n = ring_get_oldest_tail(___ring) == *(___ptail); \
	ring_consume(___ring, ___ptail, NULL, ___count); \
	if (___n) { \
		uint32_t ___oldest; \
		___n = 0; \
		___oldest = *(___ptail); \
		lws_start_foreach_llp(___type **, ___ppss, ___list_head) { \
			___m = ring_get_count_waiting_elements( \
					___ring, &(*___ppss)->tail); \
			if (___m >= ___n) { \
				___n = ___m; \
				___oldest = (*___ppss)->tail; \
			} \
		} lws_end_foreach_llp(___ppss, ___mlist); \
	\
		ring_update_oldest_tail(___ring, ___oldest); \
	} \
}

/*
 * This does the same as the ring_consume_and_update_oldest_tail()
 * helper, but for the simpler case there is only one consumer, so one
 * tail, and that tail is always the oldest tail.
 */

#define ring_consume_single_tail(\
		___ring,  /* the ring_buff object */ \
		___ptail, /* ptr to tail of obj with tail doing consuming */ \
		___count  /* count of payload objects being consumed */ \
	) { \
	ring_consume(___ring, ___ptail, NULL, ___count); \
	ring_update_oldest_tail(___ring, *(___ptail)); \
}

#ifdef __cplusplus
}
#endif
#endif