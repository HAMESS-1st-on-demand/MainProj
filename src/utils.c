#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>

char* get_current_time() {
    static char buffer[100];
    struct timeval tv;
    struct tm *timeinfo;

    gettimeofday(&tv, NULL);
    timeinfo = localtime(&tv.tv_sec); // 현재 시간을 초 단위로 얻음

    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo); // 날짜와 시간 형식화
    snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), " %06ld", tv.tv_usec); // 마이크로초 추가

    return buffer;
}

// log message 남기는 함수 
void log_message(const char *format, int thread_id) {
    int fd = open("../log_main.txt", O_WRONLY | O_CREAT | O_APPEND, 0666);
    if(fd < 0){
        return;
    }
    dprintf(fd, "%s %lu : %s", get_current_time(), thread_id, format);
    printf("%s %lu : %s", get_current_time(), thread_id, format);
    close(fd);
    return;
}

// 센서의 값들을 메시지로 변환
char makeMsg(unsigned char sensorInput) { // unsigned char로 변경
    char msg = 0;
    if (sensorInput & (1 << 4)) { // 침수
        msg |= 13 << 3;
        msg |= 1 << 2;
    } 
    else if (sensorInput & (1 << 3)) msg |= 12 << 3; // 비
    else if (sensorInput & (1 << 2)) msg |= 11 << 3; // 미세먼지
    else if (sensorInput & (1 << 1)) msg |= 10 << 3; // 온도 -> 열기
    else if (sensorInput & 1) { // 온도 -> 닫기
        msg |= 9 << 3;
        msg |= 1 << 2;
    }
    return msg;
}

