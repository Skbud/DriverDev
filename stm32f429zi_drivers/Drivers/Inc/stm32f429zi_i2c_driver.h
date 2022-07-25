/*
 * stm32f429zi_i2c_driver.h
 *
 *  Created on: 13-Jul-2022
 *      Author: Sanjay
 */

#ifndef STM32F429ZI_I2C_DRIVER_H_
#define STM32F429ZI_I2C_DRIVER_H_

#include "STM32F429ZI.h"
#include "stm32f429zi_I2C_driver.h"
/**********SCL Speed*********/
#define I2C_SCL_SPEED_SM				100000			//standard speed
#define I2C_SCL_SPEED_FM2K			200000			//Fast Mode 2khz
#define I2C_SCL_SPEED_FM4K			400000			//Fast mode 4khz

/********I2C_ACKControl*****/
#define I2C_ACK_ENABLE					1
#define I2C_ACK_DISABLE					0

/*******I2C_FMDutyCycle***/
#define	I2C_FM_DUTY_2					0	//REFER CCR
#define	I2C_FM_DUTY_16_9				1

/******I2C Flags**********/
#define I2C_FLAG_SB							(1<<I2C_SR1_SB)
#define I2C_FLAG_ADDR					(1<<I2C_SR1_ADDR)
#define I2C_FLAG_BTF						(1<<I2C_SR1_BTF)
#define I2C_FLAG_ADD10					(1<<I2C_SR1_ADD10)
#define I2C_FLAG_STOPF					(1<<I2C_SR1_STOPF)
#define I2C_FLAG_RxNE						(1<<I2C_SR1_RxNE)
#define I2C_FLAG_TxE						(1<<I2C_SR1_TxE)
#define I2C_FLAG_BERR						(1<<I2C_SR1_BERR)
#define I2C_FLAG_ARLO					(1<<I2C_SR1_ARLO)
#define I2C_FLAG_AF							(1<<I2C_SR1_AF)
#define I2C_FLAG_OVR						(1<<I2C_SR1_OVR)
#define I2C_FLAG_PEC_ERR				(1<<I2C_SR1_PECERR)
#define I2C_FLAG_TIMEOUT				(1<<I2C_SR1_TIMEOUT)
#define I2C_FLAG_SMB_ALERT			(1<<I2C_SR1_SMBALERT)

#define I2C_FLAG_MSL							(1<<I2C_SR2_MSL)
#define I2C_FLAG_BUSY							(1<<I2C_SR2_BUSY)
#define I2C_FLAG_TRA							(1<<I2C_SR2_TRA)
#define I2C_FLAG_GEN_CALL				(1<<I2C_SR2_GENCALL)
#define I2C_FLAG_SMB_DEFAULT			(1<<I2C_SR2_SMBDEFAULT)
#define I2C_FLAG_SMB_HOST				(1<<I2C_SR2_SMBHOST)
#define I2C_FLAG_DUALF						(1<<I2C_SR2_DUALF)
#define I2C_FLAG_PEC							(1<<I2C_SR2_PEC)


typedef struct
{
	uint32_t	I2C_SCLSpeed;
	uint8_t		I2CDeviceAddress;	//address of the device when working as slave(USER NEED TO SET THIS)
	uint8_t		I2C_ACKControl;		//Acknowledge control
	uint16_t	I2C_FMDutyCycle;	//Duty cycle when working in Fast mode
}I2C_Config_t;

typedef struct
{	I2C_Reg_Def_t		*pI2Cx;
	I2C_Config_t		I2C_Config;
}I2C_Handle_t;

/****************************************************************************************************************************************************************
 * 																							SUPPORTED API																															*
 ****************************************************************************************************************************************************************/
void I2C_PeriClockControl(I2C_Reg_Def_t  *pI2Cx, uint8_t EnorDi);	//Clock Enable/Disable

void I2C_Init(I2C_Handle_t *pI2CHandle);							//Initialization

void I2C_DeInit(I2C_Reg_Def_t  	*pI2Cx);							//DeInitialization

void I2C_MasterSendData(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint32_t Len, uint8_t SlaveAddr);
void I2C_IRQConfig(uint8_t IRQNumber, uint8_t EnorDi);

void I2C_IRQPrority_Config(uint8_t IRQNumber, 	uint32_t IRQPriority);

uint8_t I2C_GetFlagStatus(I2C_Reg_Def_t	*pI2Cx, uint32_t FlagName);

void I2C_PeripheralControl(I2C_Reg_Def_t *pI2Cx, uint8_t EnOrDi);

void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle,uint8_t AppEv); //Application Call back
#endif /* STM32F429ZI_I2C_DRIVER_H_ */
