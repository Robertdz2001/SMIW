/******************************************************************************
* File Name: main_cm4.c
*
* Version: 1.20
*
* Description: This file main application code for the CE223727 EmWin Graphics
*				library EInk Display.
*
* Hardware Dependency: CY8CKIT-028-EPD E-Ink Display Shield
*					   CY8CKIT-062-BLE PSoC6 BLE Pioneer Kit
*
******************************************************************************* 
* Copyright (2019), Cypress Semiconductor Corporation. All rights reserved. 
******************************************************************************* 
* This software, including source code, documentation and related materials 
* (“Software”), is owned by Cypress Semiconductor Corporation or one of its 
* subsidiaries (“Cypress”) and is protected by and subject to worldwide patent 
* protection (United States and foreign), United States copyright laws and 
* international treaty provisions. Therefore, you may use this Software only 
* as provided in the license agreement accompanying the software package from 
* which you obtained this Software (“EULA”). 
* 
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive, 
* non-transferable license to copy, modify, and compile the Software source 
* code solely for use in connection with Cypress’s integrated circuit products. 
* Any reproduction, modification, translation, compilation, or representation 
* of this Software except as specified above is prohibited without the express 
* written permission of Cypress. 
* 
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress 
* reserves the right to make changes to the Software without notice. Cypress 
* does not assume any liability arising out of the application or use of the 
* Software or any product or circuit described in the Software. Cypress does 
* not authorize its products for use in any products where a malfunction or 
* failure of the Cypress product may reasonably be expected to result in 
* significant property damage, injury or death (“High Risk Product”). By 
* including Cypress’s product in a High Risk Product, the manufacturer of such 
* system or application assumes all risk of such use and in doing so agrees to 
* indemnify Cypress against all liability.
********************************************************************************/
/******************************************************************************
* This file contains the main application code for CE223727 that demonstrates
* controlling a EInk display using the EmWin Graphics Library.
* The project displays a start up screen with Cypress logo and text "CYPRESS EMWIN
* GRAPHICS DEMO EINK DISPLAY".  The project then displays the following screens
* in a loop
*
*	1. A screen showing various text alignments, styles and modes
*   2. A screen showing normal fonts
*	3. A screen showing bold fonts
*	4. A screen showing 2D graphics with horizontal lines, vertical lines
*		arcs and filled rounded rectangle
*	5. A screen showing 2D graphics with concentric circles and ellipses
*	6. A screen showing a text box with wrapped text
*
 *******************************************************************************/
#include "project.h"
#include "GUI.h"
#include "pervasive_eink_hardware_driver.h"
#include "cy_eink_library.h"
#include "LCDConf.h"
#include <stdio.h>
#include <stdlib.h>

/* Image buffer cache */
uint8 imageBufferCache[CY_EINK_FRAME_SIZE] = {0};

/* Reference to the bitmap image for the startup screen */
extern GUI_CONST_STORAGE GUI_BITMAP bmCypressLogoFullColor_PNG_1bpp;

/* Function prototypes */
void ShowGraph(void);

/* Array of demo pages functions */
void (*demoPageArray[])(void) = {
    ShowGraph
};

/* Number of demo pages */
#define NUMBER_OF_DEMO_PAGES    (sizeof(demoPageArray)/sizeof(demoPageArray[0]))

/*******************************************************************************
* Function Name: void UpdateDisplay(void)
********************************************************************************
*
* Summary: This function updates the display with the data in the display 
*			buffer.  The function first transfers the content of the EmWin
*			display buffer to the primary EInk display buffer.  Then it calls
*			the Cy_EINK_ShowFrame function to update the display, and then
*			it copies the EmWin display buffer to the Eink display cache buffer
*
* Parameters:
*  None
*
* Return:
*  None
*
* Side Effects:
*  It takes about a second to refresh the display.  This is a blocking function
*  and only returns after the display refresh
*
*******************************************************************************/
void UpdateDisplay(cy_eink_update_t updateMethod, bool powerCycle)
{
    cy_eink_frame_t* pEmwinBuffer;

    /* Get the pointer to Emwin's display buffer */
    pEmwinBuffer = (cy_eink_frame_t*)LCD_GetDisplayBuffer();

    /* Update the EInk display */
    Cy_EINK_ShowFrame(imageBufferCache, pEmwinBuffer, updateMethod, powerCycle);

    /* Copy the EmWin display buffer to the imageBuffer cache*/
    memcpy(imageBufferCache, pEmwinBuffer, CY_EINK_FRAME_SIZE);
}

/*******************************************************************************
* Function Name: void ShowStartupScreen(void)
********************************************************************************
*
* Summary: This function displays the startup screen with Cypress Logo and 
*			the demo description text
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void ShowStartupScreen(void)
{
    /* Set foreground and background color and font size */
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetColor(GUI_BLACK);
    GUI_SetBkColor(GUI_WHITE);
    GUI_Clear();

    GUI_DrawBitmap(&bmCypressLogoFullColor_PNG_1bpp, 2, 2);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("CYPRESS", 132, 85);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("EMWIN GRAPHICS", 132, 105);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("EINK DISPLAY DEMO", 132, 125);

    /* Send the display buffer data to display*/
    UpdateDisplay(CY_EINK_FULL_4STAGE, true);
}

/*******************************************************************************
* Function Name: void ClearScreen(void)
********************************************************************************
*
* Summary: This function clears the screen
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void ClearScreen(void)
{
    GUI_SetColor(GUI_BLACK);
    GUI_SetBkColor(GUI_WHITE);
    GUI_Clear();
    UpdateDisplay(CY_EINK_FULL_4STAGE, true);
}


/*******************************************************************************
* Function Name: void WaitforSwitchPressAndRelease(void)
********************************************************************************
*
* Summary: This implements a simple "Wait for button press and release"
*			function.  It first waits for the button to be pressed and then
*			waits for the button to be released.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Side Effects:
*  This is a blocking function and exits only on a button press and release
*
*******************************************************************************/
int compareValue = 50;

int start_point = 0;

int which_data_table = 1;
int dataSize_global = 0;
#define GRAPH_POINTS 12
int modeSwitch = 0;

void WaitforSwitchPressAndRelease(void)
{   
    Cy_TCPWM_PWM_SetCompare0(PWM_HW, PWM_CNT_NUM, compareValue);
    /* Wait for SW2 to be pressed */
    while(Status_SW2_Read() != 0);
    
    /* Wait for SW2 to be released */
    while(Status_SW2_Read() == 0)
    {
        Cy_TCPWM_PWM_SetCompare0(PWM_HW, PWM_CNT_NUM, compareValue);
        compareValue = (compareValue + 1) % 100;
        CyDelay(20);
    };
    
    if(modeSwitch == 1)
    {
        start_point = 0;
        which_data_table = 1;
        modeSwitch = 0;
    }
    else if(start_point + 6 < dataSize_global)
    {
        start_point += 6;
    }
    else
    {
        start_point = 0;
        which_data_table++;
        
        if(which_data_table == 3){
            which_data_table = 1;
            modeSwitch = 1;
        }
    }
}

/*******************************************************************************
* Function Name: int main(void)
********************************************************************************
*
* Summary: This is the main function.  Following functions are performed
*			1. Initialize the EmWin library
*			2. Display the startup screen for 3 seconds
*			3. Display the instruction screen and wait for key press and release
*			4. Inside a while loop scroll through the 6 demo pages on every
*				key press and release
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void InitGraph()
{
        /* Set font size, foreground and background colors */
    GUI_SetColor(GUI_BLACK);
    GUI_SetBkColor(GUI_WHITE);
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_SetTextStyle(GUI_TS_NORMAL);

    /* Clear the screen */
    GUI_Clear();

    /* Display page title */
    GUI_SetFont(GUI_FONT_13B_1);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt("LINE GRAPH DEMO", 132, 5);
}

typedef struct {
    int time;
    int temperature;
} DataPoint;

DataPoint reflowData[5] = {
    {0, 0}, {90, 150}, {180, 150}, {220, 245}, {360, 0}
};

DataPoint leadData[GRAPH_POINTS] = {
    {0, 25}, {30, 125}, {60, 150}, {90, 180}, {120, 210}, {140, 230}, {160, 230},
    {180, 210}, {200, 180}, {220, 150}, {240, 125}, {260, 25}
};

int whichData = 0;

void ShowGraph(void)
{
    ClearScreen();
    
    whichData++;
    
    if(whichData == 3){
        whichData = 1;
    }
    
    int dataSize = 0;
    
    if(whichData == 1)
    {
        dataSize = sizeof(reflowData) / sizeof(reflowData[0]);
    }
    else if(whichData == 2)
    {
        dataSize = sizeof(leadData) / sizeof(leadData[0]);
    }
    
    DataPoint data[dataSize];
    
    if(whichData == 1)
    {
        for (int i = 0; i < dataSize; i++) {
            data[i].temperature = reflowData[i].temperature;
            data[i].time = reflowData[i].time;
        }
    }
    else if(whichData == 2)
    {
        for (int i = 0; i < dataSize; i++) {
            data[i].temperature = leadData[i].temperature;
            data[i].time = leadData[i].time;
        }
    }
    
    /* Define axis labels and ticks */
    char xAxisLabels[8][6] = {"0", "60", "120", "180", "240", "300", "360", "420"};
    char yAxisLabels[6][4] = {"0", "60", "120", "180", "240", "300"};
    int xAxisTicks[8] = {30, 60, 90, 120, 150, 180, 210, 240};
    int yAxisTicks[6] = {20, 50, 80, 110, 140, 170};
    
    /* Draw the graph */
    GUI_ClearRect(10, 20, 254, 150);  // Clear the area where the graph will be drawn
    
    /* Draw X and Y axis */
    GUI_DrawLine(30, 20, 30, 170); // Y axis
    GUI_DrawLine(30, 150, 250, 150); // X axis

    /* Draw axis labels and ticks */
    for (int i = 0; i < 8; i++) {
        GUI_DrawLine(xAxisTicks[i], 150, xAxisTicks[i], 155);
        GUI_DispStringAt(xAxisLabels[i], xAxisTicks[i] - 10, 155);
    }

    for (int i = 0; i < 6; i++) {
        GUI_DrawLine(25, 170 - yAxisTicks[i], 30, 170 - yAxisTicks[i]);
        GUI_DispStringAt(yAxisLabels[i], 5, 170 - yAxisTicks[i] - 6);
    }

    for (int i = 1; i < dataSize; i++)
    {
        GUI_DrawLine(data[i - 1].time / 2 + 30, 150 - data[i - 1].temperature/2, data[i].time / 2 + 30, 150 - data[i].temperature/2);
    }

    /* Update the display */
    UpdateDisplay(CY_EINK_PARTIAL, true);
}

void ShowGraph_WithUpdate(void)
{
    ClearScreen();
    
    whichData++;
    
    if(whichData == 3){
        whichData = 1;
    }
    
    int dataSize = 0;
    
    if(whichData == 1)
    {
        dataSize = sizeof(reflowData) / sizeof(reflowData[0]);
    }
    else if(whichData == 2)
    {
        dataSize = sizeof(leadData) / sizeof(leadData[0]);
    }
    
    DataPoint data[dataSize];
    
    if(whichData == 1)
    {
        for (int i = 0; i < dataSize; i++) {
            data[i].temperature = reflowData[i].temperature;
            data[i].time = reflowData[i].time;
        }
    }
    else if(whichData == 2)
    {
        for (int i = 0; i < dataSize; i++) {
            data[i].temperature = leadData[i].temperature;
            data[i].time = leadData[i].time;
        }
    }
    
    /* Define axis labels and ticks */
    char xAxisLabels[8][6] = {"0", "60", "120", "180", "240", "300", "360", "420"};
    char yAxisLabels[6][4] = {"0", "60", "120", "180", "240", "300"};
    int xAxisTicks[8] = {30, 60, 90, 120, 150, 180, 210, 240};
    int yAxisTicks[6] = {20, 50, 80, 110, 140, 170};
    
    /* Draw the graph */
    GUI_ClearRect(10, 20, 254, 150);  // Clear the area where the graph will be drawn
    
    /* Draw X and Y axis */
    GUI_DrawLine(30, 20, 30, 200); // Y axis
    GUI_DrawLine(30, 150, 250, 150); // X axis

    /* Draw axis labels and ticks */
    for (int i = 0; i < 8; i++) {
        GUI_DrawLine(xAxisTicks[i], 150, xAxisTicks[i], 155);
        GUI_DispStringAt(xAxisLabels[i], xAxisTicks[i] - 10, 155);
    }

    for (int i = 0; i < 6; i++) {
        GUI_DrawLine(25, 170 - yAxisTicks[i], 30, 170 - yAxisTicks[i]);
        GUI_DispStringAt(yAxisLabels[i], 5, 170 - yAxisTicks[i] - 6);
    }
    
    int min = 0;
    int max = 300;
    
    int randomTemperature1 = 0;
    int randomTemperature2 = 0;

    for(int i = 10; i <= data[dataSize-1].time; i+=10){
        if(Status_SW2_Read() == 0){
            break;
        }
        
        randomTemperature2 = (rand() % (max - min + 1)) + min;
        
        GUI_DrawLine((i-10) / 2 + 30, 150 - randomTemperature1/2, i / 2 + 30, 150 - randomTemperature2/2);
        /* Update the display */
        UpdateDisplay(CY_EINK_PARTIAL, true);
        CyDelay(1000);
        randomTemperature1 = randomTemperature2;
    }
}

void ShowTable(void)
{
    ClearScreen();
    
    /* Set font size, foreground and background colors */
    GUI_SetColor(GUI_BLACK);
    GUI_SetBkColor(GUI_WHITE);
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_SetTextStyle(GUI_TS_NORMAL);

    /* Clear the screen */
    GUI_Clear();
    
    int dataSize = 0;
    
    if(which_data_table == 1)
    {
        dataSize = sizeof(reflowData) / sizeof(reflowData[0]);
    }
    else if(which_data_table == 2)
    {
        dataSize = sizeof(leadData) / sizeof(leadData[0]);
    }
    
    dataSize_global = dataSize;
    
    DataPoint data[dataSize];
    
    char* data_title[2] = {"Reflow Profile", "Lead Profile"};
    
    if(which_data_table == 1)
    {
        for (int i = start_point; i < dataSize; i++) {
            data[i].temperature = reflowData[i].temperature;
            data[i].time = reflowData[i].time;
        }
    }
    else if(which_data_table == 2)
    {
        for (int i = start_point; i < dataSize; i++) {
            data[i].temperature = leadData[i].temperature;
            data[i].time = leadData[i].time;
        }
    }
    
    /* Display page title */
    GUI_SetFont(GUI_FONT_13B_1);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt(data_title[which_data_table-1], 132, 5);
    
    int last_point = start_point + 6;
    
    if(last_point > dataSize)
    {
        last_point = dataSize;
    }
    
    /* Display table data */
    for (int i = start_point; i < last_point; i++)
    {   
        if(i == start_point)
        {
            GUI_DispStringAt("Time (s)", 20, 30);
            GUI_DispStringAt("Temperature (C deg)", 100, 30);
        }
        
        char timeString[10];
        char dataString[10];
        
        sprintf(timeString, "%d", data[i].time);
        sprintf(dataString, "%d", data[i].temperature);
        
        GUI_DispStringAt(timeString, 20, 50 + (i-start_point) * 20);
        GUI_DispStringAt(dataString, 100, 50 + (i-start_point) * 20);
    }

    /* Update the display */
    UpdateDisplay(CY_EINK_PARTIAL, true);
}

int main(void)
{
    
    uint8 pageNumber = 0;
    
    __enable_irq(); /* Enable global interrupts. */
    
    /* Initialize emWin Graphics */
    PWM_Start();
    GUI_Init();

    /* Start the eInk display interface and turn on the display power */
    Cy_EINK_Start(20);
    Cy_EINK_Power(1);
    
    InitGraph();
    
    for(;;)
    {
        if(modeSwitch == 0){
            ShowTable();   
            WaitforSwitchPressAndRelease();
        }
        else {
            ShowGraph_WithUpdate();
            WaitforSwitchPressAndRelease();
        }
    }
}

/* [] END OF FILE */