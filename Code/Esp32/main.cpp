#include "driver/mcpwm.h"
#include <HardwareSerial.h>

HardwareSerial camSerial(2);

int PWMB = 25;
int BIN1 = 27;
int BIN2 = 26;

int PWMC = 13;
int CIN1 = 14;
int CIN2 = 12;

int PWMA = 4;
int AIN1 = 15;
int AIN2 = 2;

const float motor_A[2] = {-0.5, 0.866025};
const float motor_B[2] = {-0.5, -0.866025};
const float motor_C[2] = {1, 0};

void setup() {
    Serial.begin(115200);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, PWMA);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, PWMB);
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM0A, PWMC);

    mcpwm_config_t pwm_config = {
        .frequency = 5000,
        .cmpr_a = 0,
        .cmpr_b = 0,
        .duty_mode = MCPWM_DUTY_MODE_0,
        .counter_mode = MCPWM_UP_COUNTER
    };

    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
    mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_0, &pwm_config);

    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);
    pinMode(CIN1, OUTPUT);
    pinMode(CIN2, OUTPUT);

    camSerial.begin(115200, SERIAL_8N1, 16, 17);
}

void setMotorA(float speed) {
    if (speed > 0) {
        digitalWrite(AIN1, HIGH);
        digitalWrite(AIN2, LOW);
    } else {
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, HIGH);
        speed = -speed;
    }
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, speed);
}

void setMotorB(float speed) {
    if (speed > 0) {
        digitalWrite(BIN1, HIGH);
        digitalWrite(BIN2, LOW);
    } else {
        digitalWrite(BIN1, LOW);
        digitalWrite(BIN2, HIGH);
        speed = -speed;
    }
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, speed);
}

void setMotorC(float speed) {
    if (speed > 0) {
        digitalWrite(CIN1, HIGH);
        digitalWrite(CIN2, LOW);
    } else {
        digitalWrite(CIN1, LOW);
        digitalWrite(CIN2, HIGH);
        speed = -speed;
    }
    mcpwm_set_duty(MCPWM_UNIT_1, MCPWM_TIMER_0, MCPWM_OPR_A, speed);
}

void moveBy(float x, float y, float r = 0) {
    x *= 100;
    y *= -100;
    float powerA = x * motor_A[0] + y * motor_A[1] + r;
    float powerB = x * motor_B[0] + y * motor_B[1] + r;
    float powerC = x * motor_C[0] + y * motor_C[1] + r;
    setMotorA(powerA);
    setMotorB(powerB);
    setMotorC(powerC);
}

void rotateBy(float r) {
    setMotorA(r);
    setMotorB(r);
    setMotorC(r);
}

void loop() {
    if (camSerial.available() || Serial.available() > 0) {
        String cmd;

        if (Serial.available() > 0) {
            cmd = Serial.readStringUntil('\n');
        } else {
            cmd = camSerial.readStringUntil('\n');
        }

        char cmdType = cmd.charAt(0);
        float speed, r, x, y;
        int firstComma, secondComma;

        switch (cmdType) {
            case 'A':
            case 'B':
            case 'C':
                speed = cmd.substring(2).toFloat();
                if (cmdType == 'A') {
                    setMotorA(speed);
                    Serial.printf("set A to %.1f%%\n", speed);
                } else if (cmdType == 'B') {
                    setMotorB(speed);
                    Serial.printf("set B to %.1f%%\n", speed);
                } else {
                    setMotorC(speed);
                    Serial.printf("set C to %.1f%%\n", speed);
                }
                break;

            case 'V':
                firstComma = cmd.indexOf(',');
                secondComma = cmd.indexOf(',', firstComma + 1);
                if (secondComma > 0) {
                    x = cmd.substring(firstComma + 1, secondComma).toFloat();
                    int thirdComma = cmd.indexOf(',', secondComma + 1);
                    if (thirdComma > 0) {
                        y = cmd.substring(secondComma + 1, thirdComma).toFloat();
                        r = cmd.substring(thirdComma + 1).toFloat();
                        moveBy(x, y, r);
                        Serial.printf("moving: x=%.2f, y=%.2f, r=%.2f\n", x, y, r);
                    } else {
                        y = cmd.substring(secondComma + 1).toFloat();
                        moveBy(x, y);
                        Serial.printf("moving: x=%.2f, y=%.2f\n", x, y);
                    }
                }
                break;

            case 'R':
                r = cmd.substring(2).toFloat();
                rotateBy(r);
                Serial.printf("rotating: r=%.2f\n", r);
                break;

            case 'S':
                moveBy(0, 0);
                break;

            default:
                Serial.println("Unknown command");
                break;
        }
    }
}
