#ifndef _THREAD1_UTILS_H
#define _THREAD1_UTILS_H

extern volatile int thread1_alive;
int thread1_init(int* log_fd, int thread_id);

#endif