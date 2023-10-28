#ifndef _UTILS_H_
#define _UTILS_H_

// 현재 시간을 받아오는 함수
char* get_current_time();

// log message 남기는 함수 
void log_message(const char *format, int thread_id);

// 센서의 값들을 메시지로 변환
char makeMsg(unsigned char sensorInput);

#endif