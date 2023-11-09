#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "lcd_display.hpp"
#include "menu_info.hpp"
#include <string>
#include <map>


#define k_factor 0.0025
#define PWM_PIN 0

#define BTN_PIN 10
#define LEFT_PIN 11
#define RIGHT_PIN 12

#define DB4_PIN 2
#define DB5_PIN 3
#define DB6_PIN 4
#define DB7_PIN 5
#define RS_PIN 6
#define E_PIN 7
#define LCD_COLS 16
#define LCD_ROWS 2

#define COMPRESSOR 0
#define AIR_BRUSH 1
#define CURRENT_MODE 0
#define MODE_SELECT 1
#define PRESSURE_SELECT 2

int start = 0;
int currentMode = COMPRESSOR;
int currentMenu = 0;
uint8_t scroll = 0;
bool update = false;

LCDdisplay myLCD(DB4_PIN, DB5_PIN, DB6_PIN, DB7_PIN, RS_PIN, E_PIN, LCD_COLS, LCD_ROWS);

int millis() {
    return to_ms_since_boot(get_absolute_time());
}

bool debounce(){
    if (millis()-start>500){
        start = millis();
        return true;
    }
    return false;
}

void set_current_mode_menu(){
    myLCD.clear();
    myLCD.cursor_off();
    myLCD.print("Mode:");
    myLCD.goto_pos(0,1);
    myLCD.print(mode.at(currentMode));
}

void set_mode_select_menu(){
    myLCD.clear();
    myLCD.cursor_off();
    myLCD.print("Select Mode:");
    myLCD.goto_pos(0,1);
    myLCD.print(mode.at(scroll%2));
    currentMode = scroll%2;
}

void set_pressure_select_menu(){
    myLCD.clear();
    myLCD.cursor_off();
    myLCD.print("Select Pressure:");
    myLCD.goto_pos(0,1);

    const char* value = std::to_string(scroll).c_str();

    myLCD.print(value);
    currentMode = scroll%2;
}

void update_screen(){
    switch (currentMenu)
    {
    case CURRENT_MODE:
        set_current_mode_menu();
        break;
    case MODE_SELECT:
        set_mode_select_menu();
        break;
    case PRESSURE_SELECT:
        set_pressure_select_menu();
        break;
    default:
        break;
    }
    update = false;
}



void interrupt_handler(uint gpio, uint32_t events) {
    if (debounce()){
    switch (gpio)
    {
    case BTN_PIN:
        currentMenu = (currentMenu+1)%3;
        if (currentMode==AIR_BRUSH && currentMenu==PRESSURE_SELECT)
            currentMenu = CURRENT_MODE;
        break;
    case LEFT_PIN:
    printf("left");
        if (currentMenu==CURRENT_MODE)
            return;
        scroll--;
        break;
    case RIGHT_PIN:
    printf("right");
        if (currentMenu==CURRENT_MODE)
            return;
        scroll++;
        break;
    default:
        break;
    }
    update = true;
    }
}

int main() {
    stdio_init_all();
    myLCD.init();
    gpio_init(BTN_PIN);
    gpio_set_dir(BTN_PIN, GPIO_IN);
    //gpio_pull_up(BTN_PIN);
    gpio_set_irq_enabled_with_callback(BTN_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_handler);
    gpio_set_irq_enabled_with_callback(LEFT_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_handler);
    gpio_set_irq_enabled_with_callback(RIGHT_PIN, GPIO_IRQ_EDGE_RISE, true, &interrupt_handler);
    /*uint16_t dutyCycle;

    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);
    */
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(0);
    pwm_set_enabled(slice_num, true);
    pwm_set_wrap(slice_num, 1000);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 500);

    set_current_mode_menu();

    while (1) {
        /*// 12-bit conversion, assume max value == ADC_VREF == 3.3 V
        const float conversion_factor = 3.3f / (1 << 12);
        uint16_t result = adc_read();
        float voltage = result;
        float pressure = (voltage-0.2)/k_factor;
        printf("Pressure: %f", pressure);
        printf("voltage: %f V\n", voltage);
        dutyCycle = (result * 1000) / 4095;
        pwm_set_chan_level(slice_num, PWM_CHAN_A, dutyCycle);
        //sleep_ms(500);*/
        if (update)
            update_screen();
    }
    return 0;
}
