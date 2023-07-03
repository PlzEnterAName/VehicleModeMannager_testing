 /************************Copyright by ofilm 2020-2030*****************************
  * @file    M7_0_SOC.h 
  * @author  Application Team
  * @version V1.0.0
  * @date    30-5-2023
  * @brief   SOC manager
  *******************************************************************************/
#ifndef __M7_0_SOC_H_
#define __M7_0_SOC_H_

#include "M7_0_VMMPublic.h"

typedef enum 
{
    LMT_LEVEL0 = 0,
    LMT_LEVEL1,
    LMT_LEVEL2,
    LMT_LEVEL3
}M7_0_LIMIT_LEVEL;

typedef enum
{
    NO_WARNING = 0,
    WARNING_1,
    WARNING_RESERVED,
    WARNING_3,
}M7_0_WARNING_LEVEL;

typedef enum 
{
    RANGE_0 = 0,
    RANGE_1 = 1,
    RANGE_2 = 2,
    RANGE_INVALID = 3,
}M7_0_I_RANGE;


#define FACTOR_RANGE_0  0.0009765625
#define FACTOR_RANGE_1  0.0078125000
#define FACTOR_RANGE_2  0.0625

#define OFFSET_RANGE_0  -32
#define OFFSET_RANGE_1  -256
#define OFFSET_RANGE_2  -2048

#define MINIMUM_RANGE_0 31744
#define MAXIMUM_RANGE_0 33792
#define MINIMUM_RANGE_1 7168
#define MAXIMUM_RANGE_1 58368
#define MINIMUM_RANGE_2 8768
#define MAXIMUM_RANGE_2 56768


void M7_0_SOC_Manager_Init(void);
void M7_0_SOCModeManagerRunnable(void);

#endif
