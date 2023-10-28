#include <stdio.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

#include "SI.h" // 센서를 실제로 받는 코드 작성
#include "utils.h"
#include "thread1_utils.h"

#define BAUD 9600
#define MSGTIME 1000

struct sharedData // 공유 자원을 위한 구조체
{
    unsigned char changed; // 보낼 값이 있는지
    unsigned char ArduinoIdle; 
    unsigned char SendMsg;
    unsigned char RcvMsg;
    char sbuf;
    char rbuf;
    unsigned char status; // close / Open : 0 / 1
};

struct sharedData sd; // 공유 자원 정의

volatile int running = 1; // 프로그램 실행 상태를 제어
// volatile int thread1_alive = 0; // 스레드 1의 상태
volatile int thread2_alive = 0; // 스레드 2의 상태
volatile int stop_thread1 = 0; // kill할 때 보내는 코드
volatile int stop_thread2 = 0; 
int mainThreadId;
extern struct sharedData sd; // 공유 자원 정의
pthread_mutex_t mutex; // mutex 객체

unsigned char ssrStateFlag; //스마트 썬루프 On/Off 상태 변수
unsigned char tintingFlag; //Tinting 상태 변수
unsigned char MainStateFlag=1; // 메인 스레드 상태 check

//인터럽트 딜레이 변수
unsigned int ISR1Time=0;
unsigned int ISR2Time=0;
unsigned int MainTime=0;

// 시그널 핸들러 
// SIGUSR1과 SIGUSR2 시그널을 처리하기 위한 핸들러
// 시그널이 들어올 경우 'stop_thread1' 과 'stop_thread2' 변수 설정
// 스레드 종료
void signal_handler(int signum) {
    if (signum == SIGUSR1) {
        stop_thread1 = 1;
    } else if (signum == SIGUSR2) {
        stop_thread2 = 1;
    }
    return;
}

// ssrButton에 대한 interrupt
void ssrButtonInterrupt(void) {
    if(millis()-ISR1Time>1000){
        if (digitalRead(SSR_STATE_BUTT_PIN) == LOW) {
            ssrStateFlag = !ssrStateFlag;
            printf("ssrStateFlag: %d\n",ssrStateFlag);
            if(ssrStateFlag){
                digitalWrite(SSR_STATE_LED_PIN,HIGH);
                log_message("ssrStateFlag On\n", mainThreadId);
            }
            else {
                digitalWrite(SSR_STATE_LED_PIN,LOW);
                log_message("ssrStateFlag Off\n", mainThreadId);
            }
        }
        ISR1Time = millis();
    }
}

// tintingButton에 대한 interrupt
void tintingButtonInterrupt(void) {
    if(millis()-ISR2Time>1000){
        if (digitalRead(TINTING_BUTT_PIN) == LOW) {
            tintingFlag = !tintingFlag;
            if(tintingFlag){
                digitalWrite(TINTING_LED_PIN,HIGH);
                log_message("tintingFlag On\n", mainThreadId);
            }
            else {
                digitalWrite(TINTING_LED_PIN,LOW);
                log_message("tintingFlag Off\n", mainThreadId);
            }
        }
        ISR2Time = millis();
    }
}

// 스레드1 on/off 에 대한 interrupt
void PIButtonInterrupt(void) {
    if(millis()-MainTime>1000){
        if (digitalRead(PI_ON_OFF_STATE_BUTT_PIN) == LOW) {
            running = 0;
        }
        MainTime = millis();
    }
}

// 다양한 환경 센서를 읽고, 그 값에 따라 시스템 동작 제어
void* func1(void* arg) {
    int log_fd;
    char sendBuf, rcvBuf; // 보내는 값의 buffer : mutex 값 복사 
    // 현재 시간, msg 주기, 센서 주기 값들
    unsigned int now, smsgTime, rmsgTime, lightTime, rainTime, dustTime, waterTime, temperTime;
    unsigned char flags = 0, buffer = 0, local_status = 0, MSB = 0, priority = 0, isOpened=0, isMoving=0, isObeyed = 0; // 센서 판단 저장, sendflag

    // 멈춤 signal이 들어오지 않으면

    if(thread1_init(&log_fd, mainThreadId)==-1){
        return NULL;
    }
    dprintf(log_fd, "=================================================================\n", get_current_time(), pthread_self());
    dprintf(log_fd, "%s %lu start\n", get_current_time(), pthread_self());
    // 초기화 및 동기화
    now = lightTime = rainTime = dustTime = waterTime = temperTime = smsgTime = rmsgTime = millis();
    dprintf(log_fd, "%s %lu Before loop %d\n", get_current_time(), pthread_self(), now);
    while(!stop_thread1) { 
        now = millis();
        // 수신하는 로직 추가
        if(millis() - rmsgTime >= MSGTIME)
        {
            rmsgTime = millis();
            pthread_mutex_lock(&mutex);
            // local_status = sd.status;
            local_status = 1;
            if(sd.RcvMsg == 1)
            {
                rcvBuf = sd.rbuf;
                isOpened = sd.status;
                local_status = sd.status;
                isObeyed = sd.ArduinoIdle;
                sd.RcvMsg = 0;
                MSB = rcvBuf | (1 << 7);
                dprintf(log_fd, "%s %lu SharedMemory Data => Complete: %d, Arduino status : %d, isOpened : %d\n", get_current_time(), pthread_self(), (int)MSB, (int)isObeyed, (int)isOpened);
            }
            pthread_mutex_unlock(&mutex);
            dprintf(log_fd, "%s %lu receive : %d, state : %d \n", get_current_time(), pthread_self(), rmsgTime, local_status);
        }
        // 수위 감지 센서 추가
        if (now - waterTime >= WATERLEV_PER){
            waterTime = now;
            int waterLev = readWaterLevelSensor();
            if ((local_status==0) && (waterLev > WATERLEV_TH)){ // 판단
                buffer |= 1 << 4; // buffer: 10000
            }
            dprintf(log_fd, "%s %lu 수위 센서 : %d, buffer : %d\n", get_current_time(), pthread_self(), waterLev, (int)buffer);
        }

        if(ssrStateFlag){ // SSR 기능이 켜져있을 때만 센서 값 검사
            // 조도 센서 추가
            if(now - lightTime>=LIGHT_PER)
            {
                lightTime = now;
                int light = readLightSensor();
                light_sensor_exec(light, &tintingFlag); // 왜 이거?
                dprintf(log_fd, "%s %lu 조도 센서 : %d, buffer : %d\n", get_current_time(), pthread_self(), light, (int)buffer);
            }
        
            // rain 센서추가 
            if(now - rainTime>=RAIN_PER)
            {
                rainTime =now;
                int rain = readRainSensor();
                printf("[%d] rain = %d\n", now/100,rain);

                if(rain < RAIN_TH && (local_status == 1)) // 비가 오고, status == 1이면
                // 비가 오고, 문이 열려 있으면 닫는다
                {
                    buffer |= 1 << 3; //buffer: 01000
                }
                else{
                    buffer &= ~(1 << 3);
                }
                // if((local_status==1) &&rain<RAIN_TH){ //판단
                //     buffer |= 1<<3; //buffer: 01000
                // }
                // else if((local_status==0) && rain>RAIN_TH+1000){ //썬루프 여는 로직
                //     buffer &= !(1<<3); //buffer: 01000
                // }
                dprintf(log_fd, "%s %lu rain 센서 : %d, buffer : %d\n", get_current_time(), pthread_self(), rain, (int)buffer);
            }

            // 미세먼지 센서 추가
            if(now - dustTime>=DUST_PER)
            {
                dustTime=now;
                int dust = readDustSensor();
                printf("[%d] dust = %d\n", now/100,dust);
                
                // if((local_status==1) && dust>DUST_TH){ //판단
                //     buffer |= 1<<2; //buffer: 00100
                // }else if((local_status==0) && dust<DUST_TH-1000){ //썬루프 여는 로직
                //     buffer &= !(1<<2);
                // }

                if(dust > DUST_TH && (local_status==1)) // 문 열려있을 때 미세먼지면 닫는다.
                {
                    buffer |= (1 << 2); // buffer : 00100
                }else{
                    buffer &= ~(1 << 2);
                }

                dprintf(log_fd, "%s %lu 미세먼지 센서 : %d, buffer : %d\n", get_current_time(), pthread_self(), dust, (int)buffer);
            }
        
            // 온습도 센서 추가
            if(now - temperTime>=TEMPER_PER)
            {
                temperTime = now;
                int temper1 = readDHTSensor(DHT_PIN1);        // 10번 핀으로부터 데이터를 읽음 -> 실내온도
                int temper2 = readDHTSensor(DHT_PIN2);        // 11번 핀으로부터 데이터를 읽음 -> 실외 온도

                if(temper1 ==-1||temper2 ==-1){
                    printf("DHT data not good\n");
                    continue;
                }
                printf("[%d] temper1 = %d, temper2 = %d \n", now/10000,temper1,temper2);

                if((local_status==1) && temper1<temper2&&temper2>TEMPER_TH1){ //판단
                    buffer |= 1<<1; //buffer: 00010
                }
                else if((local_status==0) && temper1>temper2&&temper1>TEMPER_TH2){
                    buffer |= 1; 
                }
                dprintf(log_fd, "%s %lu 온습도 센서 : <%d, %d> %d\n", get_current_time(), pthread_self(), temper1, temper2, (int)buffer);
            }
        }

        // 송신 : 1초마다
        if(millis() - smsgTime >= MSGTIME)
        {
            dprintf(log_fd, "%s %lu send : %d\n", get_current_time(), pthread_self(), smsgTime);
            smsgTime = millis();
            sendBuf = makeMsg(buffer); // buffer를 sendBuf로 변환
            if(sendBuf != 0)
            {
                pthread_mutex_lock(&mutex);

                if(sd.changed == 1)++(sd.changed);
                else if(sd.changed == 0 || sd.changed == 2)
                {
                    sd.changed = 1;
                    sd.sbuf = sendBuf;
                    sd.SendMsg = 1;
                    dprintf(log_fd, "%s %lu send msg flag up : %d\n", get_current_time(), pthread_self(), (int)sd.sbuf);
                }
                sendBuf = 0;
                pthread_mutex_unlock(&mutex);
            }
        }
        if(stop_thread1)
        {
            dprintf(log_fd, "%s %lu Take End signal\n", get_current_time(), pthread_self());
            close(log_fd);
            thread1_alive = 0;
            log_message("Thread 1 is terminated.\n", mainThreadId);
            break;
        }
        // dprintf(log_fd, "%s %lu delta Time Between loop-start and loop-end: %d\n", get_current_time(), pthread_self(), millis()-now);
        if(millis() - now < 100)delay(100 - millis() + now);
    }
    close(log_fd);
    thread1_alive = 0;
    log_message("Thread 1 is terminated.\n", mainThreadId);
    return NULL;
}

void* func2(void* arg) {
    int log_fd = open("../log_thread2.txt", O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (log_fd == -1) {
        log_message("Failed to open file\n", mainThreadId);
        // thread2가 죽었다는 flag 작성
        thread2_alive = 0;
        log_message("Thread2 is terminated.\n", mainThreadId);
        return NULL;
    }
    dprintf(log_fd, "=================================================================\n", get_current_time(), pthread_self());
    dprintf(log_fd, "%s %lu start\n", get_current_time(), pthread_self());
    
    // ttyS0 이 아닌 ttyUSB0 <= dmesg | tail로 확인
    int uart_fd = serialOpen("/dev/ttyACM0", BAUD); // ARDUINO R3 USB => ttyACM0, UNO => ttyUSB0
    if(uart_fd == -1)
    {
        log_message("Device file open error!!\n", mainThreadId);
        // thread2 정리
        close(log_fd);
        thread2_alive = 0;
        log_message("Thread2 is terminated.\n", mainThreadId);
        return NULL;
    }
    unsigned char local_status = 0, MSB = 0, priority = 0, isOpened=0, isMoving=0, isObeyed = 0;
    int times = millis();
    while(!stop_thread2) {
        // 계속해서 값을 받는다. 
        // mutex 잠그고 값 확인
        unsigned char sendflag;
        char buf; 
        // 살아있다는 값 계속 전달 
        if(millis() - times >= 1000)
        {
            dprintf(log_fd, "%s %lu alive : %d\n", get_current_time(), pthread_self(), times);
            times = millis();
        }
        pthread_mutex_lock(&mutex);
        if(sd.changed > 0 && sd.SendMsg > 0)
        {
            // strcpy(buf, sd.sbuf);
            sendflag = 1;
            buf = sd.sbuf;
            dprintf(log_fd, "%s %lu send msg copy : %d\n", get_current_time(), pthread_self(), (int)buf);
            sd.changed = 0;
            sd.SendMsg = 0;
            sd.sbuf = '\0';
        }
        pthread_mutex_unlock(&mutex);

        if(sendflag == 1)
        {
            serialPutchar(uart_fd, buf);
            dprintf(log_fd, "%s %lu Writing to Arduino: %d\n", get_current_time(), pthread_self(), (int)buf);
            sendflag = 0;
            buf = '\0';
        }

        int i = 0;
        if(serialDataAvail(uart_fd))
        {
            buf = serialGetchar(uart_fd);
            printf("===> Received : %d\n", (int)buf);
            serialFlush(uart_fd);
            dprintf(log_fd, "%s %lu Received Msg trom Arduino : %d\n", get_current_time(), pthread_self(), (int)buf);
            pthread_mutex_lock(&mutex);
            sd.RcvMsg = 1;
            sd.rbuf = buf;
            sd.status = (buf & (1 << 2)) >> 2; // open인지 close인지
            sd.ArduinoIdle = buf & 1; // obey
            pthread_mutex_unlock(&mutex);
            MSB = (buf & (1 << 7)) >> 7;
        }

        if (stop_thread2) {
            dprintf(log_fd, "%s %lu Take End signal\n", get_current_time(), pthread_self());
            serialClose(uart_fd);
            close(log_fd);
            thread2_alive = 0;
            log_message("Thread2 is terminated.\n", mainThreadId);
            break;
        }
    }
    serialClose(uart_fd);
    close(log_fd);
    thread2_alive = 0;
    log_message("Thread2 is terminated.\n", mainThreadId);
    return NULL;
}

int main()
{
    mainThreadId = pthread_self();
    log_message("=======================================================================\n", mainThreadId);
    log_message("Start\n", mainThreadId);
    pthread_t thread1, thread2;

    // setWiringPi로 gpio on
    if(setWiringPi()==-1) {
        log_message("Failed to setup WiringPi\n", mainThreadId);
        return -1;
    }
    else{
        log_message("Success to setup WiringPi\n", mainThreadId);
    }

    if (wiringPiISR(SSR_STATE_BUTT_PIN, INT_EDGE_FALLING, ssrButtonInterrupt) < 0) {
        log_message("인터럽트 핸들러 1 설정 실패\n", mainThreadId);
        return 1;
    }
    else
    {
        log_message("인터럽트 핸들러 1 설정\n", mainThreadId);
    }
    if (wiringPiISR(TINTING_BUTT_PIN, INT_EDGE_FALLING, tintingButtonInterrupt) < 0) {
        log_message("인터럽트 핸들러 2 설정 실패\n", mainThreadId);
        return 1;
    }
    else{
        log_message("인터럽트 핸들러 2 설정\n", mainThreadId);
    }
    if (wiringPiISR(PI_ON_OFF_STATE_BUTT_PIN, INT_EDGE_FALLING, PIButtonInterrupt) < 0) {
        log_message("인터럽트 핸들러 3 설정 실패\n", mainThreadId);
        return 1;
    }
    else{
        log_message("인터럽트 핸들러 3 설정\n", mainThreadId);
    }

    pthread_mutex_init(&mutex, NULL); // 뮤텍스 초기화
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    if (pthread_create(&thread1, NULL, &func1, NULL)) {
        log_message("Error creating thread 1\n", mainThreadId);
        return 1;
    }
    else{
        thread1_alive = 1;
    }
    if (pthread_create(&thread2, NULL, &func2, NULL)) {
        log_message("Error creating thread 2\n", mainThreadId);
        return 1;
    }else{
        thread2_alive = 1;
    }

    while(running)
    {
        if(!thread1_alive)
        {
            log_message("Thread1 is down. Restarting...\n", mainThreadId);
            pthread_join(thread1, NULL);
            pthread_create(&thread1, NULL, &func1, NULL);
            thread1_alive = 1;
        }else{
            log_message("Thread1 running\n", mainThreadId);
        }
        if(!thread2_alive)
        {
            log_message("Thread2 is down. Restarting...\n", mainThreadId);
            pthread_join(thread2, NULL);
            pthread_create(&thread2, NULL, &func2, NULL);
            thread2_alive = 1;
        }else{
            log_message("Thread2 running\n", mainThreadId);
        }
        sleep(1);
    }

    // LED OFF 진행 
    digitalWrite(TINTING_LED_PIN,LOW);
    digitalWrite(SSR_STATE_LED_PIN,LOW);

    pthread_kill(thread1, SIGUSR1);
    pthread_kill(thread2, SIGUSR2);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    log_message("Thread1 Terminated!\n", mainThreadId);
    log_message("Thread2 Terminated!\n", mainThreadId);
    pthread_mutex_destroy(&mutex); 

    log_message("Destroy mutex!\n", mainThreadId);
    log_message("End\n", mainThreadId);

    return 0;
}