#ifndef DEF_H_
#define DEF_H_

#define COMMAND_NUMBER 10
#define COMMAND_LENGTH 80

#define IDLE 0
#define PC_COMMAND 1
#define BT_COMMAND 2
#define BUTTON_COMMAND 3

#define MANUAL_MODE 0 // BT_COMMAND RUN
#define DISTANCE_CHECK 1 // 초음파 거리 측정
#define AUTO_MODE_CHECK 2 // BUTTON0 체크
#define AUTO_MODE 3 // 자율 주행

#define FORWARD 0
#define BACKWARD 1
#define TURN_LEFT 2
#define TURN_RIGHT 3

#endif /* DEFINE_H_ */