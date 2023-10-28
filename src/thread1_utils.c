#include <stdio.h>
#include <fcntl.h>
#include <wiringPi.h>

#include "thread1_utils.h"
#include "SI.h"

volatile int thread1_alive = 0;

// thread1_init 하는 함수
int thread1_init(int* log_fd, int thread_id){
    *log_fd = open("../log_thread1.txt", O_WRONLY | O_CREAT | O_APPEND, 0666); // log file을 위한 descriptor
    if (*log_fd == -1){ // log file descriptor 못 쓰면 반환한다.
        log_message("Failed to open file.\n", thread_id);
        // thread1_alive = 0로 thread 멈췄다고 알려준다.
        thread1_alive = 0;
        log_message("Thread 1 is terminated.\n", thread_id);
        return -1;
    }
    return 0;
}

extern void light_sensor_exec(int light, unsigned char *tintingFlag){
    if ((!(*tintingFlag)) && light < LIGHT_TH){ // 판단
        // printf("썬루프 어둡게\n"); // buffer가 아닌 LED 진행
        *tintingFlag = 1; // 
        digitalWrite(TINTING_LED_PIN, HIGH);
    }
    else if ((*tintingFlag) && light > LIGHT_TH + 500){ // 썬루프 투명하게하는 로직
        *tintingFlag = 0;
        digitalWrite(TINTING_LED_PIN, LOW);
    }
}