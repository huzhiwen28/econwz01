#include "app_basicdef.h"

/*
*********************************************************************************************************
*
*                                             应用程序配置文件
*********************************************************************************************************
*/

#ifndef  __APP_CFG_H__
#define  __APP_CFG_H__

/*
*********************************************************************************************************
*                                              TASKS NAMES
*********************************************************************************************************
*/

#define  APP_TASK_START_NAME                          "Start Task"
#define  APP_TASK_232C_NAME                        		"232C"
#define  APP_TASK_USBCOM_NAME                        	"USBCOM"
#define  APP_TASK_USBGATHER_NAME                        "USBGather"
#define  APP_TASK_HEARTBEAT_NAME                        	"HeartBeat"
#define  APP_TASK_SERVOTIMER_NAME                   		"servotimer"
#define  APP_TASK_SERVO_NAME                   				"servo"
/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/

#define  APP_TASK_START_PRIO                               	7
#define  APP_TASK_232C_PRIO                            		13
#define  APP_TASK_USBCOM_PRIO                            	14
#define  APP_TASK_USBGATHER_PRIO                            15
#define  APP_TASK_HEARTBEAT_PRIO                            17
#define  APP_TASK_SERVOTIMER_PRIO                       	8
#define  APP_TASK_SERVO_PRIO                       			10

#define  OS_TASK_TMR_PRIO                (OS_LOWEST_PRIO - 2)

/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/

#define  APP_TASK_START_STK_SIZE                         	512
#define  APP_TASK_232C_STK_SIZE                       		100
#define  APP_TASK_USBCOM_STK_SIZE                       	200
#define  APP_TASK_USBGATHER_STK_SIZE                       	300
#define  APP_TASK_HEARTBEAT_STK_SIZE                       	300
#define  APP_TASK_SERVOTIMER_STK_SIZE                   	50
#define  APP_TASK_SERVO_STK_SIZE                   			200

#define MODBUSID 1
#define ControlVersion	10001
//#define TEST232
//#define TEST485
#endif
