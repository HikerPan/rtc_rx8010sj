/*
 * @Author: Alex.Pan
 * @Date: 2020-01-14 13:53:51
 * @LastEditTime : 2020-01-21 14:53:03
 * @LastEditors  : Alex.Pan
 * @Description: 
 * @FilePath: \rt-thread\bsp\AIot\packages\rx8010-v1.0.0\rx8010_example.c
 * @Copyright 2010-2015 LEKTEC or its affiliates. All Rights Reserved.
 */
#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <time.h>
#include <rtc_rx8010.h>
#include <stdlib.h>
#ifdef RT_USING_FINSH
#include <finsh.h>
#endif

#if PKG_USING_RTC_RX8010SJ_EXAMPLE
// static void rx8010_start(int argc, char**argv)
// {
//     char *rx8010name = NULL;
//     if(2 == argc)
//     {
//         rx8010name = rt_malloc(strlen(argv[1])+1);
//         if(NULL != rx8010name)
//         {
//             rt_memset(rx8010name,0,strlen(argv[1])+1);
//             strcpy(rx8010name,argv[1]);
//             if(RT_EOK!=rx8010_init(rx8010name))
//             {
//                 rt_kprintf("rx8010 device %s init failed\r\n",rx8010name);
//             }
//             rt_free(rx8010name);
//             rx8010name = NULL;
//         }
//     }  
// }

static void rx8010_set(int argc, char**argv)
{
    struct tm *dt;
    
    if(argc<2)
    {
        rt_kprintf("rx8010_set usge: \r\n");
        rt_kprintf("1.set time: rx8010_set year month day hour min sec \r\n");
        rt_kprintf("2.sync rtc: rx8010_set sync \r\n");
        return;
    }
    
    /*sync rtc*/
    if(2==argc)
    {
        time_t now;
        /* output current time */
        now = time(RT_NULL);
        rt_kprintf("%s", ctime(&now));
        dt = localtime(&now);
        rx8010_set_time(dt);
    }

    /*set time*/
    if(7==argc)
    {
        dt = (struct tm *)rt_malloc(sizeof(struct tm));
        RT_ASSERT(dt);
        if(NULL == dt)
        {
            //LOG_E("memery failed");
            return;
        }
        dt->tm_year = atoi(argv[1])-1900;
        dt->tm_mon = atoi(argv[2])-1;
        dt->tm_mday = atoi(argv[3]);
        dt->tm_hour = atoi(argv[4]);
        dt->tm_min = atoi(argv[5]);
        dt->tm_sec = atoi(argv[6]);
        rt_free(dt);
        dt = NULL;
        rx8010_set_time(dt);
    }

    
}

static void rx8010_get(int argc, char**argv)
{
    struct tm dt;
    
    if(!rx8010_get_time(&dt))
    {
        rt_kprintf("year %d mon %d mday %d wday %d hour %d min %d sec %d\n",dt.tm_year+1900,\
            dt.tm_mon+1,\
            dt.tm_mday,\
            dt.tm_wday,\
            dt.tm_hour,\
            dt.tm_min,\
            dt.tm_sec);
    }

}

static void rx8010_alarm_example_set(int argc, char**argv)
{
    if(2 != argc)
    {
        rt_kprintf("rx8010_set usge: \r\n");
        rt_kprintf("set alarm time: rx8010_alarm_example_set min \r\n");
    }
    struct tm *dt;
    struct tm *adt;
    dt = (struct tm *)rt_malloc(sizeof(struct tm));
    if(RT_NULL != dt)
    {
        time_t rx8010time;
        rx8010_alarm_time_t alarmparm;
        
       rx8010_get_time(dt);
        rx8010time = mktime(dt);
        rx8010time+=60*atoi(argv[1]);
        adt = localtime(&rx8010time);
        
        alarmparm.Min = adt->tm_min;
        alarmparm.Hour = adt->tm_hour;
        alarmparm.Dmon = adt->tm_mday;
        rt_kprintf("cur time min %d hour %d day of mon %d \r\n",dt->tm_min,dt->tm_hour,dt->tm_mday);
        rt_kprintf("alarm val min %d hour %d day of month %d\r\n",alarmparm.Min,alarmparm.Hour,alarmparm.Dmon);
        if(RT_EOK == rx8010_set_alarm(&alarmparm))
        {
            rt_pin_mode(70,PIN_MODE_OUTPUT);
            rt_pin_write(70,0);
        }  

        rt_free(dt);
        dt = NULL;
        
    }


      
}
#ifdef RT_USING_FINSH
// MSH_CMD_EXPORT(rx8010_start, start rx8010 device);
MSH_CMD_EXPORT(rx8010_set, set rx8010 device time);
MSH_CMD_EXPORT(rx8010_get, get rx8010 device time);
MSH_CMD_EXPORT(rx8010_alarm_example_set, set rx8010 alarm time);
#endif
#endif