/*
 * flash_if.c
 *
 *  Created on: 25 July 2023
 *      Author: Akash Virendra
 */
#include "flash_if.h"



/**
  * @brief  Unlocks Flash for write access
  * @param  None
  * @retval None
  */

#define FLASH_DOUBLEWORD_SIZE 8
void FLASH_If_Init(void)
{
  /* Unlock the Program memory */
  HAL_FLASH_Unlock();

  /* Clear all FLASH flags */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGSERR | FLASH_FLAG_WRPERR | FLASH_FLAG_OPTVERR);

}

/**
  * @brief  This function does an erase of all user flash area
  * @param  start: start of user flash area
  * @retval FLASHIF_OK : user flash area successfully erased
  *         FLASHIF_ERASEKO : error occurred
  */
uint32_t FLASH_If_Erase(uint32_t start)
{
  FLASH_EraseInitTypeDef desc;
  uint32_t result = FLASHIF_OK;
  uint32_t pageerror;


  /* Unlock the Flash to enable the flash control register access *************/
   FLASH_If_Init();

  desc.Page = (start - FLASH_BASE) / FLASH_PAGE_SIZE;
  desc.TypeErase = FLASH_TYPEERASE_PAGES;
  desc.Banks = FLASH_BANK_1;

/* NOTE: Following implementation expects the IAP code address to be < Application address */
  if (start < FLASH_END_ADDRESS )
  {
    desc.NbPages = (FLASH_END_ADDRESS - start) / FLASH_PAGE_SIZE; // 59 pages to be erased from the user application address till the end of the RAM
    if (HAL_FLASHEx_Erase(&desc, &pageerror) != HAL_OK)
    {
      result = FLASHIF_ERASEKO;
    }
  }
  else
	  result = FLASHIF_ERASEKO;

  HAL_FLASH_Lock();

  return result;
}

/* Public functions ---------------------------------------------------------*/
/**
  * @brief  This function writes a data buffer in flash (data are 32-bit aligned).
  * @note   After writing data buffer, the flash content is checked.
  * @param  destination: start address for target location
  * @param  p_source: pointer on buffer with data to write
  * @param  length: length of data buffer (unit is 32-bit word)
  * @retval uint32_t 0: Data successfully written to Flash memory
  *         1: Error occurred while writing data in Flash memory
  *         2: Written Data in flash memory is different from expected one
  */
uint32_t FLASH_If_Write(uint32_t destination, uint32_t *p_source, uint32_t length)
{
  uint32_t status = FLASHIF_OK;
  uint32_t i = 0;

  HAL_FLASH_Unlock();


  for (i = 0; (i < length/2) && (destination <= (USER_FLASH_END_ADDRESS-8)); i++)
   {
     /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
        be done by word */
     if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, destination, *((uint64_t *)(p_source+2*i))) == HAL_OK)
     {
      /* Check the written value */
       if (*(uint64_t*)destination != *(uint64_t *)(p_source+2*i))
       {
         /* Flash content doesn't match SRAM content */
         status = FLASHIF_WRITINGCTRL_ERROR;
         break;
       }
       /* Increment FLASH destination address */
       destination += 8;
     }
     else
     {
       /* Error occurred while writing data in Flash memory */
       status = FLASHIF_WRITING_ERROR;
       break;
     }
   }
  HAL_FLASH_Lock();

  return status;
}

/**
  * @brief  Returns the write protection status of application flash area.
  * @param  None
  * @retval If a sector in application area is write-protected returned value is a combination
            of the possible values : FLASHIF_PROTECTION_WRPENABLED, FLASHIF_PROTECTION_PCROPENABLED, ...
  *         If no sector is write-protected FLASHIF_PROTECTION_NONE is returned.
  */
uint32_t FLASH_If_GetWriteProtectionStatus(void)
{
  FLASH_OBProgramInitTypeDef config;
  uint32_t wrp1_status = 0, wrp2_status = 0, wrp_area_status = 0;
  uint32_t protected = FLASHIF_PROTECTION_NONE;

  /* Get the current configuration */
  config.WRPArea=OB_WRPAREA_ZONE_A;
  HAL_FLASHEx_OBGetConfig(&config);

  wrp_area_status = config.WRPArea;
  wrp1_status     =config.WRPStartOffset;
  wrp2_status     =config.WRPEndOffset;

  /* Final evaluation of status */
  if ((wrp_area_status != 0) && (wrp1_status < wrp2_status))
  {
    protected = FLASHIF_PROTECTION_WRPENABLED;
  }

  return protected;
}

/**
  * @brief  Configure the write protection status of user flash area.
  * @param  protectionstate : FLASHIF_WRP_DISABLE or FLASHIF_WRP_ENABLE the protection
  * @retval uint32_t FLASHIF_OK if change is applied.
  */
uint32_t FLASH_If_WriteProtectionConfig(uint32_t protectionstate)
{
  FLASH_OBProgramInitTypeDef config_new, config_old;
  HAL_StatusTypeDef result;

  	HAL_FLASH_Unlock();
	HAL_FLASH_OB_Unlock();

	 config_old.WRPArea=OB_WRPAREA_ZONE_A;
  	 HAL_FLASHEx_OBGetConfig( &config_old );

  if(protectionstate == FLASHIF_WRP_ENABLE)
  {
	  /* We want to modify only the Write protection */
	  config_new.OptionType = OPTIONBYTE_WRP;

	  /* No read protection, keep BOR and reset settings */
	  // config_new.RDPLevel = OB_RDP_LEVEL_0;
	   config_new.USERConfig = config_old.USERConfig;

	   //as the G0 here only contains one bank hence we just need to modify the WRP area 1/2(as it contains only one bank hence we can
	   //protect the areas from the application start address till the flash end address) with the
	   config_new.WRPArea=OB_WRPAREA_ZONE_A;

	   config_new.WRPStartOffset=START_0x08008000;
	   config_new.WRPEndOffset= END_OF_RAM;
  }
  else
  {
	  /* We want to modify only the Write protection */
	  config_new.OptionType = OPTIONBYTE_WRP;

	  /* No read protection, keep BOR and reset settings */
	   config_new.USERConfig = config_old.USERConfig;
	   config_new.WRPArea=OB_WRPAREA_ZONE_A;

	   config_new.WRPStartOffset=END_OF_RAM ;
	   config_new.WRPEndOffset=  START_OF_RAM;
  }

    result = HAL_FLASHEx_OBProgram(&config_new);

  return (result == HAL_OK ? FLASHIF_OK: FLASHIF_PROTECTION_ERRROR);
}

