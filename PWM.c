#include <stm32g0xx_hal.h>
#include "stm32g0xx_hal_tim.h"
#include "pwm.h"

TIM_HandleTypeDef htim1;
volatile uint32_t pwm_duty_cycle = 0;  // Variable to hold the current duty cycle

static void MX_TIM1_Init(void);
void Error_Handler(void);  // Declare Error_Handler function

void blink_led_pwm(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_TIM1_Init();

  // Start PWM output
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}

static void MX_TIM1_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 72 - 1;  // Prescaler set to 72 to get 1 MHz timer clock
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 100 - 1;  // Period set to 100 for 10 kHz frequency
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();  // Call Error_Handler on error
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();  // Call Error_Handler on error
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = pwm_duty_cycle;  // Initial duty cycle set to 0%
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();  // Call Error_Handler on error
  }

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.Break2AFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;

  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();  // Call Error_Handler on error
  }

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOD_CLK_ENABLE();

  // PD2     ------> TIM1_CH1N (PWM output)

  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  // Configure SysTick Timer to call the callback function every 500 ms
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 2);  // 500 ms
}

// SysTick interrupt handler
void SysTick_Handler(void)
{
  static uint32_t counter = 0;
  
  counter++;

  // Toggle the duty cycle every 500 ms
  if (counter >= 500)
  {
    counter = 0;
    pwm_duty_cycle = (pwm_duty_cycle == 0) ? 50 : 0;  // Toggle between 0% and 50%
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm_duty_cycle);  // Update PWM duty cycle
  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
    // Loop forever in case of error
  }
}
