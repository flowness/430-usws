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

char *UssErrorToString(int i)
{
    switch(i)
    {
        case 0: return "no error";
        case 1: return "SAPH pulseLowPhasePeriod error";
        case 2: return "SAPH pulseHighPhasePeriod error";
        case 3: return "SAPH numOfExcitationPulses error";
        case 4: return "SAPH numOfStopPulses error";
        case 5: return "SAPH update error ongoing conversion";
        case 6: return "SAPH time startPPGCount error";
        case 7: return "SAPH time turnOnADCCount error";
        case 8: return "SAPH time startADCsamplingCount error";
        case 9: return "SAPH time restartCaptureCount error";
        case 10: return "SAPH time captureTimeOutCount error";
        case 11: return "SAPH time startPGAandINBiasCount error";
        case 12: return "SAPH invalid bias impedance error";
        case 13: return "SAPH invalid rx charge pump mode error";
        case 21: return "HSPLL pllXtalFreq inHz error";
        case 22: return "HSPLL pllOutputFreq inHz error";
        case 23: return "HSPLL plllock error";
        case 24: return "HSPLL pll unlock error";
        case 26: return "HSPLL update error ongoing conversion";
        case 27: return "HSPLL verification expected count error";
        case 28: return "HSPLL invalid settling count error";
        case 41: return "SDHS threshold error";
        case 43: return "SDHS conversion overflow error";
        case 44: return "SDHS sample size error";
        case 45: return "SDHS update error ongoing conversion";
        case 46: return "SDHS window low threshold reached";
        case 47: return "SDHS window high threshold reached";
        case 48: return "SDHS max size error";
        case 61: return "UUPS update error ongoing conversion";
        case 62: return "UUPS power up time out error";
        case 63: return "UUPS power up error";
        case 64: return "UUPS power down error";
        case 81: return "data error abort";
        case 82: return "ASQ time out";
        case 101: return "measurement stopped";
        case 102: return "error conversion stopped by debugger";
        case 121: return "parameter check failed";
        case 122: return "valid results";
        case 123: return "algorithm error";
        case 124: return "algorithm error invalid iteration value";
        case 125: return "algorithm error no signal detected ups channel";
        case 126: return "algorithm error no signal detected ups dns channel";
        case 127: return "algorithm error no signal detected dns channel";
        case 128: return "algorithm captures accumulated";
        case 129: return "algorithm error invalid clock relative error";
        case 130: return "algorithm error invalid filter length";
        case 142: return "interrupt update error ongoing conversion";
        case 161: return "invalid conversion data";
        case 171: return "Calibration DAC success";
        case 172: return "Calibration DAC error";
        case 173: return "Calibration Gain error";
        case 174: return "Calibration DCO error";
        case 175: return "Signal Gain Calibration timeout";
        case 176: return "Signal Gain Calibration settling";
        case 177: return "Signal Gain Calibration successful";
        case 178: return "Calibration DAC timeout error";
        case 240: return "ACLK settling time out";
        case 241: return "silicon version does not support this functionality";
        case 254: return "USS ongoing active conversion error";
        default:
        case 255: return "error occurred";
    }
}

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
    int j=0;
    long ipart = (long)n;
    double fpart = n - (double)ipart;
    if(fpart<0) fpart*=-1;

    // convert integer part to string
    long i = longToStr(ipart, res, 1);

    // check for display option after point
    if (afterpoint != 0)
    {
        res[i++] = '.';  // add dot

        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter is needed
        // to handle cases like 233.007
        for(;j<afterpoint;j++) fpart*=10;

//        fpart = fpart * powl(10, afterpoint);

        longToStr((long)fpart, res+i, afterpoint);
    }
}

// Converts a floating point number to string.
void ftoa(float n, char *res, int afterpoint)
{
    int j=0;

    // Extract integer part
    int ipart = (int)n;

    // Extract floating part
    float fpart = n - (float)ipart;
    if(fpart<0) fpart*=-1;

    // convert integer part to string
    int i = intToStr(ipart, res, 1);

    // check for display option after point
    if (afterpoint != 0)
    {
        res[i++] = '.';  // add dot

        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter is needed
        // to handle cases like 233.007
        for(;j<afterpoint;j++) fpart*=10;
        //fpart = fpart * powl(10, afterpoint);

        longToStr((long)fpart, res+i, afterpoint);
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

void Message_Debug(const char *str)
{
    int i=0;
    while(str[i]!=0)
    {
        EUSCI_A_UART_transmitData(EUSCI_A1_BASE, str[i++]);
    }
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
    }
    Message(pcBuff);

    return iRet;
}

void debugPrintSamples(int16_t *pUPSCap,int16_t *pDNSCap,uint16_t updnsCaptureSize)
{
    char        pcBuff[2048];
    char        *ptr=pcBuff;
    int i=0;

    ptr+= sprintf (ptr, "{\"UPS\": [%d",pUPSCap[i++]);
    while(i<updnsCaptureSize)
    {
        ptr+= sprintf (ptr, ",%d", pUPSCap[i++]);
    }

    i=0;
    ptr+= sprintf (ptr, "], \"UDS\": [%d",pDNSCap[i++]);
    while(i<updnsCaptureSize)
    {
        ptr+= sprintf (ptr, ",%d", pDNSCap[i++]);
    }

    ptr+= sprintf (ptr, "] }\r\n\0");

    Message_Debug(pcBuff);

    //debugReport("{\"UPS\": [%s], \"UDS\": [%s] }\0",UPSCapBuff,UDSCapBuff);
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
    double totalVolume=0;
    double AvrgFlow=0;
    //float LiterPerMinuteFactor = 1;
    float currentFloat=0;
    unsigned char seconds=0;
    //unsigned char minutes=0;
    char    YearString[5]={0};
    char    MonthString[3]={0};
    char    DayString[3]={0};
    char    HourString[3]={0};
    char    MinuteString[3]={0};
    char    SecondString[3]={0};
    char float_Result[32]={0};
    char float_totalResult[32]={0};
    Calendar currentTime;
    int US_MeaseCountTotal=0;
    int US_CurrentMeaseCountSuccess=0;

    int USmeasErrorsCount = 0;
    int USalgorithmErrorsCount = 0;

    // Configure UART

    // Uart to CC32 via Board interface
    GPIO_setAsPeripheralModuleFunctionInputPin(
    GPIO_PORT_P8,
    GPIO_PIN2+GPIO_PIN3,
    GPIO_PRIMARY_MODULE_FUNCTION
    );

    // debug Uart for console
    GPIO_setAsPeripheralModuleFunctionInputPin(
    GPIO_PORT_P1,
    GPIO_PIN2+GPIO_PIN3,
    GPIO_PRIMARY_MODULE_FUNCTION
    );


    PMM_unlockLPM5();

    //Setup Current Time for Calendar
    currentTime.Seconds    = 0x00;
    currentTime.Minutes    = 0x45;
    currentTime.Hours      = 0x09;
    currentTime.DayOfWeek  = 0x06;
    currentTime.DayOfMonth = 0x18;
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


    EUSCI_A_UART_init(EUSCI_A1_BASE, &param);

    EUSCI_A_UART_enable(EUSCI_A1_BASE);

    EUSCI_A_UART_clearInterrupt(EUSCI_A1_BASE,
      EUSCI_A_UART_RECEIVE_INTERRUPT);

    // Enable USCI_A1 RX interrupt
    EUSCI_A_UART_enableInterrupt(EUSCI_A1_BASE,
      EUSCI_A_UART_RECEIVE_INTERRUPT);

    EUSCI_A_UART_init(EUSCI_A3_BASE, &param);

    EUSCI_A_UART_enable(EUSCI_A3_BASE);

    EUSCI_A_UART_clearInterrupt(EUSCI_A3_BASE,
      EUSCI_A_UART_RECEIVE_INTERRUPT);

    // Enable USCI_A3 RX interrupt
    EUSCI_A_UART_enableInterrupt(EUSCI_A3_BASE,
      EUSCI_A_UART_RECEIVE_INTERRUPT);                     // Enable interrupt

    code = USS_initAlgorithms(&gUssSWConfig);
    if (code != USS_message_code_no_error) { Report("\"USS_initAlgorithms error:\" (%d)-%s\n\r", code,UssErrorToString(code)); }

    code = USS_configureUltrasonicMeasurement(&gUssSWConfig);
    if (code != USS_message_code_no_error) { Report("\"USS_configureUltrasonicMeasurement error:\" (%d)-%s\n\r", code,UssErrorToString(code)); }

    code = USS_calibrateSignalGain(&gUssSWConfig);
    if (code != USS_message_code_Signal_Gain_Calibration_successful) { Report("\"USS_calibrateSignalGain error:\" (%d)-%s\n\r", code,UssErrorToString(code)); }

    code = USS_estimateDCoffset(&gUssSWConfig,
        USS_dcOffEst_Calc_Mode_trigger_UPS_DNS_capture_controlled_by_ASQ);
    if (code != USS_message_code_no_error) { Report("\"USS_estimateDCoffset error:\" (%d)-%s\n\r", code,UssErrorToString(code)); }


    while (1)
    {
        US_MeaseCountTotal++;

        code = USS_startUltrasonicMeasurement(
                &gUssSWConfig, USS_capture_power_mode_low_power_mode_0);

        debugPrintSamples(USS_getUPSPtr(),USS_getDNSPtr(),gUssSWConfig.captureConfig->sampleSize);

        if (code != USS_message_code_no_error)
        {
            Report("\"Meas error:\" (%d)-%s\n\r", code,UssErrorToString(code));
            USmeasErrorsCount++;
        }
        else
        {

            //Report("\"No Meas error\"\n\r");
            code = USS_runAlgorithms(&gUssSWConfig, &algorithms_Results);
            USS_generateLPMDelay(&gUssSWConfig,
                                 USS_low_power_mode_option_low_power_mode_3,
                                 USS_LOW_POWER_RESTART_CAP_COUNT);
            if (code != USS_message_code_valid_results)
            {
                Report("\"Algorithm error:\" (%d)-%s\n\r", code,UssErrorToString(code));
                USalgorithmErrorsCount++;
            }
            else
            {
                //totalVolume+=algorithms_Results.volumeFlowRate;
                AvrgFlow+=algorithms_Results.volumeFlowRate;

                US_CurrentMeaseCountSuccess++;
            }

        }

        currentTime=RTC_C_getCalendarTime(RTC_C_BASE);

        if(seconds!=currentTime.Seconds)
        //if(minutes!=currentTime.Minutes)
        {
            seconds = currentTime.Seconds;
            currentTime=RTC_C_getCalendarTime(RTC_C_BASE);
            sprintf(YearString,"%x",currentTime.Year);
            PadString(MonthString,currentTime.Month);
            PadString(DayString,currentTime.DayOfMonth);
            PadString(HourString,currentTime.Hours);
            PadString(MinuteString,currentTime.Minutes);
            PadString(SecondString,currentTime.Seconds);

            //minutes = currentTime.Minutes;

            // if at least 1 measurement was valid
            if (US_CurrentMeaseCountSuccess > 0)
            {
                // If reported each second - we need to divide the flow rate by 60, as it calculated by Liter per Minute
                //LiterPerMinuteFactor = 1;

                //currentFloat=(float)(AvrgFlow/LiterPerMinuteFactor/count);
                currentFloat=(float)(AvrgFlow/US_CurrentMeaseCountSuccess);


                //ftoa(algorithms_Results.volumeFlowRate,float_Result,6);
                ftoa(currentFloat,float_Result,6);

                currentFloat/=60;
                totalVolume+=currentFloat;
                dtoa(totalVolume,float_totalResult,6);

                //20180514T194646Z
                Report("{ \"measurementDate\": \"%s%s%sT%s%s%sZ\", \"measurementInterval\": %d, \"measurementAmount\": %s, \"moduleSN\": \"MSP430ABCD\", \"totalCount\": %s, \"US_MeaseCountTotal\": %d, \"USmeasErrorsCount\": %d, \"USalgorithmErrorsCount\": %d}\n\r",YearString,MonthString,DayString,HourString,MinuteString,SecondString,US_CurrentMeaseCountSuccess,float_Result,float_totalResult, US_MeaseCountTotal, USmeasErrorsCount, USalgorithmErrorsCount);
            }
            else
            {
                Report("{ \"US_MeaseCountTotal\": %d, \"USmeasErrorsCount\": %d, \"USalgorithmErrorsCount\": %d }\n\r",US_MeaseCountTotal, USmeasErrorsCount, USalgorithmErrorsCount);
            }

            US_CurrentMeaseCountSuccess=0;
            AvrgFlow=0;
            currentFloat=0;
            USmeasErrorsCount = 0;
            USalgorithmErrorsCount = 0;
        }
    }

}


//******************************************************************************
//
//This is the USCI_A0 interrupt vector service routine.
//
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A1_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_A1_VECTOR)))
#endif
void USCI_A1_ISR(void)
{
  switch(__even_in_range(UCA1IV,USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
        RXData=EUSCI_A_UART_receiveData(EUSCI_A1_BASE);
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
