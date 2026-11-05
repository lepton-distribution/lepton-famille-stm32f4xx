/*
The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
specific language governing rights and limitations under the License.

The Original Code is Lepton.

The Initial Developer of the Original Code is Philippe Le Boulanger.
Portions created by Philippe Le Boulanger are Copyright (C) 2014 <lepton.phlb@gmail.com>.
All Rights Reserved.

Contributor(s): Jean-Jacques Pitrolle <lepton.jjp@gmail.com>.

Alternatively, the contents of this file may be used under the terms of the eCos GPL license
(the  [eCos GPL] License), in which case the provisions of [eCos GPL] License are applicable
instead of those above. If you wish to allow use of your version of this file only under the
terms of the [eCos GPL] License and not to allow others to use your version of this file under
the MPL, indicate your decision by deleting  the provisions above and replace
them with the notice and other provisions required by the [eCos GPL] License.
If you do not delete the provisions above, a recipient may use your version of this file under
either the MPL or the [eCos GPL] License."
*/


/* Includes ------------------------------------------------------------------*/
#include "kernel/core/kernelconf.h"   /* configuration de carte (_GPIO_DEFAULT_SPEED), incluse par
                                       driverlib/stm32f4xx.h jusqu'a ENV-35 */
#include "stm32f4xx_hal.h"   /* HAL STM32F4 (GPIO), ENV-35 : remplace la SPL (driverlib) */
#include "kernel/core/compiler.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/types.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/gpio.h"

/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
#pragma anon_unions
typedef union __lepton_packed
{
  u32 Val;
  struct __lepton_packed
  {
    u32 Function  :8;
    u32 Type      :8;
    u32 PuPd      :8;
    u32 Speed     :7;
    u32 SetSpeed  :1;
  };
} _Gpio_Mode;

/* Private macro -------------------------------------------------------------*/
#define gpio_is_std(Gpio)   (Gpio->Type == GPIO_TYPE_STD)
#define gpio_is_i2c(Gpio)   (Gpio->Type == GPIO_TYPE_I2C)

#define gpio_is_output(Mode)  ((Mode)->Function == GPIO_FCT_OUT)

/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

#if 0
/*******************************************************************************
* Function Name  : gpio_startup_init
* Description    : Initialize all defined GPIOs
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void gpio_startup_init(void)
{
  u8 i;

  /* Enable GPIO clocks */
  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | \
                          RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_GPIOH | RCC_AHB1Periph_GPIOI, ENABLE);

  /* Initialize GPIO */
  for (i = 0 ; i < GPIO_NB ; i++) if (Gpio_Descriptor[i].Init) gpio_init((&Gpio_Descriptor[i]));
}
#endif

/*******************************************************************************
* Function Name  : gpio_set_mode
* Description    : Set the mode of a specified GPIO
* Input          : - Gpio: Select the GPIO function to be configured
*                  - Mode: Config mode (in, out, open-drain, ...)
*                  - Val: Value to set if pin is configured as an output
* Output         : None
* Return         : None
*******************************************************************************/
void gpio_set_mode(const _Gpio_Descriptor *Gpio, u32 Mode, u8 Val)
{
  GPIO_InitTypeDef gpio_init_structure;
  GPIO_TypeDef *port;
  _Gpio_Mode mode;
  u8 pos = 0;

  if (!Gpio) return;
  mode.Val = Mode;

  #ifdef _GPIO_I2CIO_SUPPORT
    if (gpio_is_i2c(Gpio))
    {
      if (gpio_is_output(&mode)) i2cio_set_mode(Gpio->Port, Gpio->GPIO_Pin, I2CIO_MODE_OUTPUT, Val);
      else i2cio_set_mode(Gpio->Port, Gpio->GPIO_Pin, I2CIO_MODE_INPUT, 0);
      return;
    }
  #endif

  /* Champs du mode Lepton = valeurs des registres (MODER, OTYPER, PUPDR, OSPEEDR), comme pour la
     SPL : traduits en mode HAL. La fonction alternative choisie par gpio_set_function (AFR) est
     conservee : HAL_GPIO_Init la reecrit en mode AF. */
  port = (GPIO_TypeDef *)Gpio->Port;
  while (pos < 15 && !(Gpio->GPIO_Pin & (1u << pos))) pos++;
  gpio_init_structure.Pin = Gpio->GPIO_Pin;
  switch (mode.Function)
  {
    case GPIO_FCT_OUT:
      gpio_init_structure.Mode = mode.Type ? GPIO_MODE_OUTPUT_OD : GPIO_MODE_OUTPUT_PP;
      break;
    case GPIO_FCT_AF:
      gpio_init_structure.Mode = mode.Type ? GPIO_MODE_AF_OD : GPIO_MODE_AF_PP;
      break;
    case GPIO_FCT_AN:
      gpio_init_structure.Mode = GPIO_MODE_ANALOG;
      break;
    default:
      gpio_init_structure.Mode = GPIO_MODE_INPUT;
      break;
  }
  gpio_init_structure.Pull = mode.PuPd;
  if (mode.SetSpeed) gpio_init_structure.Speed = mode.Speed;
  else gpio_init_structure.Speed = _GPIO_DEFAULT_SPEED;
  gpio_init_structure.Alternate = (port->AFR[pos >> 3] >> ((pos & 7u) * 4u)) & 0xFu;
  HAL_GPIO_Init(port, &gpio_init_structure);
  if (gpio_is_output(&mode))
    HAL_GPIO_WritePin(port, Gpio->GPIO_Pin, Val ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/*******************************************************************************
* Function Name  : gpio_init
* Description    : Initialize a specified GPIO to its default mode and value
* Input          : - Gpio: Select the GPIO to be initialized
* Output         : None
* Return         : None
*******************************************************************************/
void gpio_init(const _Gpio_Descriptor *Gpio)
{
  if (!Gpio) return;
  gpio_set_mode(Gpio, Gpio->DefMode, Gpio->DefVal);
}

/*******************************************************************************
* Function Name  : gpio_set
* Description    : Set the value of a specified GPIO
* Input          : - Gpio: Select the GPIO to be set
* Output         : None
* Return         : None
*******************************************************************************/
void gpio_set(const _Gpio_Descriptor *Gpio)
{
  if (!Gpio) return;
  #ifdef _GPIO_I2CIO_SUPPORT
    if (gpio_is_i2c(Gpio)) i2cio_set_output(Gpio->Port, Gpio->GPIO_Pin, 1);
    else HAL_GPIO_WritePin((GPIO_TypeDef *)Gpio->Port, Gpio->GPIO_Pin, GPIO_PIN_SET);
  #else
    HAL_GPIO_WritePin((GPIO_TypeDef *)Gpio->Port, Gpio->GPIO_Pin, GPIO_PIN_SET);
  #endif
}

/*******************************************************************************
* Function Name  : gpio_reset
* Description    : Reset the value of a specified GPIO
* Input          : - Gpio: Select the GPIO to be reset
* Output         : None
* Return         : None
*******************************************************************************/
void gpio_reset(const _Gpio_Descriptor *Gpio)
{
  if (!Gpio) return;
  #ifdef _GPIO_I2CIO_SUPPORT
    if (gpio_is_i2c(Gpio)) i2cio_set_output(Gpio->Port, Gpio->GPIO_Pin, 0);
    else HAL_GPIO_WritePin((GPIO_TypeDef *)Gpio->Port, Gpio->GPIO_Pin, GPIO_PIN_RESET);
  #else
    HAL_GPIO_WritePin((GPIO_TypeDef *)Gpio->Port, Gpio->GPIO_Pin, GPIO_PIN_RESET);
  #endif
}

/*******************************************************************************
* Function Name  : gpio_toggle
* Description    : Toggle the value of a specified GPIO
* Input          : - Gpio: Select the GPIO to be toggled
* Output         : None
* Return         : None
*******************************************************************************/
void gpio_toggle(const _Gpio_Descriptor *Gpio)
{
  if (!Gpio) return;
  #ifdef _GPIO_I2CIO_SUPPORT
    if (gpio_is_i2c(Gpio)) i2cio_toggle_output(Gpio->Port, Gpio->GPIO_Pin);
    else HAL_GPIO_TogglePin((GPIO_TypeDef *)Gpio->Port, Gpio->GPIO_Pin);
  #else
    HAL_GPIO_TogglePin((GPIO_TypeDef *)Gpio->Port, Gpio->GPIO_Pin);
  #endif
}

/*******************************************************************************
* Function Name  : gpio_read
* Description    : Read the input value of a specified GPIO
* Input          : - Gpio: Select the GPIO to be read
* Output         : None
* Return         : None
*******************************************************************************/
u8 gpio_read(const _Gpio_Descriptor *Gpio)
{
  if (!Gpio) return(0);
  #ifdef _GPIO_I2CIO_SUPPORT
    if (gpio_is_i2c(Gpio)) return(i2cio_read_register_bit(Gpio->Port, I2CIO_REG_INPUT, Gpio->GPIO_Pin));
  #endif
  return((u8)HAL_GPIO_ReadPin((GPIO_TypeDef *)Gpio->Port, Gpio->GPIO_Pin));
}

/*******************************************************************************
* Function Name  : gpio_read_output
* Description    : Read the output value of a specified GPIO
* Input          : - Gpio: Select the GPIO to be read
* Output         : None
* Return         : None
*******************************************************************************/
u8 gpio_read_output(const _Gpio_Descriptor *Gpio)
{
  if (!Gpio) return(0);
  #ifdef _GPIO_I2CIO_SUPPORT
    if (gpio_is_i2c(Gpio)) return(i2cio_read_register_bit(Gpio->Port, I2CIO_REG_OUTPUT, Gpio->GPIO_Pin));
  #endif
  return((((GPIO_TypeDef *)Gpio->Port)->ODR & Gpio->GPIO_Pin) ? 1 : 0);   /* pas d'equivalent HAL */
}

/*******************************************************************************
* Function Name  : gpio_set_function
* Description    : Changes the mapping of a specified GPIO
* Input          : - Gpio: Select the GPIO function to be configured
*                  - Function: Select a function to associate with the GPIO
* Output         : None
* Return         : None
*******************************************************************************/
void gpio_set_function(const _Gpio_Descriptor *Gpio, u8 Function)
{
  u8 i = 0;

  while ((1 << i) < Gpio->GPIO_Pin) i++;
  /* AFR[i/8], champ de 4 bits (RM0090 8.4.9-10) ; pas d'equivalent HAL hors HAL_GPIO_Init */
  GPIO_TypeDef *port = (GPIO_TypeDef *)Gpio->Port;
  port->AFR[i >> 3] = (port->AFR[i >> 3] & ~(0xFu << ((i & 7u) * 4u)))
                      | ((u32)Function << ((i & 7u) * 4u));
}

// /*******************************************************************************
// * Function Name  : gpio_set_port_mode
// * Description    : Set the mode for a GPIO port
// * Input          : - Gpio: Select a GPIO of the port to be configured
// *                  - Mode: Config mode (in, out, open-drain, ...)
// * Output         : None
// * Return         : None
// *******************************************************************************/
// void gpio_set_port_mode(const _Gpio_Descriptor *Gpio, GPIOMode_TypeDef Mode)
// {
//   GPIO_InitTypeDef gpio_init_structure;

//   if (!Gpio) return;
//   #ifdef _GPIO_I2CIO_SUPPORT
//     if (gpio_is_i2c(Gpio))
//     {
//       if ((Mode == GPIO_Mode_Out_PP) || (Mode == GPIO_Mode_Out_OD)) i2cio_set_mode(Gpio->Port, 0xFFFF, I2CIO_MODE_OUTPUT, 0);
//       else i2cio_set_mode(Gpio->Port, 0xFFFF, I2CIO_MODE_INPUT, 0);
//       return;
//     }
//   #endif
//   gpio_init_structure.GPIO_Pin = 0xFFFF;
//   gpio_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
//   gpio_init_structure.GPIO_Mode = Mode;
//   GPIO_Init((GPIO_TypeDef *)Gpio->Port, &gpio_init_structure);
//   if ((Mode == GPIO_Mode_Out_PP) || (Mode == GPIO_Mode_Out_OD)) GPIO_Write((GPIO_TypeDef *)Gpio->Port, 0);
// }

// /*******************************************************************************
// * Function Name  : gpio_read_port
// * Description    : Read the input value of a specified GPIO port
// * Input          : - Gpio: Select the GPIO port to be read
// * Output         : None
// * Return         : None
// *******************************************************************************/
// u16 gpio_read_port(const _Gpio_Descriptor *Gpio)
// {
//   if (!Gpio) return(0);
//   #ifdef _GPIO_I2CIO_SUPPORT
//     if (gpio_is_i2c(Gpio)) return(i2cio_read_register(Gpio->Port, I2CIO_REG_INPUT));
//   #endif
//   return(GPIO_ReadInputData((GPIO_TypeDef *)Gpio->Port));
// }

// /*******************************************************************************
// * Function Name  : gpio_enable_ext_int
// * Description    : Configure and enable an external interrupt line
// * Input          : - ExtInt: Select the EXTI line to be configured
// * Output         : None
// * Return         : None
// *******************************************************************************/
// void gpio_enable_ext_int(const _Gpio_ExtInt_Descriptor *ExtInt)
// {
//   EXTI_InitTypeDef exti_init_structure;

//   GPIO_EXTILineConfig(ExtInt->GPIO_Port, ExtInt->GPIO_Pin);
//   EXTI_ClearITPendingBit(ExtInt->EXTI_Line);
//   exti_init_structure.EXTI_Line = ExtInt->EXTI_Line;
//   exti_init_structure.EXTI_Mode = ExtInt->EXTI_Mode;
//   exti_init_structure.EXTI_Trigger = ExtInt->EXTI_Trigger;
//   exti_init_structure.EXTI_LineCmd = ENABLE;
//   EXTI_Init(&exti_init_structure);
//   NVIC_EnableIRQ(ExtInt->IRQn);
// }

// /*******************************************************************************
// * Function Name  : gpio_disable_ext_int
// * Description    : Disable an external interrupt line
// * Input          : - ExtInt: Select the EXTI line to be configured
// * Output         : None
// * Return         : None
// *******************************************************************************/
// void gpio_disable_ext_int(const _Gpio_ExtInt_Descriptor *ExtInt)
// {
//   EXTI_InitTypeDef exti_init_structure;

//   NVIC_DisableIRQ(ExtInt->IRQn);
//   exti_init_structure.EXTI_Line = ExtInt->EXTI_Line;
//   exti_init_structure.EXTI_LineCmd = DISABLE;
//   EXTI_Init(&exti_init_structure);
//   EXTI_ClearITPendingBit(ExtInt->EXTI_Line);
// }

