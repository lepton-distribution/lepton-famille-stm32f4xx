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
#include <stdint.h>

#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/system.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/cpu.h"
#include "kernel/fs/vfs/vfstypes.h"
#include "kernel/core/malloc.h"

#include "stm32f4xx_hal.h"   /* en-tetes CMSIS et HAL ; USART et DMA pilotes par registres (ENV-35) */
#include "kernel/dev/arch/cortexm/stm32f4xx/types.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/gpio.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/dma.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/uart.h"

#include "kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_uart_x.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define uart_is_rx_hw_fc(Uart)      gpio_read_output((*Uart->Ctrl)->Gpio)
#define uart_set_rx_hw_fc(Uart)     gpio_set((*Uart->Ctrl)->Gpio)
#define uart_reset_rx_hw_fc(Uart)   gpio_reset((*Uart->Ctrl)->Gpio)
#define uart_tx_enable(Uart)        gpio_set((*Uart->Ctrl)->Gpio)
#define uart_tx_disable(Uart)       gpio_reset((*Uart->Ctrl)->Gpio)

//#define _malloc   malloc
//#define _free     free

/* Private variables & constants ---------------------------------------------*/
_Uart_Ctrl *Uart_Ctrl[UART_NB];

/* --- USART et DMA par registres (ENV-35, sans SPL ni module UART de la HAL, comme le F7).
   Memes reglages et memes calculs que USART_Init, USART_ChangeBaudrate, DMA_Init, DMA_DeInit et
   RCC_GetClocksFreq de la SPL V1.0.2 employes jusque-la (RM0090 6.3, 10.5, 30.6). */
static const u8 uart_ahb_apb_presc[16] = {0, 0, 0, 0, 1, 2, 3, 4, 1, 2, 3, 4, 6, 7, 8, 9};

/* horloge APB de l'USART (USART1 et USART6 sur APB2), d'apres les registres du RCC */
static u32 uart_apb_clock(const USART_TypeDef *UARTx)
{
  u32 sysclk, pllm, pllvco, pllp, hclk;
  switch (RCC->CFGR & RCC_CFGR_SWS)
  {
    case RCC_CFGR_SWS_HSE:
      sysclk = HSE_VALUE;
      break;
    case RCC_CFGR_SWS_PLL:
      pllm = RCC->PLLCFGR & RCC_PLLCFGR_PLLM;
      pllvco = ((RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC) ? HSE_VALUE : HSI_VALUE) / pllm
               * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> RCC_PLLCFGR_PLLN_Pos);
      pllp = (((RCC->PLLCFGR & RCC_PLLCFGR_PLLP) >> RCC_PLLCFGR_PLLP_Pos) + 1u) * 2u;
      sysclk = pllvco / pllp;
      break;
    default:
      sysclk = HSI_VALUE;
      break;
  }
  hclk = sysclk >> uart_ahb_apb_presc[(RCC->CFGR & RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos];
  if ((UARTx == USART1) || (UARTx == USART6))
    return hclk >> uart_ahb_apb_presc[(RCC->CFGR & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos];
  return hclk >> uart_ahb_apb_presc[(RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos];
}

/* BRR, calcul de la SPL (parties entiere et fractionnaire au centieme, sur-echantillonnage 8/16) */
static void uart_set_brr(USART_TypeDef *UARTx, u32 BaudRate)
{
  u32 apbclock = uart_apb_clock(UARTx);
  u32 integerdivider, fractionaldivider, tmpreg;
  if (UARTx->CR1 & USART_CR1_OVER8) integerdivider = (25u * apbclock) / (2u * BaudRate);
  else integerdivider = (25u * apbclock) / (4u * BaudRate);
  tmpreg = (integerdivider / 100u) << 4;
  fractionaldivider = integerdivider - (100u * (tmpreg >> 4));
  if (UARTx->CR1 & USART_CR1_OVER8) tmpreg |= (((fractionaldivider * 8u) + 50u) / 100u) & 0x07u;
  else tmpreg |= (((fractionaldivider * 16u) + 50u) / 100u) & 0x0Fu;
  UARTx->BRR = (u16)tmpreg;
}

/* remise a zero de l'USART par le RCC (USART_DeInit) */
static void uart_reset(const _Uart_Descriptor *Uart)
{
  *Uart->RCC_APBxRSTR |= Uart->RCC_APBxPeriph;
  *Uart->RCC_APBxRSTR &= ~Uart->RCC_APBxPeriph;
}

/* interruption USART active et signalee : validation dans CR1, indicateur dans SR
   (USART_GetITStatus) */
static int uart_it_status(const USART_TypeDef *UARTx, u32 cr1_ie, u32 sr_flag)
{
  return ((UARTx->CR1 & cr1_ie) && (UARTx->SR & sr_flag)) ? 1 : 0;
}

/* validation d'une interruption USART dans CR1 (USART_ITConfig) */
static void uart_it_config(USART_TypeDef *UARTx, u32 cr1_ie, int enable)
{
  if (enable) UARTx->CR1 |= cr1_ie;
  else UARTx->CR1 &= ~cr1_ie;
}

/* flux DMA a l'etat de reset, indicateurs effaces (DMA_DeInit) */
static void uart_dma_deinit(DMA_Stream_TypeDef *Stream)
{
  Stream->CR &= ~DMA_SxCR_EN;
  Stream->CR = 0;
  Stream->NDTR = 0;
  Stream->PAR = 0;
  Stream->M0AR = 0;
  Stream->M1AR = 0;
  Stream->FCR = 0x00000021u;
  dma_stream_clear_flags(Stream);
}

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : uart_open
* Description    : Open the UART port communication
* Input          : - Uart: Select the USART or the UART peripheral
*                : - BaudRate: Baud rate configuration
*                : - DmaBufSize: DMA buffer size
*                : - RxBufSize: Receive buffer size
*                : - TxBufSize: Transmit buffer size
*                : - HwCtrl: Hardware control options
*                : - Gpio: GPIO used for hardware control
* Output         : None
* Return         : 0 if OK, -1 in case of error
*******************************************************************************/
int uart_open (const _Uart_Descriptor *Uart, u32 BaudRate, u8 DmaBufSize, u16 RxBufSize, u16 TxBufSize, u8 HwCtrl, const _Gpio_Descriptor *Gpio)
{

  /* Init control variables and bufers */
  if (*Uart->Ctrl) _sys_free(*Uart->Ctrl);
  if ((*Uart->Ctrl = _sys_malloc(sizeof(_Uart_Ctrl) + DmaBufSize + RxBufSize + TxBufSize)) == 0) return(-1);
  memset(*Uart->Ctrl, 0, sizeof(_Uart_Ctrl));
  (*Uart->Ctrl)->DmaBufPtr = (char *)*Uart->Ctrl + sizeof(_Uart_Ctrl);
  (*Uart->Ctrl)->DmaBufSize = DmaBufSize;
  (*Uart->Ctrl)->iDma = DmaBufSize;
  (*Uart->Ctrl)->RxBufSize = RxBufSize;
  (*Uart->Ctrl)->RxBufPtr = (*Uart->Ctrl)->DmaBufPtr + (*Uart->Ctrl)->DmaBufSize;
  (*Uart->Ctrl)->TxBufSize = TxBufSize;
  (*Uart->Ctrl)->TxBufPtr = (*Uart->Ctrl)->RxBufPtr + (*Uart->Ctrl)->RxBufSize;
  (*Uart->Ctrl)->HwCtrl = HwCtrl;
  (*Uart->Ctrl)->Gpio = Gpio;
  #ifdef _UART_OS_SUPPORT
    (*Uart->Ctrl)->Event = SYS_EVT_INCOMING_DATA;
    (*Uart->Ctrl)->Task = sys_task_self();
  #endif

  /* Enable peripheral clock */
  *Uart->RCC_APBxENR |= Uart->RCC_APBxPeriph;

  /* Init GPIO */
  gpio_set_function(Uart->TxGpio, Uart->GPIO_AF);
  gpio_set_function(Uart->RxGpio, Uart->GPIO_AF);
  gpio_set_mode(Uart->TxGpio, GPIO_MODE_AF, 0);
  gpio_set_mode(Uart->RxGpio, GPIO_MODE_AF, 0);
  if ((*Uart->Ctrl)->HwCtrl & (UART_HW_FLOW_CTRL_RX | UART_HALF_DUPLEX)) gpio_set_mode((*Uart->Ctrl)->Gpio, GPIO_FCT_OUT, 0);

  /* Init UART peripheral */
  uart_reset(Uart);
  /* valeurs de USART_StructInit : 8 bits, 1 bit d'arret, sans parite, emission et reception */
  Uart->UARTx->CR2 &= ~USART_CR2_STOP;
  Uart->UARTx->CR1 = (Uart->UARTx->CR1
                      & ~(USART_CR1_M | USART_CR1_PCE | USART_CR1_PS | USART_CR1_TE | USART_CR1_RE))
                     | USART_CR1_TE | USART_CR1_RE;
  Uart->UARTx->CR3 = (Uart->UARTx->CR3 & ~(USART_CR3_CTSE | USART_CR3_RTSE))
                     | (((*Uart->Ctrl)->HwCtrl & UART_HW_FLOW_CTRL_TX) ? USART_CR3_CTSE : 0u);
  uart_set_brr(Uart->UARTx, BaudRate);

  /* Configure DMA (if used) */
  if ((*Uart->Ctrl)->DmaBufSize)
  {
    /* peripherique vers memoire, memoire incrementee, octets, circulaire, priorite moyenne,
       sans FIFO (reglages de DMA_StructInit et d'uart_open jusqu'a ENV-35) */
    uart_dma_deinit(Uart->DMAy_Streamx);
    Uart->DMAy_Streamx->CR = Uart->DMA_Channel | DMA_SxCR_MINC | DMA_SxCR_CIRC | DMA_SxCR_PL_0;
    Uart->DMAy_Streamx->FCR &= ~(DMA_SxFCR_DMDIS | DMA_SxFCR_FTH);
    Uart->DMAy_Streamx->NDTR = (*Uart->Ctrl)->DmaBufSize;
    Uart->DMAy_Streamx->PAR = (u32)&Uart->UARTx->DR;
    Uart->DMAy_Streamx->M0AR = (u32)(*Uart->Ctrl)->DmaBufPtr;
    Uart->DMAy_Streamx->CR |= DMA_SxCR_TCIE | DMA_SxCR_HTIE;
    Uart->DMAy_Streamx->CR |= DMA_SxCR_EN;
    NVIC_EnableIRQ(Uart->DMAx_IRQn);
    //
    //NVIC_SetPriority((IRQn_Type)Uart->DMAx_IRQn, (uint32_t)129);
    NVIC_SetPriority((IRQn_Type)Uart->DMAx_IRQn, (1 << __NVIC_PRIO_BITS) -3);
    //
    Uart->UARTx->CR3 |= USART_CR3_DMAR;
    uart_it_config(Uart->UARTx, USART_CR1_IDLEIE, 1);
  }
  else uart_it_config(Uart->UARTx, USART_CR1_RXNEIE, 1);

  /* Enable IT and start peripheral */
  //NVIC_InitStructure.NVIC_IRQChannel = Uart->IRQn;		 // we want to configure the USART1 interrupts
  //NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;// this sets the priority group of the USART1 interrupts
  //NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		 // this sets the subpriority inside the group
  //NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			 // the USART1 interrupts are globally enabled
  //NVIC_Init(&NVIC_InitStructure);	
  //
  //NVIC_SetPriority((IRQn_Type)Uart->IRQn, (uint32_t)129);
  NVIC_SetPriority((IRQn_Type)Uart->IRQn, (1 << __NVIC_PRIO_BITS) -4);
  //
   
  uart_it_config(Uart->UARTx, USART_CR1_TCIE, 1);
  NVIC_EnableIRQ(Uart->IRQn);
  Uart->UARTx->CR1 |= USART_CR1_UE;
  
  
  while (!gpio_read(Uart->TxGpio));

  return(0);
}

/*******************************************************************************
* Function Name  : uart_close
* Description    : Close the USART port communication
* Input          : - Uart: Select the USART or the UART peripheral.
* Output         : None
* Return         : 0
*******************************************************************************/
int uart_close(const _Uart_Descriptor *Uart)
{
  /* Disable IT */
  NVIC_DisableIRQ(Uart->IRQn);

  /* Free control variables and buffers */
  if (*Uart->Ctrl)
  {
    if ((*Uart->Ctrl)->DmaBufSize)
    {
      Uart->UARTx->CR3 &= ~USART_CR3_DMAR;
      NVIC_DisableIRQ(Uart->DMAx_IRQn);
      uart_dma_deinit(Uart->DMAy_Streamx);
    }
    if (((*Uart->Ctrl)->HwCtrl & UART_HW_FLOW_CTRL_RX) || ((*Uart->Ctrl)->HwCtrl & UART_HALF_DUPLEX)) gpio_init((*Uart->Ctrl)->Gpio);
    _sys_free(*Uart->Ctrl);
    *Uart->Ctrl = 0;
  }

  /* Deactivate UART peripheral */
  uart_reset(Uart);

  /* Disable peripheral clock */
  *Uart->RCC_APBxENR &= ~Uart->RCC_APBxPeriph;

  /* Configure GPIO to default state */
  gpio_init(Uart->TxGpio);
  gpio_init(Uart->RxGpio);

  return(0);
}
 
/*******************************************************************************
* Function Name  : uart_set_baudrate
* Description    : set baudrate of the UART port communication
* Input          : - Uart: Select the USART or the UART peripheral
*                : - BaudRate: Baud rate configuration
* Output         : None
* Return         : 0 if OK, -1 in case of error
*******************************************************************************/
int uart_set_baudrate (const _Uart_Descriptor *Uart, u32 BaudRate){
   //
   uart_set_brr(Uart->UARTx, BaudRate);
   //
   return(0);
}

/*******************************************************************************
* Function Name  : uart_irq_handler
* Description    : Interrupt handle function
* Input          : - Uart: Select the USART or the UART peripheral.
* Output         : None
* Return         : None
*******************************************************************************/
void uart_irq_handler(const _Uart_Descriptor *Uart)
{
  //
  __hw_enter_interrupt();
  //
  if (*Uart->Ctrl && (*Uart->Ctrl)->DmaBufSize)
  {
    if (uart_it_status(Uart->UARTx, USART_CR1_IDLEIE, USART_SR_IDLE))                          // Idle line
    {
      NVIC_SetPendingIRQ(Uart->DMAx_IRQn);
      (void)Uart->UARTx->DR;   /* lecture de SR puis de DR : efface IDLE */
    }
  }
  else if (uart_it_status(Uart->UARTx, USART_CR1_RXNEIE, USART_SR_RXNE))                       // Received Data Ready to be Read
  {
    if (*Uart->Ctrl && ((*Uart->Ctrl)->RxCnt < (*Uart->Ctrl)->RxBufSize))
    {
      (*Uart->Ctrl)->RxBufPtr[(*Uart->Ctrl)->RxiPut++] = (char)(Uart->UARTx->DR & 0x1FFu);
      //lepton
      if(!(*Uart->Ctrl)->RxCnt){
        if(Uart->board_uart_info && Uart->board_uart_info->desc_r!=-1)
           __fire_io_int(ofile_lst[Uart->board_uart_info->desc_r].owner_pthread_ptr_read);
      }
      //lepton
      (*Uart->Ctrl)->RxCnt++;
      if ((*Uart->Ctrl)->RxiPut >= (*Uart->Ctrl)->RxBufSize) (*Uart->Ctrl)->RxiPut = 0;
      if (((*Uart->Ctrl)->HwCtrl & UART_HW_FLOW_CTRL_RX) && ((*Uart->Ctrl)->RxCnt > ((*Uart->Ctrl)->RxBufSize - (*Uart->Ctrl)->DmaBufSize))) uart_set_rx_hw_fc(Uart);
    }
    else Uart->UARTx->SR = (u16)~USART_SR_RXNE;
    #ifdef _UART_OS_SUPPORT
      isr_evt_set((*Uart->Ctrl)->Event, (*Uart->Ctrl)->Task);
    #endif
  }

  if (uart_it_status(Uart->UARTx, USART_CR1_TXEIE, USART_SR_TXE))                             // Transmit Data Register Empty
  {
    if (*Uart->Ctrl && (*Uart->Ctrl)->TxCnt)
    {
      Uart->UARTx->DR = (u16)((*Uart->Ctrl)->TxBufPtr[(*Uart->Ctrl)->TxiGet++] & 0x1FFu);
      (*Uart->Ctrl)->TxCnt--;
      if ((*Uart->Ctrl)->TxiGet >= (*Uart->Ctrl)->TxBufSize) (*Uart->Ctrl)->TxiGet = 0;
    }
    else{
      uart_it_config(Uart->UARTx, USART_CR1_TXEIE, 0);
      //lepton
        if(Uart->board_uart_info && Uart->board_uart_info->desc_w!=-1)
           __fire_io_int(ofile_lst[Uart->board_uart_info->desc_w].owner_pthread_ptr_write);
      //lepton
    }
  }

  if (uart_it_status(Uart->UARTx, USART_CR1_TCIE, USART_SR_TC))                              // Transmission complete
  {
    if (*Uart->Ctrl && ((*Uart->Ctrl)->HwCtrl & UART_HALF_DUPLEX)) 
      uart_tx_disable(Uart);
    Uart->UARTx->SR = (u16)~USART_SR_TC;
  }
  //
  __hw_leave_interrupt();
}

/*******************************************************************************
* Function Name  : uart_dma_irq_handler
* Description    : DMA interrupt handle function
* Input          : - Uart: Select the USART or the UART peripheral.
* Output         : None
* Return         : None
*******************************************************************************/
void uart_dma_irq_handler(const _Uart_Descriptor *Uart){
  //
  __hw_enter_interrupt();
  //
  //
  if (*Uart->Ctrl)
  {
    while ((Uart->DMAy_Streamx->NDTR != (*Uart->Ctrl)->iDma) && ((*Uart->Ctrl)->RxCnt < (*Uart->Ctrl)->RxBufSize))
    {
      (*Uart->Ctrl)->RxBufPtr[(*Uart->Ctrl)->RxiPut++] = (*Uart->Ctrl)->DmaBufPtr[(*Uart->Ctrl)->DmaBufSize - (*Uart->Ctrl)->iDma--];
      if (!(*Uart->Ctrl)->iDma) (*Uart->Ctrl)->iDma = (*Uart->Ctrl)->DmaBufSize;
      //lepton
      if(!(*Uart->Ctrl)->RxCnt){
        if(Uart->board_uart_info && Uart->board_uart_info->desc_r!=-1)
           __fire_io_int(ofile_lst[Uart->board_uart_info->desc_r].owner_pthread_ptr_read);
      }
      //lepton
      (*Uart->Ctrl)->RxCnt++;
      if ((*Uart->Ctrl)->RxiPut >= (*Uart->Ctrl)->RxBufSize){
         (*Uart->Ctrl)->RxiPut = 0;
      }
      if (((*Uart->Ctrl)->HwCtrl & UART_HW_FLOW_CTRL_RX) && ((*Uart->Ctrl)->RxCnt > ((*Uart->Ctrl)->RxBufSize - (*Uart->Ctrl)->DmaBufSize))) {
         uart_set_rx_hw_fc(Uart);
      }
    }
  }
  #ifdef _UART_OS_SUPPORT
    isr_evt_set((*Uart->Ctrl)->Event, (*Uart->Ctrl)->Task);
  #endif
  //phlb:20190701: to debug : fix stream_(I) -> DMA_IT_ALL_(I)
  // for nuodio: 
  //DMA_ClearITPendingBit(Uart->DMAy_Streamx, DMA_IT_ALL_5);
  //for techne/hrc2demo:
  //DMA_ClearITPendingBit(Uart->DMAy_Streamx,DMA_IT_ALL_1);
  //for  bret interface usart1
  //DMA_ClearITPendingBit(Uart->DMAy_Streamx,DMA_IT_ALL_2);
  
   //fix here (ENV-35 : indicateurs du flux calcules d'apres son adresse, DMA1 ou DMA2)
   dma_stream_clear_flags(Uart->DMAy_Streamx);
   //
   __hw_leave_interrupt();
   //
}

/*******************************************************************************
* Function Name  : uart_get_char
* Description    : Get a single data byte from the USARTx RxBuffer.
* Input          : - Uart: Select the USART or the UART peripheral.
* Output         : None
* Return         : The data read from the buffer (-1 if the RxBuffer is empty)
*******************************************************************************/
int uart_get_char(const _Uart_Descriptor *Uart)
{
  char data;

  #ifdef _UART_OS_SUPPORT
    if (!(*Uart->Ctrl) || (sys_task_self() != (*Uart->Ctrl)->Task) || !(*Uart->Ctrl)->RxCnt) return(-1);
  #else
    if (!(*Uart->Ctrl) || !(*Uart->Ctrl)->RxCnt) return(-1);
  #endif
  //
  if ((*Uart->Ctrl)->DmaBufSize) 
    NVIC_DisableIRQ(Uart->DMAx_IRQn);
  else 
    uart_it_config(Uart->UARTx, USART_CR1_RXNEIE, 0);
  //
  data = (*Uart->Ctrl)->RxBufPtr[(*Uart->Ctrl)->RxiGet++];
  (*Uart->Ctrl)->RxCnt--;
  //
  if ((*Uart->Ctrl)->RxiGet >= (*Uart->Ctrl)->RxBufSize) 
    (*Uart->Ctrl)->RxiGet = 0;
  //
  if (((*Uart->Ctrl)->HwCtrl & UART_HW_FLOW_CTRL_RX) && (uart_is_rx_hw_fc(Uart)) && ((*Uart->Ctrl)->RxCnt < ((*Uart->Ctrl)->RxBufSize / 2))) 
    uart_reset_rx_hw_fc(Uart);
  //
  if ((*Uart->Ctrl)->DmaBufSize) 
    NVIC_EnableIRQ(Uart->DMAx_IRQn);
  else 
    uart_it_config(Uart->UARTx, USART_CR1_RXNEIE, 1);
  
  return((int)data);
}

/*******************************************************************************
* Function Name  : uart_put_char
* Description    : Put a single data byte to the USARTx TxBuffer.
* Input          : - Data: the data to transmit.
*                  - Uart: Select the USART or the UART peripheral.
* Output         : None
* Return         : 0 if OK, -1 if ERROR
*******************************************************************************/
int uart_put_char(char Data, const _Uart_Descriptor *Uart)
{
  if (!(*Uart->Ctrl) || !(*Uart->Ctrl)->TxBufPtr) 
    return(-1);
  //
  while ((*Uart->Ctrl)->TxCnt == (*Uart->Ctrl)->TxBufSize)
  {
    #ifdef _UART_OS_SUPPORT
      sys_wait(1);
    #endif
  }
  //
  //USART_ITConfig(Uart->UARTx, USART_IT_TXE, DISABLE);
  //
  (*Uart->Ctrl)->TxBufPtr[(*Uart->Ctrl)->TxiPut++] = Data;
  (*Uart->Ctrl)->TxCnt++;
  //
  if ((*Uart->Ctrl)->TxiPut >= (*Uart->Ctrl)->TxBufSize) 
    (*Uart->Ctrl)->TxiPut = 0;
  //
  //if ((*Uart->Ctrl)->HwCtrl & UART_HALF_DUPLEX) 
  //  uart_tx_enable(Uart);
  //
  //USART_ITConfig(Uart->UARTx, USART_IT_TXE, ENABLE);
  return(0);
}

/*******************************************************************************
* Function Name  : uart_read
* Description    : Read data bytes from the USARTx RxBuffer.
* Input          : - WrPtr: Location where the first byte is to be stored
*                : - Size: Number of bytes to be read
*                : - Uart: Select the USART or the UART peripheral.
* Output         : None
* Return         : Number of bytes read from the UARTx RxBuffer (-1 if the number
*                  of bytes to be read superior to the number of bytes available)
*******************************************************************************/
int uart_read(void *WrPtr, u16 Size, const _Uart_Descriptor *Uart)
{
  u16 i;
  char *ptr;
  int ch;
  int cb;

  //
  if (!(*Uart->Ctrl)) 
    return(-1);
  //
  if((*Uart->Ctrl)->RxCnt>Size)
    cb=Size;
  else
    cb=(*Uart->Ctrl)->RxCnt;
  //
  ptr = (char *)WrPtr;
  //
  for (i = 0 ; i < cb ; i++)
  {
    if ((ch = uart_get_char(Uart)) == -1) 
      break;
    *ptr++ = (char)ch;
  }
  return((int)(i));
}

/*******************************************************************************
* Function Name  : uart_write
* Description    : Write data bytes to the USARTx TxBuffer.
* Input          : - RdPtr: Location where the first byte is to be read
*                : - Size: Number of bytes to be written
*                : - Uart: Select the USART or the UART peripheral.
* Output         : None
* Return         : -1 if ERROR, Number of char not written (0 if OK)
*******************************************************************************/
int uart_write(const void *RdPtr, u16 Size, const _Uart_Descriptor *Uart)
{
  u16 i;
  char *ptr = (char *)RdPtr;
  
  if(Size>(*Uart->Ctrl)->TxBufSize)
     Size=(*Uart->Ctrl)->TxBufSize;

  //
  uart_it_config(Uart->UARTx, USART_CR1_TXEIE, 0);
  //
  for (i = 0 ; i < Size ; i++){ 
    if (uart_put_char(*ptr++, Uart) == -1) 
      break;
  }
  //
  if ((*Uart->Ctrl)->HwCtrl & UART_HALF_DUPLEX) 
    uart_tx_enable(Uart);
  //
  uart_it_config(Uart->UARTx, USART_CR1_TXEIE, 1);
  //
  return((int)(i));
}

/*******************************************************************************
* Function Name  : uart_flush_rx
* Description    :flush rx buffer
* Input          : - Uart: Select the USART or the UART peripheral.
* Output         : None
* Return         : 
* Note: lepton development
*******************************************************************************/
int uart_flush_rx(const _Uart_Descriptor *Uart)
{

  #ifdef _UART_OS_SUPPORT
    if (!(*Uart->Ctrl) || (sys_task_self() != (*Uart->Ctrl)->Task) || !(*Uart->Ctrl)->RxCnt) return(0);
  #else
    if (!(*Uart->Ctrl) || !(*Uart->Ctrl)->RxCnt) return(0);
  #endif
  //
  if ((*Uart->Ctrl)->DmaBufSize) 
    NVIC_DisableIRQ(Uart->DMAx_IRQn);
  else 
    uart_it_config(Uart->UARTx, USART_CR1_RXNEIE, 0);
  //
  (*Uart->Ctrl)->RxCnt=0;
  //
  (*Uart->Ctrl)->RxiGet = 0;
  //
  if (((*Uart->Ctrl)->HwCtrl & UART_HW_FLOW_CTRL_RX) && (uart_is_rx_hw_fc(Uart)) && ((*Uart->Ctrl)->RxCnt < ((*Uart->Ctrl)->RxBufSize / 2))) 
    uart_reset_rx_hw_fc(Uart);
  //
  if ((*Uart->Ctrl)->DmaBufSize) 
    NVIC_EnableIRQ(Uart->DMAx_IRQn);
  else 
    uart_it_config(Uart->UARTx, USART_CR1_RXNEIE, 1);
  
  return 0;
}
