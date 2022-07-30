#ifndef SETTINGS_H
#define SETTINGS_H
#define MOTOR_FL 1
#define MOTOR_FR 2
#define MOTOR_BL 3
#define MOTOR_BR 4

#define PWM_FULL 19999

#define MOTOR_FL_PIN 0
#define MOTOR_FR_PIN 1
#define MOTOR_BL_PIN 2
#define MOTOR_BR_PIN 3

#define LED_RUN_PIN 15
#define LED_ERROR_PIN 14
#define LED_AUTO_PIN 11
#define LED_WHITE_PIN 13
#define LED_BLUE_PIN 12

#define ENABLE_FL 1
#define ENABLE_FR 1
#define ENABLE_BL 1
#define ENABLE_BR 1

#define FLBR_DIAGONAL 1
#define FRBL_DIAGONAL 2

#define THROTTLE_MIN 1000 // throttle minimum pwm 1 ms
#define THROTTLE_MAX 2000 // throttle maximum pwm 2 ms

#define DEFAULT_CONFIG_FILE "./config/config.json"
#define DRONE_STATE_CONFIG_FILE "./run/drone.json"


// UNCOMMENT THIS TO ENABLE MOTOR

// #define ENABLE_MOTOR

#endif

