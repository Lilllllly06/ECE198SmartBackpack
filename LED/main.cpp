#include "main.h"

// Define constants for vibration intensity levels
#define MIN_VIBRATION 20   // 0.2g
#define MAX_VIBRATION 60   // 0.6g

// Global variables
uint8_t vibration_intensity = 40;  // Default vibration intensity (e.g., 0.4g)

// Declare your PWM and ADC handle variables
extern TIM_HandleTypeDef htim2;  // Timer for PWM 
extern ADC_HandleTypeDef hadc1;  // ADC for reading pressure sensor

// Function prototypes
void set_vibration_intensity(uint8_t intensity);
void update_led_brightness(uint8_t intensity);
uint16_t read_pressure_sensor(void);
void handle_serial_input(void);

// Main function
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_TIM2_Init();  // Init PWM for vibration motor
    MX_USART1_UART_Init();  // UART initialization 
    
    // Start PWM output for vibration motor
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    
    // Main loop
    while (1)
    {
        // Read the current pressure sensor value 
        uint16_t pressure_value = read_pressure_sensor();
        
        // Adjust vibration intensity based on internal logic or user input 
        handle_serial_input();
        
        // Update the brightness of the single LED based on vibration intensity
        update_led_brightness(vibration_intensity);
        
        HAL_Delay(100);  // Short delay which allows for stable reading
    }
}

// Function to set vibration intensity by adjusting PWM duty cycle (also controls LED brightness)
void set_vibration_intensity(uint8_t intensity)
{
    // Map intensity to PWM duty cycle (assuming 0-100 range for PWM duty)
    uint32_t pwm_duty = (intensity * (htim2.Init.Period + 1)) / 100; 
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pwm_duty);  // Set PWM duty cycle for vibration motor
}

// Function to update the brightness of a single LED based on the vibration intensity
void update_led_brightness(uint8_t intensity)
{
    // Map vibration intensity to LED brightness (PWM duty cycle)
    uint32_t led_pwm_duty = (intensity * (htim2.Init.Period + 1)) / 100;
    
    // Use PWM to adjust LED brightness
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, led_pwm_duty);  //  Another PWM channel is used for LED brightness
}

// Function to read pressure sensor value (ADC reading)
uint16_t read_pressure_sensor(void)
{
    HAL_ADC_Start(&hadc1);  // Start ADC conversion
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
        return HAL_ADC_GetValue(&hadc1);  // Return the ADC value from the sensor
    }
    return 0;  // Return 0 if ADC fails
}

// Function to handle serial input (UART) for controlling vibration intensity
void handle_serial_input(void)
{
    uint8_t received_data[20];  // Buffer to store received data
    if (HAL_UART_Receive(&huart1, received_data, sizeof(received_data), 100) == HAL_OK)
    {
        if (strncmp((char*)received_data, "vibration_up", 12) == 0) {
            if (vibration_intensity < MAX_VIBRATION) {
                vibration_intensity += 10;  // Increase vibration intensity
                set_vibration_intensity(vibration_intensity);  // Update vibration motor PWM
            }
        }
        else if (strncmp((char*)received_data, "vibration_down", 14) == 0) {
            if (vibration_intensity > MIN_VIBRATION) {
                vibration_intensity -= 10;  // Decrease vibration intensity
                set_vibration_intensity(vibration_intensity);  // Update vibration motor PWM
            }
        }
    }
}
