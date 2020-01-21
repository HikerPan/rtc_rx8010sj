// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for the Epson RTC module RX-8010 SJ
 *
 * Copyright(C) Timesys Corporation 2015
 * Copyright(C) General Electric Company 2015
 */

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include <string.h>

#define DBG_ENABLE
#define DBG_SECTION_NAME "RX8010"
#define DBG_LEVEL DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>

#include "rtc_rx8010.h"
#include <time.h>


#ifdef PKG_USING_RTC_RX8010SJ


#define BIT(n) (1<<n)
#define BCD_TO_BIN(val)   ((((val) >> 4) * 10) +  ((val)&0x0f))
#define BIN_TO_BCD(val)   ((((val) / 10) << 4) + (val) % 10)
#define BCD_TO_ASCCII(val)    (((((val) >> 8) + 0x30) << 8) +  (((val) & 0xFF) + 0x30))

#define RX8010_ADDR    0x32
#define RX8010_WRITE_ADDR    0x64
#define RX8010_READ_ADDR    0x65

#define RX8010_SEC     0x10
#define RX8010_MIN     0x11
#define RX8010_HOUR    0x12
#define RX8010_WDAY    0x13
#define RX8010_MDAY    0x14
#define RX8010_MONTH   0x15
#define RX8010_YEAR    0x16
#define RX8010_RESV17  0x17
#define RX8010_ALMIN   0x18
#define RX8010_ALHOUR  0x19
#define RX8010_ALWDAY  0x1A
#define RX8010_TCOUNT0 0x1B
#define RX8010_TCOUNT1 0x1C
#define RX8010_EXT     0x1D
#define RX8010_FLAG    0x1E
#define RX8010_CTRL    0x1F
/* 0x20 to 0x2F are user registers */
#define RX8010_RESV30  0x30
#define RX8010_RESV31  0x31
#define RX8010_IRQ     0x32

#define RX8010_EXT_WADA  BIT(3)

#define RX8010_FLAG_VLF  BIT(1)
#define RX8010_FLAG_AF   BIT(3)
#define RX8010_FLAG_TF   BIT(4)
#define RX8010_FLAG_UF   BIT(5)

#define RX8010_CTRL_AIE  BIT(3)
#define RX8010_CTRL_UIE  BIT(5)
#define RX8010_CTRL_STOP BIT(6)
#define RX8010_CTRL_TEST BIT(7)

#define RX8010_ALARM_AE  BIT(7)

struct rt_i2c_bus_device *rx8010_i2c_bus;


//写rx8010单个寄存器
//reg:寄存器地址
//data:数据
//返回值: 0,正常 / -1,错误代码
rt_err_t rx8010_write_reg(rt_uint8_t reg, rt_uint8_t data)
{
    rt_uint8_t buf[2];

    buf[0] = reg;
    buf[1] = data;

	RT_ASSERT(rx8010_i2c_bus);
	if(RT_NULL == rx8010_i2c_bus)
	{
		LOG_E("rx8010 i2c bus not init");
		return -RT_ERROR;
	}
       
    if (rt_i2c_master_send(rx8010_i2c_bus, RX8010_ADDR, 0, buf ,2) == 2)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }
}

//读取寄存器数据
//reg:要读取的寄存器地址
//len:要读取的数据字节数
//buf:读取到的数据存储区
//返回值: 0,正常 / -1,错误代码
rt_err_t rx8010_read_reg(rt_uint8_t reg, rt_uint8_t len, rt_uint8_t *buf)
{   
	RT_ASSERT(rx8010_i2c_bus);
	if(RT_NULL == rx8010_i2c_bus)
	{
		LOG_E("rx8010 i2c bus not init");
		return -RT_ERROR;
	}

    if (rt_i2c_master_send(rx8010_i2c_bus, RX8010_ADDR, 0, &reg, 1) == 1)
    {
        if (rt_i2c_master_recv(rx8010_i2c_bus, RX8010_ADDR, 0, buf, len) == len)
        {
            return RT_EOK;
        }
        else
        {
            return -RT_ERROR;
        }
    }
    else
    {
        return -RT_ERROR;
    }

}

/**
 * @description: get time from rx8010
 * @param {type} 
 * @return: 
 */
rt_err_t rx8010_get_time(struct tm *dt)
{
	rt_uint8_t date[7];
	rt_uint8_t flagreg;
	rt_err_t err=RT_EOK;
	rt_uint8_t i = 0;
	rt_uint8_t wday = 0;
	rt_uint8_t testreg;


	err = rx8010_read_reg(RX8010_FLAG,1,&flagreg);
	if (err < 0)
		return err;

	if (flagreg & RX8010_FLAG_VLF) {
		LOG_W("Frequency stop detected");
		return -RT_ERROR;
	}

	/*read timer register*/
	err = rx8010_read_reg(RX8010_SEC,7,date);
	if (err < 0)
		return err ;

	LOG_D("Time Get RX8010_SEC 0x%x",date[RX8010_SEC - RX8010_SEC]);
	dt->tm_sec = BCD_TO_BIN(date[RX8010_SEC - RX8010_SEC] & 0x7f);

	LOG_D("Time Get RX8010_MIN 0x%x",date[RX8010_MIN - RX8010_SEC]);
	dt->tm_min = BCD_TO_BIN(date[RX8010_MIN - RX8010_SEC] & 0x7f);

	LOG_D("Time Get RX8010_HOUR 0x%x",date[RX8010_HOUR - RX8010_SEC]);
	dt->tm_hour = BCD_TO_BIN(date[RX8010_HOUR - RX8010_SEC] & 0x3f);

	LOG_D("Time Get RX8010_MDAY 0x%x",date[RX8010_MDAY - RX8010_SEC]);
	dt->tm_mday = BCD_TO_BIN(date[RX8010_MDAY - RX8010_SEC] & 0x3f);

	LOG_D("Time Get RX8010_MONTH 0x%x",date[RX8010_MONTH - RX8010_SEC]);
	dt->tm_mon = BCD_TO_BIN(date[RX8010_MONTH - RX8010_SEC] & 0x1f) - 1;

	LOG_D("Time Get RX8010_YEAR 0x%x",date[RX8010_YEAR - RX8010_SEC]);
	dt->tm_year = BCD_TO_BIN(date[RX8010_YEAR - RX8010_SEC]) + 100;

	LOG_D("Time Get RX8010_WDAY 0x%x",date[RX8010_WDAY - RX8010_SEC]);
	wday = (date[RX8010_WDAY - RX8010_SEC] & 0x7f);
	for(i=0;i<7;i++)
	{
		if(wday&(1<<i))
		{
			dt->tm_wday = i;
			break;
		}
	}

	return err;
}
/**
 * @description: set time to rx8010
 * @param {type} 
 * @return: 
 */
rt_err_t  rx8010_set_time(struct tm *dt)
{
	rt_uint8_t date[7];
	rt_uint8_t ctrl, flagreg;
	rt_err_t  ret = RT_EOK;
	rt_uint8_t testreg;


	if ((dt->tm_year < 100) || (dt->tm_year > 199))
		return -RT_ERROR;

	/* set STOP bit before changing clock/calendar */
	ret = rx8010_read_reg(RX8010_CTRL,1,&ctrl);
	if (ret < 0)
		return ret;
	ctrl |= RX8010_CTRL_STOP;
	ret = rx8010_write_reg(RX8010_CTRL,ctrl);
	if (ret < 0)
		return ret;

	/*set timer register*/
	date[RX8010_SEC - RX8010_SEC] = BIN_TO_BCD(dt->tm_sec);
	ret = rx8010_write_reg(RX8010_SEC,date[RX8010_SEC - RX8010_SEC]);
	if (ret < 0)
		return ret;

	date[RX8010_MIN - RX8010_SEC] = BIN_TO_BCD(dt->tm_min);
	ret = rx8010_write_reg(RX8010_MIN,date[RX8010_MIN - RX8010_SEC]);
	if (ret < 0)
		return ret;

	date[RX8010_HOUR - RX8010_SEC] = BIN_TO_BCD(dt->tm_hour);
	ret = rx8010_write_reg(RX8010_HOUR,date[RX8010_HOUR - RX8010_SEC]);
	if (ret < 0)
		return ret;

	date[RX8010_MDAY - RX8010_SEC] = BIN_TO_BCD(dt->tm_mday);
	ret = rx8010_write_reg(RX8010_MDAY,date[RX8010_MDAY - RX8010_SEC]);
	if (ret < 0)
		return ret;

	date[RX8010_MONTH - RX8010_SEC] = BIN_TO_BCD((dt->tm_mon + 1));
	ret = rx8010_write_reg(RX8010_MONTH,date[RX8010_MONTH - RX8010_SEC]);
	if (ret < 0)
		return ret;

	date[RX8010_YEAR - RX8010_SEC] = BIN_TO_BCD((dt->tm_year - 100));
	ret = rx8010_write_reg(RX8010_YEAR,date[RX8010_YEAR - RX8010_SEC]);
	if (ret < 0)
		return ret;

	date[RX8010_WDAY - RX8010_SEC] = BIN_TO_BCD((1 << dt->tm_wday));
	ret = rx8010_write_reg(RX8010_WDAY,date[RX8010_WDAY - RX8010_SEC]);
	if (ret < 0)
		return ret;

	/* clear STOP bit after changing clock/calendar */
	ret = rx8010_read_reg(RX8010_CTRL,1,&ctrl);
	if (ret < 0)
		return ret;

	ctrl = ctrl & ~RX8010_CTRL_STOP;
	ret = rx8010_write_reg(RX8010_CTRL,ctrl);
	if (ret < 0)
		return ret;

	ret = rx8010_read_reg(RX8010_CTRL,1,&flagreg);
	if (ret < 0)
		return ret;

	if (flagreg & RX8010_FLAG_VLF)
	{
		ret = rx8010_write_reg(RX8010_FLAG,(flagreg & ~RX8010_FLAG_VLF));	
			return ret;
	}

	return ret;
}

/**
 * @description: Init rx8010
 * @param {type} 
 * @return: 
 */
rt_err_t rx8010_init(char const *devicename)
{
	rt_err_t err = RT_EOK;
	rt_uint8_t flagreg,extreg;
	rt_uint8_t need_clear = 0;

	RT_ASSERT(devicename);

	/* get the rx8010 handle*/
	rx8010_i2c_bus = (struct rt_i2c_bus_device *) rt_i2c_bus_device_find(devicename);
	RT_ASSERT(rx8010_i2c_bus);
	if(RT_NULL == rx8010_i2c_bus)
	{
		LOG_E("RX8010 Init devicename %s not found!",devicename);
		return -RT_ERROR;
	}
	LOG_D("RX8010 Device usd %s attached",devicename);

	/* Initialize reserved registers as specified in datasheet */
	err = rx8010_write_reg(RX8010_RESV17, 0xD8);
	if (err < 0)
		return err;

	err = rx8010_write_reg(RX8010_RESV30, 0x00);
	if (err < 0)
		return err;

	err = rx8010_write_reg(RX8010_RESV31, 0x08);
	if (err < 0)
		return err;

	err = rx8010_write_reg(RX8010_IRQ, 0x00);
	if (err < 0)
		return err;
	
	err = rx8010_read_reg(RX8010_FLAG,1,&flagreg);
	if (err < 0)
		return err;
	

	if (flagreg & RX8010_FLAG_VLF)
	{
		need_clear = 1;
		LOG_W("Frequency stop was detected flag 0x%x",flagreg);
	}

	if (flagreg & RX8010_FLAG_AF) {
		LOG_W("Alarm was detected");
		need_clear = 1;
	}

	if (flagreg & RX8010_FLAG_TF)
		need_clear = 1;

	if (flagreg & RX8010_FLAG_UF)
		need_clear = 1;

	if (need_clear) 
	{
		flagreg &= ~(RX8010_FLAG_AF | RX8010_FLAG_TF | RX8010_FLAG_UF|RX8010_FLAG_VLF);
		LOG_D("Write RX8010_FLAG");
		err = rx8010_write_reg(RX8010_FLAG, flagreg);
		if (err < 0)
			return err;
	}

	err = rx8010_read_reg(RX8010_EXT,1,&extreg);
	if (err < 0)
		return err;

	if((extreg&(BIT(4)))!=0)
	{
		extreg &= ~BIT(4);
		err = rx8010_write_reg(RX8010_EXT, extreg);
		if (err < 0)
			return err;

		err = rx8010_read_reg(RX8010_EXT,1,&extreg);
		if (err < 0)
			return err;
	}
	

	return err;
}


/**
 * @description: Read rx8010 alarm setting
 * @param {type} 
 * @return: 
 */
rt_err_t rx8010_read_alarm(rx8010_alarm_time_t *t)
{
	rt_uint8_t alarmvals[3];
	rt_uint8_t flagreg;

	/*read alarm register from ALMIN ALHOUR ALDAY*/
	if(RT_EOK != rx8010_read_reg(RX8010_ALMIN, 3, alarmvals))
	{	
		return -RT_ERROR;
	}

	if (!(alarmvals[0] & RX8010_ALARM_AE))
		t->Min = BCD_TO_BIN(alarmvals[0] & 0x7f);
	else
		t->Min = 0;

	if (!(alarmvals[1] & RX8010_ALARM_AE))	
		t->Hour = BCD_TO_BIN(alarmvals[1] & 0x3f);
	else
		t->Hour = 0;

	if (!(alarmvals[2] & RX8010_ALARM_AE))
		t->Dmon = BCD_TO_BIN(alarmvals[2] & 0x7f);
	else
		t->Dmon = 0;

	return RT_EOK;
}

/**
 * @description: set rx8010 alarm param
 * @param {type} 
 * @return: 
 */
rt_err_t rx8010_set_alarm(rx8010_alarm_time_t *t)
{
	rt_uint8_t alarmvals[3];
	rt_uint8_t extreg, flagreg,ctrlreg;

	RT_ASSERT(t);
	if(RT_NULL == t)
	{
		LOG_E("t should not be RT_NULL");
		return -RT_ERROR;
	}

	/*Read & change ctrl register, disable Alarm & update interrupt function*/
	/*disable interrupt*/
	if(RT_EOK!=rx8010_read_reg(RX8010_CTRL, 1, &ctrlreg))
	{
		return -RT_ERROR;
	}
	ctrlreg &= ~RX8010_CTRL_AIE;
	if(RT_EOK != rx8010_write_reg(RX8010_CTRL,ctrlreg))
	{
		return -RT_ERROR;
	}

	/*Read & change flag register, disable alarm function*/
	/*clear interrupt flag bit*/
	if(RT_EOK != rx8010_read_reg(RX8010_FLAG, 1, &flagreg))
	{
		return -RT_ERROR;
	}  
	flagreg &= ~RX8010_FLAG_AF;
	if(RT_EOK != rx8010_write_reg(RX8010_FLAG,flagreg))
	{
		return -RT_ERROR;
	}  

	/*Set alarm min*/
	alarmvals[0] = BIN_TO_BCD(t->Min);
	alarmvals[0] &= ~RX8010_ALARM_AE;
	if(RT_EOK != rx8010_write_reg(RX8010_ALMIN,alarmvals[0]))
	{
		return -RT_ERROR;
	} 

	/*Set alarm hour*/
	alarmvals[1] = BIN_TO_BCD(t->Hour);
	alarmvals[1] &= ~RX8010_ALARM_AE;
	if(RT_EOK != rx8010_write_reg(RX8010_ALHOUR,alarmvals[1]))
	{
		return -RT_ERROR;
	} 

	/*set wada flag to day of month*/
	if(RT_EOK != rx8010_read_reg(RX8010_EXT,1,&extreg))
	{
		return -RT_ERROR;
	} 
	extreg |= RX8010_EXT_WADA;
	if(RT_EOK != rx8010_write_reg(RX8010_EXT,extreg))
	{
		return -RT_ERROR;
	}

	/*Set alarm day_week/day_month*/
	alarmvals[2] = BIN_TO_BCD(t->Dmon);
	if (alarmvals[2] == 0)
		alarmvals[2] |= RX8010_ALARM_AE;
	else
		alarmvals[2] &= ~RX8010_ALARM_AE;
	if(RT_EOK != rx8010_write_reg(RX8010_ALWDAY,alarmvals[2]))
	{
		return -RT_ERROR;
	}

	rx8010_alarm_time_t alt;
	if(RT_EOK == rx8010_read_alarm(&alt))
	{
		LOG_D("rx8010 set alarm value min %d hour %d day %d",alt.Min,alt.Hour,alt.Dmon);
	}


	ctrlreg |=RX8010_CTRL_AIE;
	if(RT_EOK != rx8010_write_reg(RX8010_CTRL,ctrlreg))
	{
		return -RT_ERROR;
	}

	return RT_EOK;
}


/**
 * @description: enable or disable rx8010 alarm function
 * @param {type} 
 * @return: 
 */
rt_err_t rx8010_alarm_irq_enable(rt_flag_t enabled)
{
	rt_uint8_t flagreg;
	rt_uint8_t ctrl;
	rt_err_t err;

	if (enabled) {
		ctrl |= (RX8010_CTRL_AIE | RX8010_CTRL_UIE);
	} else {
		ctrl &= ~RX8010_CTRL_UIE;
		ctrl &= ~RX8010_CTRL_AIE;
	}

	if(RT_EOK!=rx8010_read_reg(RX8010_FLAG,1,&flagreg))
	{
		return -RT_ERROR;
	}
	flagreg &= ~RX8010_FLAG_AF;
	if(RT_EOK!=rx8010_write_reg(RX8010_FLAG,flagreg))
	{
		return -RT_ERROR;
	}

	if(RT_EOK!=rx8010_write_reg(RX8010_CTRL,ctrl))
	{
		return -RT_ERROR;
	}

	return RT_EOK;
}
#endif

