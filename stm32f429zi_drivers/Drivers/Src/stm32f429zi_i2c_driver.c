/*
 * stm32f429zi_i2c_driver.c
 *
 *  Created on: 13-Jul-2022
 *      Author: Sanjay
 */

#include "STM32F429ZI.h"
#include "stm32f429zi_I2C_driver.h"
static void 	I2C_ExecuteAddressPhase(I2C_Reg_Def_t	*pI2Cx, uint8_t SlaveAddr);
static void I2C_GenerateStartCondition(I2C_Reg_Def_t	*pI2Cx);
static void I2C_ClearADDRFlag(I2C_Reg_Def_t	*pI2Cx);
static void I2C_GenerateStopCondition(I2C_Reg_Def_t	*pI2Cx);
uint16_t AHB_PreScalar[8]={2,4,8,16,64,128,256,512};
uint16_t APB1_PreScalar[4]={2,4,8,16};
/***********************************************************************************************
 * @Function:							-RCC_GetPCLK1Value
 * @Description:						-returns the clock frequency for the peripheral
 * @Parameter[]						-None
 * @Parameter[]						-None
 * @Parameter[]
 * @return								-CLk Value
 * @special note:						-Refer RCC register
 */

uint32_t RCC_GetPCLK1Value(void){
		uint32_t pclk1,SystemClk;
		uint8_t clksrc,temp,ahbp,apb1p;
		clksrc=(RCC->CFGR>>2)	&	 0x3;
		if(clksrc==0){
			SystemClk=16000000;
			}else if(clksrc==2){
			SystemClk=8000000;
			}else if(clksrc==3){
			//SystemClk=RCC_getPLLOutputClk(); //get output from pll source.
			//This function is not implemented
			}
		temp=((RCC->CFGR>>4)	&	0xF);
		if(temp<8){
			ahbp=1;
		}else{
			ahbp=AHB_PreScalar[temp-8];
		}

		temp=((RCC->CFGR>>10)	&	0x7);
			if(temp<4){
				apb1p=1;
			}else{
				apb1p=APB1_PreScalar[temp-4];
			}

		pclk1=(SystemClk/ahbp)/apb1p;

		return pclk1;
}

/***********************************************************************************************
 * @Function:							-I2C_GetFlagStatus
 * @Description:						-Gives status of the reqested flag
 * @Parameter[pI2Cx]				-Holds base address of the I2C chosen
 * @Parameter[FlagName]		-Flag which needs to be checked
 * @Parameter[]
 * @return								-Flag SET or RESET
 * @special note:						-None
 */

uint8_t I2C_GetFlagStatus(I2C_Reg_Def_t	*pI2Cx, uint32_t FlagName){
	if(pI2Cx->CR1	&	FlagName){
		return FLAG_SET;
	}
	return FLAG_RESET;
}
/***********************************************************************************************
 * @Function:							-I2C_PeriClockControl
 * @Description:						-This function will enable/disable the peripheral clock for the Selected I2C
 * @Parameter[pI2Cx]			-Holds base address of the I2C chosen
 * @Parameter[EnorDi]			-Holds the selection(Enable/Disable) for the I2C. //these are macros
 * @Parameter[]
 * @return								-None
 * @special note:						-None
 */

void I2C_PeriClockControl(I2C_Reg_Def_t  *pI2Cx, uint8_t EnorDi){
	if(EnorDi==ENABLE){
				if(pI2Cx==I2C1){
					I2C1_CLK_EN();
				}else if(pI2Cx==I2C2){
					I2C2_CLK_EN();
				}else if(pI2Cx==I2C3){
					I2C3_CLK_EN();
				}
		}else
		{
			if(pI2Cx==I2C1){
				}else if(pI2Cx==I2C1){
					I2C1_CLK_DI();
				}else if(pI2Cx==I2C2){
					I2C2_CLK_DI();
				}else if(pI2Cx==I2C3){
					I2C3_CLK_DI();
				}
		}
}
/***********************************************************************************************
 * @Function:							-I2C_Init
 * @Description:						-This function will initilize the I2C with the parameter defined in I2C Handle structure
 * @Parameter[pI2CHandle]-Holds settings
 * @return								-None
 * @special note:						-Needs to be called only after setting up proper handle structure
 */
void I2C_Init(I2C_Handle_t *pI2CHandle){
	/* General steps for I2c config
	 * 1.configure the mode(Standard or Fast)
	 * 2.Configure the Speed of Serial clock(SCL) Note:High Speed will limit the bus length
	 * 3.Configure the device address(in slave mode)
	 * 4.Enable Acking
	 * 5.Configure the rise time for I2C pins
	 *Note! I2C peripheral should be disabled during configuring phase
	 */

	//enable clock
	//I2C_PeriClockControl(pI2CHandle->pI2Cx, ENABLE);
	uint32_t tempreg=0;

	//ack control
	tempreg |=pI2CHandle->I2C_Config.I2C_ACKControl<<I2C_CR1_ACK;
	pI2CHandle->pI2Cx->CR1=tempreg;

	//configure FREQENCY in CR2
	tempreg=0;
	tempreg |=RCC_GetPCLK1Value()/1000000U;
	pI2CHandle->pI2Cx->CR2	=(tempreg	&	0x3F);

	//Store the device address(when working in slave mode)
	tempreg=0;
	tempreg |=pI2CHandle->I2C_Config.I2CDeviceAddress<<I2C_OAR1_ADD1;
	tempreg	|=(1<<14);
	pI2CHandle->pI2Cx->OAR1=tempreg;

	//CCR Calculation
	uint16_t ccr_value=0;
	tempreg=0;
	if(pI2CHandle->I2C_Config.I2C_SCLSpeed<=	I2C_SCL_SPEED_SM){
		//Standard mode speed
		ccr_value=(RCC_GetPCLK1Value()/(2*pI2CHandle->I2C_Config.I2C_SCLSpeed));
		tempreg	|=(ccr_value		&	0xFFF);
	}else {
		//Fast mode
		tempreg	|=(1<<15);
		tempreg	|=(pI2CHandle->I2C_Config.I2C_FMDutyCycle<<14);
		if(pI2CHandle->I2C_Config.I2C_FMDutyCycle==I2C_FM_DUTY_2){
			ccr_value=(RCC_GetPCLK1Value()/(3*pI2CHandle->I2C_Config.I2C_SCLSpeed)); //Calculated according to duty cycle formula
		}else{
			ccr_value=(RCC_GetPCLK1Value()/(25*pI2CHandle->I2C_Config.I2C_SCLSpeed));//Calculated according to duty cycle formula
		}
		tempreg	|=(ccr_value		&	0xFFF);
	}
	pI2CHandle->pI2Cx->CCR = tempreg;
}
/***********************************************************************************************
 * @Function:							-I2C_DeInit
 * @Description:						-This function will reset the selected  I2C to default values
 * @Parameter[pI2Cx]					-Holds base address of the I2C
 * @return								-None
 * @special note:						-None
 */

void I2C_DeInit(I2C_Reg_Def_t  	*pI2Cx){
					if(pI2Cx==I2C1){
						I2C1_REG_RESET();
					}else if(pI2Cx==I2C2){
						I2C2_REG_RESET();
					}else if(pI2Cx==I2C3){
						I2C3_REG_RESET();
					}
}


/***********************************************************************************************
 * @Function:							-I2C_MasterSendData
 * @Description:						-This function will configure the interrupt on the selected I2C(This Function will configure the NVIC registers)
 * @Parameter[pI2CHandle]				-Holds settings
 * @Parameter[pTxBuffer]					-Holds data to be transmitted
 * @Parameter[Len]					-Holds length of data to be transmitted
 * @Parameter[SlaveAddr]					-Holds slave address to which data is transferred
 * @return								-None
 * @special note:						-None
 */
void I2C_MasterSendData(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint32_t Len, uint8_t SlaveAddr){
/***********Some General steps in sending Data to slave
 * 1.Generate Start Condition
 * 2.Confirm that start generation is completed by checking the  SB: Start bit (Master mode) flag in SR1
 * Note:	Until SB is cleared SCL will be stretched(pulled to low)
 * 3.Send the address of the slave with r/nw bit set to w(0) (total 8 bits)
 * 4.Confirm that the address phase is completed by checking ADDR(ADDR: Address sent (master mode)/matched (slave mode)) flag in SR1
 * 5.Clear the ADDR flag according to its software sequence
 * Note: Until ADDR is cleared SCL will be stretched to low
 * 6.Send the data until Len becomes 0
 * 7.When Len becomes zero wait for TXE=1(Data register empty (transmitters))  and BTF=1( Byte transfer finished)
 * before generating STOP Condition
 * (TXE=BTF=1 means Shift Reg and Data Reg are empty and next transmission should begin)
 * note: when BTF=1 SCL will be stretched
 * 8.Generate Stop condition
 * Note: Generation of stop automatically clears BTF
 */

	//Generate Start Condition
	I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

	//Confirm Completion of Start generation
	while(!	I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB)); //Clearing SB by reading it

	//Send Address with R/W bit
	I2C_ExecuteAddressPhase(pI2CHandle->pI2Cx,SlaveAddr);

	//Confirm if address phase is completed
	I2C_ClearADDRFlag(pI2CHandle->pI2Cx);

	//Send Data until Len becomes zero
	while(Len>0)
	{
		while(!	I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TxE)); //wait till TXE is set
		pI2CHandle->pI2Cx->DR=	*pTxBuffer;
		pTxBuffer++;
		Len--;

	}
	//when Len Becomes 0 wait before TXE=BTF=1 before generate stop condition
	while(!	I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TxE));
	while(!	I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_BTF));

	//Generate Stop Condition
	I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
}

/***********************************************************************************************
 * @Function:							-I2C_IRQConfig
 * @Description:						-This function will configure the interrupt on the selected I2C(This Function will configure the NVIC registers)
 * @Parameter[IRQNumber]				-Holds the interrupt number(IRQ number)
 * @Parameter[EnorDi]					-Holds the selection(Enable/Disable) for the I2C. //these are macros
 * @return								-None
 * @special note:						-None
 */
void I2C_IRQConfig(uint8_t IRQNumber, uint8_t EnorDi){
	if(EnorDi==ENABLE){
				if(IRQNumber<=31){
					//Set ISER0
					*NVIC_ISER0	|=(1<<IRQNumber);
					}else if(IRQNumber>31	&&	IRQNumber<64){
						//Set ISER1
						*NVIC_ISER1	|=(1<<(IRQNumber%32));
					}else if(IRQNumber>=31	&&	IRQNumber<96){
						//Set ISER2
						*NVIC_ISER2	|=(1<<(IRQNumber%64));
					}
					else if(IRQNumber>=96	&&	IRQNumber<101){
						//Set ISER3
						*NVIC_ISER3	|=(1<<(IRQNumber%96));
					}
		}else{
				if(IRQNumber<=31){
					//Set ICER0
						*NVIC_ICER0	|=(1<<IRQNumber);
					}else if(IRQNumber>31	&&	IRQNumber<64){
						//Set ICER1
							*NVIC_ICER1	|=(1<<(IRQNumber%32));
					}else if(IRQNumber>=64	&&	IRQNumber<96){
						//Set ICER2
							*NVIC_ICER2	|=(1<<(IRQNumber%64));

					}else if(IRQNumber>=96	&&	IRQNumber<101){
						//Set ICER3
							*NVIC_ICER3	|=(1<<(IRQNumber%96));
					}
		}
}
/***********************************************************************************************
 * @Function:							-I2C_IRQPrority_Config
 * @Description:						-This function is used to set the priority of the interrupt
 *@Parameter[IRQNumber]					-Holds the interrupt number(IRQ number)
 * @Parameter[IRQPriority]				-Holds the interrupt priority for the selected I2C
 * @return								-None
 * @special note:						-
 */
void I2C_IRQPrority_Config(uint8_t IRQNumber, 	uint32_t IRQPriority){
	//Finding IPR register
		uint8_t iprx=IRQNumber/4;
		uint8_t iprx_section=IRQNumber%4;
		uint8_t shift_amount=	((8*iprx_section)+(8-NO_PR_BITS_IMPLEMENTED));
		*(NVIC_PR_BASE_ADD+(iprx))	|=	(IRQPriority<<shift_amount);
}



/***********************************************************************************************
 * @Function:							-I2C_PeripheralControl
 * @Description:						-This function will enable the I2C peripheral(sets spe bit)
 *
 * @Parameter[pI2Cx]					-base address of I2C peripheral
 * @return								-None
 * @special note:						-need to be called at last after calling the init function
 */
void I2C_PeripheralControl(I2C_Reg_Def_t *pI2Cx, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
	{
		pI2Cx->CR1 |=  (1 << I2C_CR1_PE);
	}else
	{
		pI2Cx->CR1 &=  ~(1 << I2C_CR1_PE);
	}

}


__attribute__((weak)) void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle,uint8_t AppEv)
{

	//This is a weak implementation . the user application may override this function.
}

/*
 * Some helper functions
 */

static void I2C_GenerateStartCondition(I2C_Reg_Def_t	*pI2Cx){
	pI2Cx->CR1		|=	(1<<I2C_CR1_START);
}

static void I2C_GenerateStopCondition(I2C_Reg_Def_t	*pI2Cx){
	/*
	 * STOP: Stop generation
	The bit is set and cleared by software, cleared by hardware when a Stop condition is
	detected, set by hardware when a timeout error is detected.
	 */
	pI2Cx->CR1		|=	(1<<I2C_CR1_STOP);
}

static void 	I2C_ExecuteAddressPhase(I2C_Reg_Def_t	*pI2Cx, uint8_t SlaveAddr){
	SlaveAddr=SlaveAddr<<1; //Makes space for R/W bit + 7 bit address
	SlaveAddr	&=	~(1); 	//clear the last bit(last bit 0=Write)
	pI2Cx->DR=SlaveAddr;
}

static void I2C_ClearADDRFlag(I2C_Reg_Def_t	*pI2Cx){
	/*
	 * Note: ADDR Flag is cleared by software reading SR1 register followed reading SR2, or by hardware
when PE=0.
	 */
uint32_t dummyRead=pI2Cx->SR1;
 dummyRead=pI2Cx->CR2;
(void)dummyRead; //To overcome unused variable warning
}

