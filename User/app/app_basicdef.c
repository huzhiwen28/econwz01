
/*  
Чёву
time:2008,09,26
*/
#include "stm32f10x.h"
#include "app_basicdef.h"

typedef unsigned int uint;    
typedef unsigned long ulong;    
typedef unsigned char uchar;

uint ADCBuf[21];

int PidOut;
int KPout;
int KIout;
int KDout;

struct PID Pid; // PID Control Structure

struct PidRunState RUNState;
 
struct ParameterSate Param;

struct state State;

struct PortState Port;

unsigned int rout; // PID Response (Output)
unsigned int rin; // PID Feedback (Input)

uint  OUTDATA[10];
uint  OUTDATB[10];
long int SETD;



