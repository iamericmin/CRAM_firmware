#include <stm32f4xx.h>
#include <stm32f4xx_hal.h>
#define ARM_MATH_CM4

void GPIO_Init(void);
void CAN1_Init(void);
void CAN1_Tx(uint8_t tr);
uint8_t CAN1_Rx(void);
void TIM4_ms_Delay(uint32_t delay);
uint8_t k = 0;
uint8_t rec = 0;

void GPIO_INIT() {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

  GPIOB->MODER |= (GPIO_MODER_MODER8_1 | GPIO_MODER_MODER9_1);

  GPIOB->AFR[1] |= (GPIO_AFRH_AFSEL8_3 | GPIO_AFRH_AFSEL8_0 |
                    GPIO_AFRH_AFSEL9_0 | GPIO_AFRH_AFSEL9_3);
}

void CAN1_INIT() {
  RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;

  CAN1->MCR |= CAN_MCR_INRQ;
  while(!(CAN1->MSR & CAN_MSR_INAK)){}

  CAN1->BTR |= CAN_BTR_LBKM;
  CAN1->BTR &= ~CAN_BTR_SJW;

  CAN1->BTR &= ~CAN_BTR_TS2;
  CAN1->BTR |= (CAN_BTR_TS2_1 | CAN_BTR_TS2_0);

  CAN1->BTR &= ~CAN_BTR_TS1;
  CAN1->BTR |= CAN_BTR_TS1_1;

  CAN1->BTR |= ((28-1) << 0);

  // Exit the Initialization mode for CAN1
  // Wait until the INAK bit is cleared by hardware
  CAN1->MCR &= ~CAN_MCR_INRQ;
  while (CAN1->MSR & CAN_MSR_INAK){}

  //Exit Sleep Mode
  CAN1->MCR &= ~ CAN_MCR_SLEEP;
  while (CAN1->MSR & CAN_MSR_SLAK){}

  /* 2. Setting up the Transmission----------------------------------------*/

  CAN1->sTxMailBox[0].TIR = 0;

  //Setting up the Std. ID
  CAN1->sTxMailBox[0].TIR = (0x245<<21);
  CAN1->sTxMailBox[0].TDHR = 0;

  // Setting Data Length to 1 Byte.
  CAN1->sTxMailBox[0].TDTR = 1;

  /* 3. Configuring the Filters--------------------------------------------*/

  //Enter Filter Initialization mode to configure the filters
  CAN1->FMR |= CAN_FMR_FINIT;

  // Configuring the Number of Filters Reserved for CAN1
  // and also the start bank for CAN2
  CAN1->FMR |= 14<<8;

  // Select the single 32-bit scale configuration
  CAN1->FS1R |= CAN_FS1R_FSC13;

  // Set the receive ID
  CAN1->sFilterRegister[13].FR1 = 0x245<<21;

  // Select Identifier List mode
  CAN1->FM1R |= CAN_FM1R_FBM13;

  //Activating filter 14
  CAN1->FA1R |= CAN_FA1R_FACT13;

  //Exit filter Initialization Mode
  CAN1->FMR &= ~CAN_FMR_FINIT;
}

void CAN1_Tx(uint8_t data) {
  CAN1->sTxMailBox[0].TDLR = data;
  CAN1->sTxMailBox[0].TIR |= 1;
}

uint8_t CAN1_Rx(){
    // Monitoring FIFO 0 message pending bits FMP0[1:0]
    while(!(CAN1->RF0R & 3)){}

    // Read the data from the FIFO 0 mailbox from Mailbox data low register
    uint8_t RxD = (CAN1->sFIFOMailBox[0].RDLR) & 0xFF;
    rec = RxD;

    // Releasing FIFO 0 output mailbox
    CAN1->RF0R |= 1<<5;

    return RxD;
}

int main(void){
  GPIO_Init();
  CAN1_Init();
  while(1){
      CAN1_Tx(k);
      rec= CAN1_Rx();
      k += 1;
      if (k>25)
          k = 0;
      HAL_Delay(1000);
  }
  return 0;
}
