/* --COPYRIGHT--,BSD
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * --/COPYRIGHT--*/
#include <msp430.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <stdint.h>
#include <stdbool.h>
#include "ussSwLib.h"
#include "USS_Config/USS_userConfig.h"
#include "driverlib.h"


extern int vsnprintf (char * s, size_t n, const char * format, va_list arg );

USS_Algorithms_Results algorithms_Results;
volatile Calendar newTime;

uint8_t RXData = 0, TXData = 0;
uint8_t check = 0;
char pcBuffer[256]={0};
int     iLen = 0;

// reverses a string 'str' of length 'len'
void reverse(char *str, int len)
{
    int i=0, j=len-1, temp;
    while (i<j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++; j--;
    }
}
// Converts a given integer x to string str[].  d is the number
// of digits required in output. If d is more than the number
// of digits in x, then 0s are added at the beginning.
int longToStr(long x, char str[], int d)
{
   bool Negativ=false;
   int i = 0;
   if(x<0)
   {
       Negativ=true;
       x*=-1;
   }

   while (x)
   {
       str[i++] = (x%10) + '0';
       x/=10;
   }

   // If number of digits required is more, then
   // add 0s at the beginning
   while (i < d)
       str[i++] = '0';

   if(Negativ) str[i++]='-';

   reverse(str, i);
   str[i] = '\0';
   return i;
}
 // Converts a given integer x to string str[].  d is the number
 // of digits required in output. If d is more than the number
 // of digits in x, then 0s are added at the beginning.
int intToStr(int x, char str[], int d)
{
    bool Negativ=false;
    int i = 0;
    if(x<0)
    {
        Negativ=true;
        x*=-1;
    }

    while (x)
    {
        str[i++] = (x%10) + '0';
        x/=10;
    }

    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';

    if(Negativ) str[i++]='-';

    reverse(str, i);
    str[i] = '\0';
    return i;
}

void dtoa(double n, char *res, int afterpoint)
{
    long ipart = (long)n;
    double fpart = n - (double)ipart;
    if(fpart<0) fpart*=-1;

    // convert integer part to string
    long i = longToStr(ipart, res, 0);

    // check for display option after point
    if (afterpoint != 0)
    {
        res[i] = '.';  // add dot

        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter is needed
        // to handle cases like 233.007
        fpart = fpart * pow(10, afterpoint);

        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}

// Converts a floating point number to string.
void ftoa(float n, char *res, int afterpoint)
{
    // Extract integer part
    int ipart = (int)n;

    // Extract floating part
    float fpart = n - (float)ipart;
    if(fpart<0) fpart*=-1;

    // convert integer part to string
    int i = intToStr(ipart, res, 0);

    // check for display option after point
    if (afterpoint != 0)
    {
        res[i] = '.';  // add dot

        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter is needed
        // to handle cases like 233.007
        fpart = fpart * pow(10, afterpoint);

        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}


//*****************************************************************************
//
//! Outputs a character string to the console
//!
//! This function
//!        1. prints the input string character by character on to the console.
//!
//! \param[in]  str - is the pointer to the string to be printed
//!
//! \return none
//!
//! \note If UART_NONPOLLING defined in than Message or UART write should be
//!       called in task/thread context only.
//
//*****************************************************************************
void Message(const char *str)
{
    int i=0;
    while(str[i]!=0)
    {
        EUSCI_A_UART_transmitData(EUSCI_A3_BASE, str[i++]);
    }

    EUSCI_A_UART_transmitData(EUSCI_A3_BASE, 0);
}


//*****************************************************************************
//
//! prints the formatted string on to the console
//!
//! \param[in]  format  - is a pointer to the character string specifying the
//!                       format in the following arguments need to be
//!                       interpreted.
//! \param[in]  [variable number of] arguments according to the format in the
//!             first parameters
//!
//! \return count of characters printed
//
//*****************************************************************************
int Report(const char *pcFormat, ...)
{
    int         iRet = 0;
    char        pcBuff[256]={0};
    int         iSize = 256;
    va_list     list;


    while(1)
    {
        va_start(list,pcFormat);
        iRet = vsnprintf(pcBuff, iSize, pcFormat, list);
        va_end(list);
        if((iRet > -1) && (iRet < iSize))
        {
            break;
        }
        /*
        else
        {
            iSize *= 2;
            if((pcTemp = realloc(pcBuff, iSize)) == NULL)
            {
                Message("Could not reallocate memory\n\r");
                iRet = -1;
                break;
            }
            else
            {
                pcBuff = pcTemp;
            }
        }
        */
    }
    Message(pcBuff);

    return iRet;
}
void PadString(char *buffer,uint8_t time)
{
    if(time<0x10)
        sprintf(buffer,"0%x",time);
    else
        sprintf(buffer,"%x",time);
}

int main(void)
{
    volatile USS_message_code code;
    static double totalVolume=0;
    static double AvrgFlow=0;
    float currentFloat=0;
    unsigned char seconds=0;
    char    YearString[5]={0};
    char    MonthString[3]={0};
    char    DayString[3]={0};
    char    HourString[3]={0};
    char    MinuteString[3]={0};
    char    SecondString[3]={0};
    char float_Result[32]={0};
    char float_totalResult[32]={0};
    Calendar currentTime;
    int count=0;


    // Configure UART

    // Uart to CC32 via Board interface
    GPIO_setAsPeripheralModuleFunctionInputPin(
    GPIO_PORT_P8,
    GPIO_PIN2+GPIO_PIN3,
    GPIO_PRIMARY_MODULE_FUNCTION
    );

    // debug Uart for console
    GPIO_setAsPeripheralModuleFunctionInputPin(
    GPIO_PORT_P2,
    GPIO_PIN0+GPIO_PIN1,
    GPIO_PRIMARY_MODULE_FUNCTION
    );


    PMM_unlockLPM5();

    //Setup Current Time for Calendar
    currentTime.Seconds    = 0x00;
    currentTime.Minutes    = 0x26;
    currentTime.Hours      = 0x23;
    currentTime.DayOfWeek  = 0x02;
    currentTime.DayOfMonth = 0x14;
    currentTime.Month      = 0x05;
    currentTime.Year       = 0x2018;

    //Initialize Calendar Mode of RTC
    /*
     * Base Address of the RTC_B
     * Pass in current time, intialized above
     * Use BCD as Calendar Register Format
     */
    RTC_C_initCalendar(RTC_C_BASE,
        &currentTime,
        RTC_C_FORMAT_BCD);

    //Start RTC Clock
    RTC_C_startClock(RTC_C_BASE);

    EUSCI_A_UART_initParam param = {0};
    param.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
    param.clockPrescalar = 4;
    param.firstModReg = 5;
    param.secondModReg = 85;
    param.parity = EUSCI_A_UART_NO_PARITY;
    param.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
    param.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
    param.uartMode = EUSCI_A_UART_MODE;
    param.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;


    EUSCI_A_UART_init(EUSCI_A0_BASE, &param);

    EUSCI_A_UART_enable(EUSCI_A0_BASE);

    EUSCI_A_UART_clearInterrupt(EUSCI_A0_BASE,
      EUSCI_A_UART_RECEIVE_INTERRUPT);

    // Enable USCI_A0 RX interrupt
    EUSCI_A_UART_enableInterrupt(EUSCI_A0_BASE,
      EUSCI_A_UART_RECEIVE_INTERRUPT);

    EUSCI_A_UART_init(EUSCI_A3_BASE, &param);

    EUSCI_A_UART_enable(EUSCI_A3_BASE);

    EUSCI_A_UART_clearInterrupt(EUSCI_A3_BASE,
      EUSCI_A_UART_RECEIVE_INTERRUPT);

    // Enable USCI_A3 RX interrupt
    EUSCI_A_UART_enableInterrupt(EUSCI_A3_BASE,
      EUSCI_A_UART_RECEIVE_INTERRUPT);                     // Enable interrupt

    code = USS_initAlgorithms(&gUssSWConfig);
    code = USS_configureUltrasonicMeasurement(&gUssSWConfig);
    code = USS_calibrateSignalGain(&gUssSWConfig);
    code = USS_estimateDCoffset(&gUssSWConfig,
        USS_dcOffEst_Calc_Mode_trigger_UPS_DNS_capture_controlled_by_ASQ);

    while (1)
    {

        code = USS_startUltrasonicMeasurement(
                &gUssSWConfig, USS_capture_power_mode_low_power_mode_0);
        code = USS_runAlgorithms(&gUssSWConfig, &algorithms_Results);
        USS_generateLPMDelay(&gUssSWConfig,
                             USS_low_power_mode_option_low_power_mode_3,
                             USS_LOW_POWER_RESTART_CAP_COUNT);
        //totalVolume+=algorithms_Results.volumeFlowRate;
        AvrgFlow+=algorithms_Results.volumeFlowRate;
        currentTime=RTC_C_getCalendarTime(RTC_C_BASE);

        count++;
        if(seconds!=currentTime.Seconds)
        {
            seconds = currentTime.Seconds;
            currentFloat=(float)(AvrgFlow/count);
            //ftoa(algorithms_Results.volumeFlowRate,float_Result,6);
            ftoa(currentFloat,float_Result,6);

            totalVolume+=AvrgFlow;
            dtoa(totalVolume,float_totalResult,6);

            currentTime=RTC_C_getCalendarTime(RTC_C_BASE);
            sprintf(YearString,"%x",currentTime.Year);
            PadString(MonthString,currentTime.Month);
            PadString(DayString,currentTime.DayOfMonth);
            PadString(HourString,currentTime.Hours);
            PadString(MinuteString,currentTime.Minutes);
            PadString(SecondString,currentTime.Seconds);

            //20180514T194646Z
            Report("{ \"measurementDate\": \"%s%s%sT%s%s%sZ\", \"measurementInterval\": %d, \"measurementAmount\": %s, \"moduleSN\": \"MSP430ABCD\", \"totalCount\": %s}",YearString,MonthString,DayString,HourString,MinuteString,SecondString,count,float_Result,float_totalResult);
            count=0;
            AvrgFlow=0;
            currentFloat=0;

        }
    }

}


//******************************************************************************
//
//This is the USCI_A0 interrupt vector service routine.
//
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_A0_VECTOR)))
#endif
void USCI_A0_ISR(void)
{
  switch(__even_in_range(UCA0IV,USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
        RXData=EUSCI_A_UART_receiveData(EUSCI_A0_BASE);
        pcBuffer[iLen] = RXData;

        //
        // Handling overflow of buffer
        //
        if(iLen >= 256)
        {
            iLen=0;
        }

        //
        // Copying Data from UART into a buffer
        //
        if(RXData == '\0')
        {
            //ProcessString();
            //break;
        }
        iLen++;

      break;
    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
  }
}

//******************************************************************************
//
//This is the USCI_A3 interrupt vector service routine.
//
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A3_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_A3_VECTOR)))
#endif
void USCI_A3_ISR(void)
{
  switch(__even_in_range(UCA3IV,USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
        RXData=EUSCI_A_UART_receiveData(EUSCI_A3_BASE);
        pcBuffer[iLen] = RXData;

        //
        // Handling overflow of buffer
        //
        if(iLen >= 256)
        {
            iLen=0;
        }

        //
        // Copying Data from UART into a buffer
        //
        if(RXData == '\0')
        {
            //ProcessString();
            //break;
        }
        iLen++;

      break;
    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
  }
}
