/*
 * snep_service_thread.h
 *
 *  Created on: 21-okt-2012
 *      Author: sam
 */

#ifndef SNEP_SERVICE_THREAD_H_
#define SNEP_SERVICE_THREAD_H_

void *snep_service_thread (void *arg);
int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1);
void timeval_print(struct timeval *tv);

#endif /* SNEP_SERVICE_THREAD_H_ */
