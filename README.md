# MainProj

## CMake

### 기본 설명 

- **다양한 컴파일러와 사용 가능**
- **컴파일 설정의 관리**
- **복잡한 프로젝트 구조 지원**

### 사용 방법

- **CMakeLists.txt** : 빌드 프로세스에 대한 지침 추가.


### 예시

```
# 최소 CMake 버전 지정
cmake_minimum_required(VERSION 3.10)

# 프로젝트 이름과 버전 설정
project(MyProject VERSION 1.0)

# C++ 표준 지정
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 실행 파일을 위한 소스 추가
add_executable(MyExecutable main.cpp MyLibrary.cpp)

# 헤더 파일을 포함할 위치 지정
target_include_directories(MyExecutable PUBLIC "${PROJECT_BINARY_DIR}")

# (선택적) 라이브러리 링크
# target_link_libraries(MyExecutable SomeLibrary)

```

##### 설명

- **cmake_minimum_required:** 필요한 최소 CMake 버전을 지정합니다. CMake의 새로운 버전에서 도입된 기능을 사용할 경우 이를 반영하여 설정해야 합니다.

- **project:** 프로젝트의 이름과 버전을 설정합니다. 이것은 또한 언어(기본적으로 C와 C++)를 지정할 수 있는 옵션을 제공합니다.

- **set(CMAKE_CXX_STANDARD ...):** C++ 표준을 지정합니다. 여기서는 C++11을 사용합니다.

- **add_executable:** 빌드할 실행 파일과 그에 필요한 소스 파일을 지정합니다. 여기서는 main.cpp와 MyLibrary.cpp를 사용합니다.

- **target_include_directories:** 프로젝트가 헤더 파일을 찾을 위치를 지정합니다. 여기서는 프로젝트의 이진 디렉토리(PROJECT_BINARY_DIR)를 사용합니다.

- **target_link_libraries (선택적):** 프로젝트가 필요로 하는 라이브러리를 링크합니다. 이 예제에서는 주석 처리되어 있습니다.


## CTest

### 기본 정리

- CMake 테스트 추가

#### 1. CTest 사용 준비

- **CTest** 테스트 도구와 통합
- **enable_testing()** 명령 추가

#### 2. 테스트 실행 파일 추가

- **add_executable** 과 **add_test** 명령을 사용해 정의

```
# 테스트 실행 파일 추가
add_executable(test_mylibrary test_mylibrary.cpp)

# MyLibrary와 연결 (필요한 경우)
target_link_libraries(test_mylibrary MyLibrary)
```

#### 3. 테스트 추가

- **add_test** 로 테스트 추가

```
# 테스트 케이스 추가
add_test(NAME MyLibraryTest COMMAND test_mylibrary)
```

#### 4. 테스트 실행

- **ctest** 명령으로 실행

```
cd build
ctest
```

#### 추가 사용 방법

**테스트 속성 설정:** set_tests_properties 명령을 사용하여 특정 테스트에 대한 추가 속성을 설정가능. **EX) 테스트 작업 시간 제한**

**테스트 그룹화:** -R 플래그를 사용. 정규 표현식에 일치하는 테스트만 실행 가능

**통합 테스트 프레임워크 사용:** Google Test (GTest), Boost.Test, Qt Test와 같은 테스트 프레임워크와 CMake/CTest를 통합하여 사용 가능. 프레임워크가 테스트 관리. 

## Git 

### 기본 설명

- **분산 버전 관리 시스템(DVCS)** : 분산형 모델
- **데이터 무결성** : SHA-1 해시 사용, 파일 디렉토리 변경 확인
- **브랜치 및 병합** : 브랜치(branch)와 병합(merging) 감지
- **비선형 개발** : 여러 브랜치를 운영하고 합치거나 재정렬 가능

### 기본 용어

- **리포지토리 (Repository)**: 프로젝트의 파일과 해당 파일의 변경 이력 저장
- **커밋 (Commit)**: 파일의 변경 사항을 저장소에 안전하게 기록
- **브랜치 (Branch)**: 독립적인 작업 영역
- **병합 (Merge)**: 두 개 이상의 개발 히스토리를 하나로 통합하는 과정
- **풀 (Pull)**: 원격 저장소의 변경 사항을 로컬 저장소로 다운로드
- **푸시 (Push)**: 로컬 저장소의 변경 사항을 원격 저장소에 업로드
- **클론 (Clone)** : 원격 저장소의 복사본을 로컬로 생성

### Git 초기설정

- Git 저장소 생성
```
$ git clone <github url 주소>
# 또는
$ mkdir <Dir 이름>
$ cd <Dir 이름>
$ git init
```

- 파일 추가 및 커밋 진행

```
$ git add <파일 이름>
$ git commit -m "커밋 내용"
```

- git pull & push

```
$ git push origin <branch 이름> # push로 로컬 저장소에서 원격 저장소로
$ git pull origin main # pull로 원격 저장소를 로컬로
```

### Git 상태 관리

- git 상태 확인

```
$ git status
```

- git branch 확인

```
$ git branch
```

- git branch 생성

```
$ git branch <branch 이름>
```

- git branch 변경

```
$ git checkout <branch 이름>
```

- git commit 취소

```
$ git revert <되돌리고 싶은 커밋 이름>
```

- 원격 저장소 추가

```
$ git remote add origin <원격_저장소_url>
```

### .gitignore

- 파일이나 디렉토리에 추적하지 않을 문서나 자료 추가

- 예제

```
# 로그 파일 무시
*.log

# 특정 디렉토리 무시
node_modules/
temp/

# 특정 파일 무시
config.env

# 특정 확장자를 가진 파일 무시
*.class

# 특정 파일 예외 처리 (무시하지 않음)
!lib/important.log

```

## pthread

### 기본 설명

- POSIX 스레드(POSIX Threads) 구현
- UNIX 운영체제에서 멀티스레딩 지원
  

### 주요 기능

- **스레드 생성 및 관리:** 스레드를 생성, 실행 및 종료
- **뮤텍스(Mutexes):** 스레드 간의 상호 배제를 위한 메커니즘 제공
- **조건 변수(Condition Variables):** 스레드 간의 신호 전달
- **스레드 특정 데이터(Thread-specific Data):** 각 스레드에 고유한 데이터를 저장
- **스레드 동기화:** 락(lock)과 배리어(barrier)를 통한 스레드 동기화 지원


### 기본 함수

#### pthread 생성

#### 1. pthread_create()
- 새로운 스레드 생성
```
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);
```
#### 2. pthread_exit()
- 호출 스레드 종료
```
void pthread_exit(void *retval);
```
#### 3. pthread_join()
- 특정 스레드 종료까지 대기
```
int pthread_join(pthread_t thread, void **retval);
```
#### 4. pthread_detach()
- 스레드를 분리.
- 종료 시 자동으로 모든 자원 해제
```
int pthread_detach(pthread_t thread);
```

#### mutex 관리

#### 1. pthread_mutex_init()
- 뮤텍스 초기화
```
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
```
#### 2. pthread_mutex_destroy()
- 뮤텍스 파괴
```
int pthread_mutex_destroy(pthread_mutex_t *mutex);
```
#### 3. pthread_mutex_lock()
- 뮤텍스 잠금 요청
```
int pthread_mutex_lock(pthread_mutex_t *mutex);
```
#### 4. pthread_mutex_unlock()
- 뮤텍스 잠금 해제
```
int pthread_mutex_unlock(pthread_mutex_t *mutex);
```
#### 5. pthread_mutex_trylock()
- 뮤텍스 잠금 시도.
- 잠금 실패 시 오류 반환
```
int pthread_mutex_trylock(pthread_mutex_t *mutex);
```

#### 기타

#### 1. pthread_self()

- 현재 스레드의 식별자 반환
```
 pthread_t pthread_self(void)
```
#### 2. pthtread_equal()

- 두 스레드 식별자가 같은지 비교
```
int pthread_equal(pthread_t t1, pthread_t t2);
```
#### 3. pthread_key_create()
- 스레드별 데이터 키 생성
```
int pthread_key_create(pthread_key_t *key, void (*destructor)(void*));
```

#### 4. pthread_kill()

- 함수 원형
```
int pthread_kill(pthread_t thread, int sig);
```

- POSIX 스레드에 신호를 보내는데 사용
- 스레드를 종료시키는 것이 아닌, 특정 신호에 어떻게 반응해야할지 작성


## PWM

### 기본 설명

- 디지털 신호를 사용해 아날로그 장치 제어하는 방법
- 전압 레벨, 전류를 조절하지 않고, 신호의 폭(펄스의 길이)를 변조해 평균 전력 전달량 제어

### 기본 원리

#### 1. Duty Cycle

- 신호가 1로 되어 있는 시간의 비율

#### 2. Frequency

- 신호의 빈도수. 펄스가 얼마나 빠르게 반복되는지

### 아두이노 샘플 코드

```
const int motorPin = 9; // 모터 드라이버를 연결할 PWM 핀 설정

void setup() {
  pinMode(motorPin, OUTPUT); // 모터 핀을 출력으로 설정
}

void loop() {
  // 모터 속도를 점차 증가
  for (int speed = 0; speed <= 255; speed++) {
    analogWrite(motorPin, speed); // 모터의 속도 설정
    delay(20); // 속도 변경 간의 지연 시간
  }

  // 잠시 정지
  delay(1000);

  // 모터 속도를 점차 감소
  for (int speed = 255; speed >= 0; speed--) {
    analogWrite(motorPin, speed); // 모터의 속도 설정
    delay(20); // 속도 변경 간의 지연 시간
  }

  // 잠시 정지
  delay(1000);
}

```

## SPI(Serial Peripheral Interface)

### 기본 설명
#### 1. Master-Slave 구조

1대 다수의 통신을 지원하는 동기식 통신 방식. 다수의 통신을 위해서는 다수만큼 선이 필요.

#### 2. 4개의 기본 신호선
- **SCLK(Serial Clock)**
- **MOSI(Master Out Slave In)**
- **MISO(Master In Slave Out)**
- **SS(Slave Select)**

#### 3. 전이중 통신
#### 4. 고속 데이터 전송

## UART 통신(Universal Asynchronous Receiver/Transmitter)

### 기본 설명

#### 1. 비동기 통신
#### 2. 전이중 통신
#### 3. Baud rate 
    - 초당 전송되는 bit 수
#### 4. Data format 
    - 사전에 정의 


## UART 통신(Universal Asynchronous Receiver/Transmitter)

### 기본 설명

#### 1. 비동기 통신
#### 2. 전이중 통신
#### 3. Baud rate 
    - 초당 전송되는 bit 수
#### 4. Data format 
    - 사전에 정의 

    ## WiringPi

### 기본 설명

- 다양한 GPIO 관리 가능
- 명령행 유틸리티 
- 확장 가능성

## WiringPi

### 기본 설명

- 다양한 GPIO 관리 가능
- 명령행 유틸리티 
- 확장 가능성

## 시스템 구조도

### 부품 사양서
![image](https://github.com/HAMESS-1st-on-demand/MainProj/assets/44926279/47985aaa-4058-48f1-a96b-1860aff177d6)

### H/W PIN MAP
![image](https://github.com/HAMESS-1st-on-demand/MainProj/assets/44926279/4f2c11d4-eee7-43f8-8a02-7237da7c4543)

![image](https://github.com/HAMESS-1st-on-demand/MainProj/assets/44926279/ca025523-9ba2-4bff-ab3b-2437768a297d)


