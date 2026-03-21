#!/bin/bash

TITLE="BlueStar LapisHAT Tester"
LAPISHAT_I2C_ADDR="0x69"
LAPISHAT_THRUSTER_PWM="0xFF"

set_i2c_val() {
    i2cset -y 1 $LAPISHAT_I2C_ADDR $1 $2
}

run_motor() { # Idk what to call this when this is also used for LEDs. -PC
    set_i2c_val $1 $2
    sleep 2
    set_i2c_val $1 0x00 # Havent actually checked if this turns it off, but i would assume that it does -PC
}

servo_menu() {
    SERVO_NUM=$(whiptail --title "$TITLE" --inputbox "Select a Servo (1-4):" 8 50 3>&1 1>&2 2>&3)
    [ $? -ne 0 ] && return
    SERVO_VAL=$(whiptail --title "$TITLE" --inputbox "What Level (0x00-0xFF):" 8 50 3>&1 1>&2 2>&3)
    [ $? -ne 0 ] && return

    case $SERVO_NUM in

    1)  set_i2c_val 0x0A $SERVO_VAL ;; # Pretty sure these are the right addresses
    2)  set_i2c_val 0x0B $SERVO_VAL ;;
    3)  set_i2c_val 0x0C $SERVO_VAL ;;
    4)  set_i2c_val 0x0D $SERVO_VAL ;;
    *)  return ;;
    esac
}

led_menu() {
    LED_NUM=$(whiptail --title "$TITLE" --inputbox "Select a LED (1-2):" 8 50 3>&1 1>&2 2>&3)
    [ $? -ne 0 ] && return
    LED_VAL=$(whiptail --title "$TITLE" --inputbox "What Level (0x00-0xFF):" 8 50 3>&1 1>&2 2>&3)
    [ $? -ne 0 ] && return

    case $LED_NUM in

    1)  run_motor 0x06 $LED_VAL ;;
    2)  run_motor 0x07 $LED_VAL ;;
    *)  return ;;
    esac
}

motor_menu() {
    MOTOR_NUM=$(whiptail --title "$TITLE" --inputbox "Select a motor (1-2):" 8 50 3>&1 1>&2 2>&3)
    [ $? -ne 0 ] && return
    MOTOR_VAL=$(whiptail --title "$TITLE" --inputbox "What Speed (0x00-0xFF):" 8 50 3>&1 1>&2 2>&3)
    [ $? -ne 0 ] && return

    case $MOTOR_NUM in

    1)  run_motor 0x08 $MOTOR_VAL ;;
    2)  run_motor 0x09 $MOTOR_VAL ;;
    *)  return ;;
    esac
}

thruster_menu() {
    THRUSTER=$(whiptail --title "$TITLE" --inputbox "Enter the thruster you want to run (1-6):" 8 50 3>&1 1>&2 2>&3)
    case $THRUSTER in

    1)  set_i2c_val 0x00 $LAPISHAT_THRUSTER_PWM ;;
    2)  set_i2c_val 0x01 $LAPISHAT_THRUSTER_PWM ;;
    3)  set_i2c_val 0x02 $LAPISHAT_THRUSTER_PWM ;;
    4)  set_i2c_val 0x03 $LAPISHAT_THRUSTER_PWM ;;
    5)  set_i2c_val 0x04 $LAPISHAT_THRUSTER_PWM ;;
    6)  set_i2c_val 0x05 $LAPISHAT_THRUSTER_PWM ;;
    *)  return ;;
    esac
}

main_menu() {
    while true; do
        sub_menu_choice=$(whiptail --title "$TITLE" --menu "Select an option:" 15 50 5 "1" "Thrusters" "2" "Servos" "3" "DC Motors" "4" "External LEDs" "5" "Pico LEDs" 3>&1 1>&2 2>&3)
        case $sub_menu_choice in

        1)  thruster_menu ;;
        2)  servo_menu ;;
        3)  motor_menu ;;
        4)  led_menu ;;
        5) 
            # LED Blink
            for i in {1..5}; do
                set_i2c_val 0xFF 0xFF
                sleep 0.2
                set_i2c_val 0xFF 0x00
                sleep 0.2 
            done
            ;;
        *)  break ;;
        esac
    done
}

main_menu
clear