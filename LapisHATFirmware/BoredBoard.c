#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <hardware/i2c.h>
#include <hardware/irq.h>
#include <hardware/pwm.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define CLOCK_DIV 250
#define PWM_WRAP 5000

#define I2C_SDA_PIN 20
#define I2C_SCL_PIN 21
#define I2C_SLAVE_ADDR 0x69

#define THRUSTER_ACCELERATION 1
#define THRUSTER_TIMEOUT 2500

uint8_t thrusterPins[6] = {4, 5, 6, 7, 8, 9};
uint8_t targetThrust[6] = {127, 127, 127, 127, 127, 127};
uint8_t currentThrust[6] = {127, 127, 127, 127, 127, 127};

uint8_t ledPins[2] = {14, 15};
uint8_t motorPins[4] = {10, 11, 12, 13};
uint8_t servoPins[4] = {16, 17, 18, 19};

// NOTE: the led todos below may be correct, and may be the same mosfets as last year, im writing this on a plane so idk
#define LED_WRAP 255 // TODO
#define LED_CLOCK_DIV 9.77f // TODO

#define MOTOR_WRAP 255 // TODO
#define MOTOR_CLOCK_DIV 9.77f // TODO

uint64_t lastInputTime = 0;

int sgn(int x) {
    if (x > 0) return 1;
    else if (x < 0) return -1;
    else return 0;
}

void i2cSlaveHandler() {
    while (i2c_get_read_available(i2c0)) {
        uint8_t receivedData[2];
        i2c_read_raw_blocking(i2c0, receivedData, 2);
        
        lastInputTime = to_ms_since_boot(get_absolute_time());

        switch (receivedData[0]) {
            case 0: case 1: case 2: case 3: case 4: case 5:
                if (receivedData[1] == 255) targetThrust[receivedData[0]] = 254;
                else targetThrust[receivedData[0]] = receivedData[1];
            case 6: case 7:
                pwm_set_gpio_level(ledPins[receivedData[0]-6], receivedData[1]);
                break;
            case 8: case 9:
                // NOTE: no clue if this works
                switch (sgn(receivedData[1])) {
                    case 1:
                        pwm_set_gpio_level(motorPins[(receivedData[0]-8)*2], receivedData[1]);
                        pwm_set_gpio_level(motorPins[(receivedData[0]-8)*2+1], 0);
                        break;
                    case -1:
                        pwm_set_gpio_level(motorPins[(receivedData[0]-8)*2], 0);
                        pwm_set_gpio_level(motorPins[(receivedData[0]-8)*2+1], -1 * receivedData[i]);
                        break;
                    default:
                        pwm_set_gpio_level(motorPins[(receivedData[0]-8)*2], 0);
                        pwm_set_gpio_level(motorPins[(receivedData[0]-8)*2+1], 0);
                        break
                }
            case 10: case 11: case 12: case 13:
                // NOTE: not sure if i need to set to 254 if its at 255 with servos
                pwm_set_gpio_level(servoPins[receivedData[0]-10], PWM_WRAP / 10 * receivedData[1]);
                break;
            case 255: //test case for pico led
                gpio_put(PICO_DEFAULT_LED_PIN, receivedData[1]);
            default:
                break;
        }
    }
}

void i2cSlave() {
    i2c_init(i2c0, 100000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    // TODO: unsure if its i2c0 or i2c1? i think its i2c0
    i2c_set_slave_mode(i2c0, true, I2C_SLAVE_ADDR);

    irq_set_exclusive_handler(I2C0_IRQ, i2cSlaveHandler);
    irq_set_enabled(I2C0_IRQ, true);

    while (true) tight_loop_contents();
}

int main() {
    // stdio_init_all();

    for (int i = 0; i < 6; i++) {
        gpio_set_function(thrusterPins[i], GPIO_FUNC_PWM);
        uint8_t slice_num = pwm_gpio_to_slice_num(thrusterPins[i]);
        pwm_set_clkdiv(slice_num, CLOCK_DIV);
        pwm_set_wrap(slice_num, PWM_WRAP);
        pwm_set_enabled(slice_num, true);

        // init all thrusters to 1500us
        pwm_set_gpio_level(thrusterPins[i], PWM_WRAP / 10 * 1.5);
    }

    for (int i = 0; i < 2; i++) {
        gpio_set_function(ledPins[i], GPIO_FUNC_PWM);
        uint8_t slice_num = pwm_gpio_to_slice_num(ledPins[i]);
        pwm_set_clkdiv(slice_num, LED_CLOCK_DIV);
        pwm_set_wrap(slice_num, LED_WRAP);
        pwm_set_enabled(slice_num, true);
        pwm_set_gpio_level(ledPins[i], 0);
    }
    
    for (int i = 0; i < 4; i++) {
        gpio_set_function(motorPins[i], GPIO_FUNC_PWM);
        uint8_t slice_num = pwm_gpio_to_slice_num(motorPins[i]);
        pwm_set_clkdiv(slice_num, MOTOR_CLOCK_DIV);
        pwm_set_wrap(slice_num, MOTOR_WRAP);
        pwm_set_enabled(slice_num, true);
        pwm_set_gpio_level(motorPins[i], 0);
    }

    for (int i = 0; i < 4; i++) {
        // NOTE: im pretty sure servos use the same timing as the thrusters?
        gpio_set_function(servoPins[i], GPIO_FUNC_PWM);
        uint8_t slice_num = pwm_gpio_to_slice_num(servoPins[i]);
        pwm_set_clkdiv(slice_num, CLOCK_DIV);
        pwm_set_wrap(slice_num, PWM_WRAP);
        pwm_set_enabled(slice_num, true);
        pwm_set_gpio_level(servoPins[i], PWM_WRAP / 10 * 1.5);
    }

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    multicore_launch_core1(i2cSlave);

    for (;;) {
        if ((to_ms_since_boot(get_absolute_time()) - lastInputTime) > THRUSTER_TIMEOUT) {
            for (int i = 0; i < 6; i++) {
                targetThrust[i] = 127;
            }
            lastInputTime = to_ms_since_boot(get_absolute_time());
        }
        for (int i = 0; i < 6; i++) {
            if (targetThrust[i] != currentThrust[i]) {
                if (abs(targetThrust[i] - currentThrust[i]) > THRUSTER_ACCELERATION) {
                    currentThrust[i] += sgn(targetThrust[i] - currentThrust[i]) * THRUSTER_ACCELERATION;
                    if (currentThrust[i] > 254) currentThrust[i] = 254;
                    else if (currentThrust[i] < 0) currentThrust[i] = 0;
                } else if (abs(targetThrust[i] - currentThrust[i]) > 0) {
                    currentThrust[i] = targetThrust[i];
                }
                float thrust = (currentThrust[i] - -127) * (2.0 - 1.0) / (127 - -127) + 0.5;
                pwm_set_gpio_level(thrusterPins[i], PWM_WRAP / 10 * thrust);
            }
        }
        sleep_ms(5);
    }
}
