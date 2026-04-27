#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>

#define TRIG_PIN  GPIO_PIN_9
#define ECHO_PIN  GPIO_PIN_10
#define TRIG_PORT GPIOB
#define ECHO_PORT GPIOB
#define LED_PORT  GPIOD
#define LED1      GPIO_PIN_12 // green
#define LED2      GPIO_PIN_13 // orange
#define LED3      GPIO_PIN_14 // red
#define LED4      GPIO_PIN_15 // blue

UART_HandleTypeDef huart2;
TIM_HandleTypeDef htim3;

char msg[50];

void uart_init(void);
void timer_init(void);
void delay_us(uint16_t us);
int get_distance(void);
void led_init(void);
void blink_led(int distance);

int distance = 0;

int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, 10);
	return ch;
}

int main(void)
{
    HAL_Init();
    uart_init();
    timer_init();
    led_init();

    while(1)
    {
        distance = get_distance();

        printf("Distance = %d\n\r",distance);

        blink_led(distance);
        HAL_Delay(300);

    }
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}


void timer_init(void)
{
    __HAL_RCC_TIM3_CLK_ENABLE();

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 16 - 1;   // 16 MHz / 16 = 1 MHz
    htim3.Init.Period = 0xFFFF;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

    HAL_TIM_Base_Init(&htim3);
    HAL_TIM_Base_Start(&htim3);
}

void delay_us(uint16_t us)
{
	//printf("3\n\r");
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    while(__HAL_TIM_GET_COUNTER(&htim3) < us);
}

int get_distance(void)
{
    uint32_t duration = 0;
    uint32_t timeout;

    //  Send TRIG pulse
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
    delay_us(2);

    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
    delay_us(10);

    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);

    //  Wait for ECHO HIGH
    timeout = 30000;
    while(HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN) == GPIO_PIN_RESET)
    {
           if(--timeout == 0) return -1;
    }

    //  Start timer
    __HAL_TIM_SET_COUNTER(&htim3, 0);


    // Wait for ECHO LOW
    timeout = 30000;
    while(HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN) == GPIO_PIN_SET)
    {
        if(__HAL_TIM_GET_COUNTER(&htim3) > 30000) break;
    }
    //  Read duration
    duration = __HAL_TIM_GET_COUNTER(&htim3);

    //  Convert to distance
    distance = duration / 58;
    return distance;
}

void blink_led(int distance)
{
    // Turn OFF all LEDs first
    HAL_GPIO_WritePin(LED_PORT, LED1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_PORT, LED2, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_PORT, LED3, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_PORT, LED4, GPIO_PIN_RESET);

    if(distance <= 20 && distance > 15)
    {
        HAL_GPIO_WritePin(LED_PORT, LED1, GPIO_PIN_SET);
    }
    else if(distance <= 15 && distance > 10)
    {
        HAL_GPIO_WritePin(LED_PORT, LED1, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_PORT, LED2, GPIO_PIN_SET);
    }
    else if(distance <= 10 && distance > 5)
    {
        HAL_GPIO_WritePin(LED_PORT, LED1, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_PORT, LED2, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_PORT, LED3, GPIO_PIN_SET);
    }
    else if(distance <= 5)
    {
        HAL_GPIO_WritePin(LED_PORT, LED1, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_PORT, LED2, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_PORT, LED3, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_PORT, LED4, GPIO_PIN_SET);
    }
}
void uart_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();

    // UART pins
    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX;
    HAL_UART_Init(&huart2);

    // TRIG → OUTPUT
    GPIO_InitStruct.Pin = TRIG_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // ECHO → INPUT
    GPIO_InitStruct.Pin = ECHO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOB, TRIG_PIN, GPIO_PIN_RESET);
}

void led_init(void)
{
	__HAL_RCC_GPIOD_CLK_ENABLE();
	GPIO_InitTypeDef    GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = LED1;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init (LED_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LED2;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init (LED_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LED3;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init (LED_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LED4;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init (LED_PORT, &GPIO_InitStruct);
}


