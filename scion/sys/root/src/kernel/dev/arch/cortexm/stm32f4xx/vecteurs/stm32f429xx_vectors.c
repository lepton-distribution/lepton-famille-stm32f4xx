/*
 * Lepton — vecteurs d'interruption des STM32F429 et STM32F439 (famille stm32f4xx ; ENV-6, table
 * complète ENV-20). Licence : voir LICENSE (MPL 1.1).
 *
 * Le démarrage générique ARMv7-M (kernel/core/arch/cortexm/startup_armv7m.c) fournit la table
 * des exceptions et des IRQ 0 à 63 sous les noms IRQ<n>_Handler (faibles). Ce fichier :
 *  - déclare les 91 gestionnaires aux noms CMSIS, faibles, alias d'un relais vers Default_Handler :
 *    un pilote ou un BSP fournit un gestionnaire en définissant son nom CMSIS en fort
 *    (USART3_IRQHandler…), sans toucher à ce fichier ;
 *  - définit les relais IRQ<n>_Handler (n < 64) vers ces noms ;
 *  - prolonge la table pour les IRQ 64 à 90 (section .isr_vector_ext, placée juste après
 *    .isr_vector par ld/common-cortexm.ld), directement vers les noms CMSIS.
 * Une seule table pour les deux MCU (le lecteur CMake lie tous les fichiers « vecteurs » de la
 * famille, ENV-20 PA-27) : seule différence, l'IRQ 79 (CRYP), réservée sur le STM32F429.
 * Noms et numéros : startup/iar/startup_stm32f439xx.s (CMSIS device ST V2.3.0) ; RM0090.
 * Un nom faible défini ici n'entraîne pas l'extraction, depuis une archive, du membre qui le
 * définit en fort : ce membre doit être tiré par un autre symbole (ETH_IRQHandler :
 * dev_stm32f4xx_eth.c, tiré par dev_stm32f4xx_eth_map), ou le gestionnaire lié comme objet (BSP).
 */
#include <stdint.h>

void Default_Handler(void);   /* startup_armv7m.c */

/* cible locale des alias faibles (un alias exige une cible définie dans la même unité) */
static void stm32f4xx_irq_default(void) { Default_Handler(); }

#define STM32F4XX_WEAK_IRQ(name) void name(void) __attribute__((weak, alias("stm32f4xx_irq_default")))

STM32F4XX_WEAK_IRQ(WWDG_IRQHandler);               /*  0 */
STM32F4XX_WEAK_IRQ(PVD_IRQHandler);                /*  1 */
STM32F4XX_WEAK_IRQ(TAMP_STAMP_IRQHandler);         /*  2 */
STM32F4XX_WEAK_IRQ(RTC_WKUP_IRQHandler);           /*  3 */
STM32F4XX_WEAK_IRQ(FLASH_IRQHandler);              /*  4 */
STM32F4XX_WEAK_IRQ(RCC_IRQHandler);                /*  5 */
STM32F4XX_WEAK_IRQ(EXTI0_IRQHandler);              /*  6 */
STM32F4XX_WEAK_IRQ(EXTI1_IRQHandler);              /*  7 */
STM32F4XX_WEAK_IRQ(EXTI2_IRQHandler);              /*  8 */
STM32F4XX_WEAK_IRQ(EXTI3_IRQHandler);              /*  9 */
STM32F4XX_WEAK_IRQ(EXTI4_IRQHandler);              /* 10 */
STM32F4XX_WEAK_IRQ(DMA1_Stream0_IRQHandler);       /* 11 */
STM32F4XX_WEAK_IRQ(DMA1_Stream1_IRQHandler);       /* 12 */
STM32F4XX_WEAK_IRQ(DMA1_Stream2_IRQHandler);       /* 13 */
STM32F4XX_WEAK_IRQ(DMA1_Stream3_IRQHandler);       /* 14 */
STM32F4XX_WEAK_IRQ(DMA1_Stream4_IRQHandler);       /* 15 */
STM32F4XX_WEAK_IRQ(DMA1_Stream5_IRQHandler);       /* 16 */
STM32F4XX_WEAK_IRQ(DMA1_Stream6_IRQHandler);       /* 17 */
STM32F4XX_WEAK_IRQ(ADC_IRQHandler);                /* 18 */
STM32F4XX_WEAK_IRQ(CAN1_TX_IRQHandler);            /* 19 */
STM32F4XX_WEAK_IRQ(CAN1_RX0_IRQHandler);           /* 20 */
STM32F4XX_WEAK_IRQ(CAN1_RX1_IRQHandler);           /* 21 */
STM32F4XX_WEAK_IRQ(CAN1_SCE_IRQHandler);           /* 22 */
STM32F4XX_WEAK_IRQ(EXTI9_5_IRQHandler);            /* 23 */
STM32F4XX_WEAK_IRQ(TIM1_BRK_TIM9_IRQHandler);      /* 24 */
STM32F4XX_WEAK_IRQ(TIM1_UP_TIM10_IRQHandler);      /* 25 */
STM32F4XX_WEAK_IRQ(TIM1_TRG_COM_TIM11_IRQHandler); /* 26 */
STM32F4XX_WEAK_IRQ(TIM1_CC_IRQHandler);            /* 27 */
STM32F4XX_WEAK_IRQ(TIM2_IRQHandler);               /* 28 */
STM32F4XX_WEAK_IRQ(TIM3_IRQHandler);               /* 29 */
STM32F4XX_WEAK_IRQ(TIM4_IRQHandler);               /* 30 */
STM32F4XX_WEAK_IRQ(I2C1_EV_IRQHandler);            /* 31 */
STM32F4XX_WEAK_IRQ(I2C1_ER_IRQHandler);            /* 32 */
STM32F4XX_WEAK_IRQ(I2C2_EV_IRQHandler);            /* 33 */
STM32F4XX_WEAK_IRQ(I2C2_ER_IRQHandler);            /* 34 */
STM32F4XX_WEAK_IRQ(SPI1_IRQHandler);               /* 35 */
STM32F4XX_WEAK_IRQ(SPI2_IRQHandler);               /* 36 */
STM32F4XX_WEAK_IRQ(USART1_IRQHandler);             /* 37 */
STM32F4XX_WEAK_IRQ(USART2_IRQHandler);             /* 38 */
STM32F4XX_WEAK_IRQ(USART3_IRQHandler);             /* 39 */
STM32F4XX_WEAK_IRQ(EXTI15_10_IRQHandler);          /* 40 */
STM32F4XX_WEAK_IRQ(RTC_Alarm_IRQHandler);          /* 41 */
STM32F4XX_WEAK_IRQ(OTG_FS_WKUP_IRQHandler);        /* 42 */
STM32F4XX_WEAK_IRQ(TIM8_BRK_TIM12_IRQHandler);     /* 43 */
STM32F4XX_WEAK_IRQ(TIM8_UP_TIM13_IRQHandler);      /* 44 */
STM32F4XX_WEAK_IRQ(TIM8_TRG_COM_TIM14_IRQHandler); /* 45 */
STM32F4XX_WEAK_IRQ(TIM8_CC_IRQHandler);            /* 46 */
STM32F4XX_WEAK_IRQ(DMA1_Stream7_IRQHandler);       /* 47 */
STM32F4XX_WEAK_IRQ(FMC_IRQHandler);                /* 48 */
STM32F4XX_WEAK_IRQ(SDIO_IRQHandler);               /* 49 */
STM32F4XX_WEAK_IRQ(TIM5_IRQHandler);               /* 50 */
STM32F4XX_WEAK_IRQ(SPI3_IRQHandler);               /* 51 */
STM32F4XX_WEAK_IRQ(UART4_IRQHandler);              /* 52 */
STM32F4XX_WEAK_IRQ(UART5_IRQHandler);              /* 53 */
STM32F4XX_WEAK_IRQ(TIM6_DAC_IRQHandler);           /* 54 */
STM32F4XX_WEAK_IRQ(TIM7_IRQHandler);               /* 55 */
STM32F4XX_WEAK_IRQ(DMA2_Stream0_IRQHandler);       /* 56 */
STM32F4XX_WEAK_IRQ(DMA2_Stream1_IRQHandler);       /* 57 */
STM32F4XX_WEAK_IRQ(DMA2_Stream2_IRQHandler);       /* 58 */
STM32F4XX_WEAK_IRQ(DMA2_Stream3_IRQHandler);       /* 59 */
STM32F4XX_WEAK_IRQ(DMA2_Stream4_IRQHandler);       /* 60 */
STM32F4XX_WEAK_IRQ(ETH_IRQHandler);                /* 61 */
STM32F4XX_WEAK_IRQ(ETH_WKUP_IRQHandler);           /* 62 */
STM32F4XX_WEAK_IRQ(CAN2_TX_IRQHandler);            /* 63 */
STM32F4XX_WEAK_IRQ(CAN2_RX0_IRQHandler);           /* 64 */
STM32F4XX_WEAK_IRQ(CAN2_RX1_IRQHandler);           /* 65 */
STM32F4XX_WEAK_IRQ(CAN2_SCE_IRQHandler);           /* 66 */
STM32F4XX_WEAK_IRQ(OTG_FS_IRQHandler);             /* 67 */
STM32F4XX_WEAK_IRQ(DMA2_Stream5_IRQHandler);       /* 68 */
STM32F4XX_WEAK_IRQ(DMA2_Stream6_IRQHandler);       /* 69 */
STM32F4XX_WEAK_IRQ(DMA2_Stream7_IRQHandler);       /* 70 */
STM32F4XX_WEAK_IRQ(USART6_IRQHandler);             /* 71 */
STM32F4XX_WEAK_IRQ(I2C3_EV_IRQHandler);            /* 72 */
STM32F4XX_WEAK_IRQ(I2C3_ER_IRQHandler);            /* 73 */
STM32F4XX_WEAK_IRQ(OTG_HS_EP1_OUT_IRQHandler);     /* 74 */
STM32F4XX_WEAK_IRQ(OTG_HS_EP1_IN_IRQHandler);      /* 75 */
STM32F4XX_WEAK_IRQ(OTG_HS_WKUP_IRQHandler);        /* 76 */
STM32F4XX_WEAK_IRQ(OTG_HS_IRQHandler);             /* 77 */
STM32F4XX_WEAK_IRQ(DCMI_IRQHandler);               /* 78 */
STM32F4XX_WEAK_IRQ(CRYP_IRQHandler);               /* 79  STM32F439 seul */
STM32F4XX_WEAK_IRQ(HASH_RNG_IRQHandler);           /* 80 */
STM32F4XX_WEAK_IRQ(FPU_IRQHandler);                /* 81 */
STM32F4XX_WEAK_IRQ(UART7_IRQHandler);              /* 82 */
STM32F4XX_WEAK_IRQ(UART8_IRQHandler);              /* 83 */
STM32F4XX_WEAK_IRQ(SPI4_IRQHandler);               /* 84 */
STM32F4XX_WEAK_IRQ(SPI5_IRQHandler);               /* 85 */
STM32F4XX_WEAK_IRQ(SPI6_IRQHandler);               /* 86 */
STM32F4XX_WEAK_IRQ(SAI1_IRQHandler);               /* 87 */
STM32F4XX_WEAK_IRQ(LTDC_IRQHandler);               /* 88 */
STM32F4XX_WEAK_IRQ(LTDC_ER_IRQHandler);            /* 89 */
STM32F4XX_WEAK_IRQ(DMA2D_IRQHandler);              /* 90 */

/* IRQ 0 à 63 : relais vers les noms CMSIS (table générique, startup_armv7m.c) */
void IRQ0_Handler(void) { WWDG_IRQHandler(); }
void IRQ1_Handler(void) { PVD_IRQHandler(); }
void IRQ2_Handler(void) { TAMP_STAMP_IRQHandler(); }
void IRQ3_Handler(void) { RTC_WKUP_IRQHandler(); }
void IRQ4_Handler(void) { FLASH_IRQHandler(); }
void IRQ5_Handler(void) { RCC_IRQHandler(); }
void IRQ6_Handler(void) { EXTI0_IRQHandler(); }
void IRQ7_Handler(void) { EXTI1_IRQHandler(); }
void IRQ8_Handler(void) { EXTI2_IRQHandler(); }
void IRQ9_Handler(void) { EXTI3_IRQHandler(); }
void IRQ10_Handler(void) { EXTI4_IRQHandler(); }
void IRQ11_Handler(void) { DMA1_Stream0_IRQHandler(); }
void IRQ12_Handler(void) { DMA1_Stream1_IRQHandler(); }
void IRQ13_Handler(void) { DMA1_Stream2_IRQHandler(); }
void IRQ14_Handler(void) { DMA1_Stream3_IRQHandler(); }
void IRQ15_Handler(void) { DMA1_Stream4_IRQHandler(); }
void IRQ16_Handler(void) { DMA1_Stream5_IRQHandler(); }
void IRQ17_Handler(void) { DMA1_Stream6_IRQHandler(); }
void IRQ18_Handler(void) { ADC_IRQHandler(); }
void IRQ19_Handler(void) { CAN1_TX_IRQHandler(); }
void IRQ20_Handler(void) { CAN1_RX0_IRQHandler(); }
void IRQ21_Handler(void) { CAN1_RX1_IRQHandler(); }
void IRQ22_Handler(void) { CAN1_SCE_IRQHandler(); }
void IRQ23_Handler(void) { EXTI9_5_IRQHandler(); }
void IRQ24_Handler(void) { TIM1_BRK_TIM9_IRQHandler(); }
void IRQ25_Handler(void) { TIM1_UP_TIM10_IRQHandler(); }
void IRQ26_Handler(void) { TIM1_TRG_COM_TIM11_IRQHandler(); }
void IRQ27_Handler(void) { TIM1_CC_IRQHandler(); }
void IRQ28_Handler(void) { TIM2_IRQHandler(); }
void IRQ29_Handler(void) { TIM3_IRQHandler(); }
void IRQ30_Handler(void) { TIM4_IRQHandler(); }
void IRQ31_Handler(void) { I2C1_EV_IRQHandler(); }
void IRQ32_Handler(void) { I2C1_ER_IRQHandler(); }
void IRQ33_Handler(void) { I2C2_EV_IRQHandler(); }
void IRQ34_Handler(void) { I2C2_ER_IRQHandler(); }
void IRQ35_Handler(void) { SPI1_IRQHandler(); }
void IRQ36_Handler(void) { SPI2_IRQHandler(); }
void IRQ37_Handler(void) { USART1_IRQHandler(); }
void IRQ38_Handler(void) { USART2_IRQHandler(); }
void IRQ39_Handler(void) { USART3_IRQHandler(); }
void IRQ40_Handler(void) { EXTI15_10_IRQHandler(); }
void IRQ41_Handler(void) { RTC_Alarm_IRQHandler(); }
void IRQ42_Handler(void) { OTG_FS_WKUP_IRQHandler(); }
void IRQ43_Handler(void) { TIM8_BRK_TIM12_IRQHandler(); }
void IRQ44_Handler(void) { TIM8_UP_TIM13_IRQHandler(); }
void IRQ45_Handler(void) { TIM8_TRG_COM_TIM14_IRQHandler(); }
void IRQ46_Handler(void) { TIM8_CC_IRQHandler(); }
void IRQ47_Handler(void) { DMA1_Stream7_IRQHandler(); }
void IRQ48_Handler(void) { FMC_IRQHandler(); }
void IRQ49_Handler(void) { SDIO_IRQHandler(); }
void IRQ50_Handler(void) { TIM5_IRQHandler(); }
void IRQ51_Handler(void) { SPI3_IRQHandler(); }
void IRQ52_Handler(void) { UART4_IRQHandler(); }
void IRQ53_Handler(void) { UART5_IRQHandler(); }
void IRQ54_Handler(void) { TIM6_DAC_IRQHandler(); }
void IRQ55_Handler(void) { TIM7_IRQHandler(); }
void IRQ56_Handler(void) { DMA2_Stream0_IRQHandler(); }
void IRQ57_Handler(void) { DMA2_Stream1_IRQHandler(); }
void IRQ58_Handler(void) { DMA2_Stream2_IRQHandler(); }
void IRQ59_Handler(void) { DMA2_Stream3_IRQHandler(); }
void IRQ60_Handler(void) { DMA2_Stream4_IRQHandler(); }
void IRQ61_Handler(void) { ETH_IRQHandler(); }
void IRQ62_Handler(void) { ETH_WKUP_IRQHandler(); }
void IRQ63_Handler(void) { CAN2_TX_IRQHandler(); }

#define STM32F4XX_IRQ_FIRST_EXT  64
#define STM32F4XX_IRQ_COUNT      91

typedef void (*stm32f4xx_vector_t)(void);

/* IRQ 64 à 90 */
__attribute__((section(".isr_vector_ext"), used))
const stm32f4xx_vector_t stm32f4xx_vector_table_ext[STM32F4XX_IRQ_COUNT - STM32F4XX_IRQ_FIRST_EXT] = {
   CAN2_RX0_IRQHandler,           /* 64 */
   CAN2_RX1_IRQHandler,           /* 65 */
   CAN2_SCE_IRQHandler,           /* 66 */
   OTG_FS_IRQHandler,             /* 67 */
   DMA2_Stream5_IRQHandler,       /* 68 */
   DMA2_Stream6_IRQHandler,       /* 69 */
   DMA2_Stream7_IRQHandler,       /* 70 */
   USART6_IRQHandler,             /* 71 */
   I2C3_EV_IRQHandler,            /* 72 */
   I2C3_ER_IRQHandler,            /* 73 */
   OTG_HS_EP1_OUT_IRQHandler,     /* 74 */
   OTG_HS_EP1_IN_IRQHandler,      /* 75 */
   OTG_HS_WKUP_IRQHandler,        /* 76 */
   OTG_HS_IRQHandler,             /* 77 */
   DCMI_IRQHandler,               /* 78 */
   CRYP_IRQHandler,               /* 79 */
   HASH_RNG_IRQHandler,           /* 80 */
   FPU_IRQHandler,                /* 81 */
   UART7_IRQHandler,              /* 82 */
   UART8_IRQHandler,              /* 83 */
   SPI4_IRQHandler,               /* 84 */
   SPI5_IRQHandler,               /* 85 */
   SPI6_IRQHandler,               /* 86 */
   SAI1_IRQHandler,               /* 87 */
   LTDC_IRQHandler,               /* 88 */
   LTDC_ER_IRQHandler,            /* 89 */
   DMA2D_IRQHandler,              /* 90 */
};
