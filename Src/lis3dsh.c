#include "lis3dsh.h"
//------------------------------------
extern SPI_HandleTypeDef hspi1;
extern int16_t acc_buffer[];
uint8_t buf2[8]={0};
char str1[30]={0};
//--------------------------------------
uint8_t SPIx_WriteRead(uint8_t Byte)
{
	uint8_t receivedbyte = 0;
	HAL_SPI_TransmitReceive_IT(&hspi1,(uint8_t*) &Byte,(uint8_t*) &receivedbyte, 1);
	while (hspi1.TxXferCount || hspi1.RxXferCount);
	return receivedbyte;
}
//--------------------------------------
void Accel_IO_Read(uint8_t *pBuffer, uint8_t ReadAddr, uint16_t NumByteToRead)
{
	if(NumByteToRead>0x01)
	{
		ReadAddr |= (uint8_t) (READWRITE_CMD | MULTIPLEBYTE_CMD);
	}
	else
	{
		ReadAddr |= (uint8_t)READWRITE_CMD;
	}
	CSN_ON;
	SPIx_WriteRead(ReadAddr);
	while(NumByteToRead>0x00)
	{
		*pBuffer=SPIx_WriteRead(DUMMY_BYTE);
		NumByteToRead--;
		pBuffer++;
	}
	CSN_OFF;
}
//--------------------------------------
void Accel_IO_Write(uint8_t *pBuffer, uint8_t WriteAddr, uint16_t NumByteToWrite)
{
	CSN_OFF;
	if(NumByteToWrite>0x01)
	{
		WriteAddr |= (uint8_t) MULTIPLEBYTE_CMD;
	}
	CSN_ON;
	SPIx_WriteRead(WriteAddr);
	while(NumByteToWrite>=0x01)
	{
		SPIx_WriteRead(*pBuffer);
		NumByteToWrite--;
		pBuffer++;
	}
	CSN_OFF;
}
//--------------------------------------
uint8_t Accel_ReadID(void)
{  
  uint8_t ctrl = 0;
	Accel_IO_Read(&ctrl,LIS3DSH_WHO_AM_I_ADDR,1);
	return ctrl;
}
//--------------------------------------
void AccInit(uint16_t InitStruct)
{
  uint8_t ctrl = 0;
	ctrl=(uint8_t)(InitStruct);
	Accel_IO_Write(&ctrl, LIS3DSH_CTRL_REG4_ADDR,1);
	ctrl=(uint8_t)(InitStruct>>8);
	Accel_IO_Write(&ctrl, LIS3DSH_CTRL_REG5_ADDR,1);
}
//--------------------------------------
void Accel_GetXYZ(int16_t* pData)
{
	int8_t buffer[6];
	Accel_IO_Read((uint8_t*)&buffer[0], LIS3DSH_OUT_X_L_ADDR,1);
	Accel_IO_Read((uint8_t*)&buffer[1], LIS3DSH_OUT_X_H_ADDR,1);
	Accel_IO_Read((uint8_t*)&buffer[2], LIS3DSH_OUT_Y_L_ADDR,1);
	Accel_IO_Read((uint8_t*)&buffer[3], LIS3DSH_OUT_Y_H_ADDR,1);
	Accel_IO_Read((uint8_t*)&buffer[4], LIS3DSH_OUT_Z_L_ADDR,1);
	Accel_IO_Read((uint8_t*)&buffer[5], LIS3DSH_OUT_Z_H_ADDR,1);
	
	#ifndef PURE_DATA
	uint8_t ctrl,i = 0x00;
	float sensitivity = LIS3DSH_SENSITIVITY_0_06G;
	float value_f32 = 0;
	Accel_IO_Read(&ctrl, LIS3DSH_CTRL_REG5_ADDR,1);
	switch(ctrl & LIS3DSH__FULLSCALE_SELECTION)
	{
		case LIS3DSH_FULLSCALE_2:
			sensitivity=LIS3DSH_SENSITIVITY_0_06G;
			break;
		case LIS3DSH_FULLSCALE_4:
			sensitivity=LIS3DSH_SENSITIVITY_0_12G;
			break;
		case LIS3DSH_FULLSCALE_6:
			sensitivity=LIS3DSH_SENSITIVITY_0_18G;
			break;
		case LIS3DSH_FULLSCALE_8:
			sensitivity=LIS3DSH_SENSITIVITY_0_24G;
			break;
		case LIS3DSH_FULLSCALE_16:
			sensitivity=LIS3DSH_SENSITIVITY_0_73G;
			break;
		default:
			break;
	}
	#endif
	for(int i = 0; i < 3; i++)
	{
		#ifdef PURE_DATA
		pData[i] = (buffer[2*i+1] << 8) + buffer[2*i];
		#else
		value_f32 = ((buffer[2*i+1] << 8) + buffer[2*i]); //* sensitivity;
		pData[i] = (int16_t) value_f32;
		#endif
	}
}
//--------------------------------------
void Accel_ReadAcc(void)
{
	Accel_GetXYZ(acc_buffer);
}

//--------------------------------------
void Accel_Ini(void)
{
	uint16_t ctrl = 0x0000;
	HAL_Delay(1000);
	ctrl = (uint16_t) (LIS3DSH_DATARATE_100 | LIS3DSH_XYZ_ENABLE);
	ctrl |= ((uint16_t) (LIS3DSH_SERIALINTERFACE_4WIRE|\
											LIS3DSH_SELFTEST_NORMAL|\
											LIS3DSH_FULLSCALE_2|\
											LIS3DSH_FILTER_BW_50))<<8;
	AccInit(ctrl);
}
//--------------------------------------
