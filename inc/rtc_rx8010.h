/*
 * @Author: Alex.Pan
 * @Date: 2020-01-14 13:54:03
 * @LastEditTime : 2020-01-21 15:26:43
 * @LastEditors  : Alex.Pan
 * @Description: 
 * @FilePath: \rt-thread\bsp\AIot\packages\rx8010-v1.0.0\rtc-rx8010.h
 * @Copyright 2010-2015 LEKTEC or its affiliates. All Rights Reserved.
 */


#ifndef __RX8010_H__
#define __RX8010_H__

#include <rthw.h>
#include <rtthread.h>

typedef struct _rx8010_alarm_time_
{
   	rt_uint8_t  Min;  		/*alarm min*/      
	rt_uint8_t  Hour;       /*alarm min*/      
	rt_uint8_t  Dweek;       /*alarm week*/
	rt_uint8_t  Dmon;     	/*alarm day*/

}rx8010_alarm_time_t;


int rx8010_init(void);
rt_err_t  rx8010_set_time(struct tm *dt);
rt_err_t rx8010_get_time(struct tm *dt);
rt_err_t rx8010_set_alarm(rx8010_alarm_time_t *t);
#endif
