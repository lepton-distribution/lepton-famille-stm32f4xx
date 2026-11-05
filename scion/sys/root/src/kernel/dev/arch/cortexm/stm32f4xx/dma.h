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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DMA_H
#define __DMA_H

/* Includes ------------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Indicateurs d'un flux (FEIF, DMEIF, TEIF, HTIF, TCIF : 0x3D) dans LIFCR (flux 0-3) ou HIFCR
   (flux 4-7), decales de 0, 6, 16 ou 22 bits (RM0090 10.5.3-4) ; registres CMSIS, sans SPL ni
   HAL (ENV-35). Flux : DMAy_Streamx = DMAy_BASE + 0x10 + 0x18 * x. */
#define DMA_STREAM_FLAGS_ALL  0x3Du

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define dma_startup_init()  (RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN | RCC_AHB1ENR_DMA2EN)

/* efface tous les indicateurs d'interruption du flux (equivalent de DMA_ClearITPendingBit de la
   SPL avec tous les indicateurs du flux) */
static __inline void dma_stream_clear_flags(DMA_Stream_TypeDef *Stream)
{
  static const unsigned char shift[4] = {0, 6, 16, 22};
  DMA_TypeDef *dma = (DMA_TypeDef *)((uint32_t)Stream & ~(uint32_t)0x3FF);
  uint32_t x = (((uint32_t)Stream & 0x3FFu) - 0x10u) / 0x18u;
  if (x < 4u) dma->LIFCR = DMA_STREAM_FLAGS_ALL << shift[x];
  else dma->HIFCR = DMA_STREAM_FLAGS_ALL << shift[x - 4u];
}

/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

#endif /* __DMA_H */
