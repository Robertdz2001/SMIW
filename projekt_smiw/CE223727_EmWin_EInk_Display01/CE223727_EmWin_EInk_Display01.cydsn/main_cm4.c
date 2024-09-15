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

#define TIMER_PERIOD   1000000U

//maksymalna liczba punktow w tabeli.
#define MAX_POINTS 100


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

//zmienna do sterowania pwm
int compareValue = 50;

//ktora tabela zostala wybrana.
int which_data_table = 0;

//wybor trybu.
int modeSwitch = 0;

//struktura do przechowywania danych o tabeli.
typedef struct {
    int time;
    int temperature;
} DataPoint;

typedef struct {
    DataPoint data[MAX_POINTS]; //tablica z punktami.
    int dataSize;    // Rozmiar tej tablicy
    char* name; //Nazwa tablicy.
} DataSet;

int dataSize = 6;
    
DataPoint data[6];

//zmienna przechowujaca tablice z danymi.
DataSet table_data[3] = {
    {
        .data = {{0, 25}, {90, 90}, {180, 130}, {210, 138}, {240, 165}, {270, 138}},
        .dataSize = 6,
        .name = "Reflow Sn42/Bi57.6/Ag0.4"
    },
    {
        .data = {{0, 25}, {30, 100}, {120, 150}, {150, 183}, {210, 235}, {240, 183}},
        .dataSize = 6,
        .name = "Reflow Sn63/Pb37 T4"
    },
    {
        .data = {{0, 25}, {90, 90}, {180, 130}, {210, 170}, {240, 197}, {270, 170}},
        .dataSize = 6,
        .name = "Reflow Sn60/Bi40 T4"
    }
};

//Funkcja do zmiany trybów uzywając przycisków.
void WaitforSwitchPressAndRelease(void)
{   
    int table_count = sizeof(table_data) / sizeof(table_data[0]);
    
    Cy_TCPWM_PWM_SetCompare0(PWM_HW, PWM_CNT_NUM, compareValue);
    
    /* Oczekiwanie na wciśnięcie */
    while(Status_Button1_Read() != 0 && Status_Button2_Read() != 0 && Status_Button3_Read() != 0);
    
    /* Oczekiwanie na puszczenie */
    while(Status_Button1_Read() == 0)
    {
        Cy_TCPWM_PWM_SetCompare0(PWM_HW, PWM_CNT_NUM, compareValue);
        compareValue = (compareValue + 1) % 100;
        CyDelay(20);
    };
    
    //Wcisniety drugi przycisk
    if(Status_Button2_Read() == 0)
    {
        modeSwitch = 1;
        return;
    }
    
    //Wcisniety trzeci przycisk
    if(Status_Button3_Read() == 0)
    {
        modeSwitch = 3;
        return;
    }
    
    if(modeSwitch == 1 || modeSwitch == 3)
    {
        modeSwitch = 0;
    }
    else
    {
        which_data_table++;
        if(which_data_table == 3)
        {
            modeSwitch = 2;   
        }
    }
}

//czy program (rysowanie wykresu) zostal uruchomiony.
bool programStarted = false;

//ile trwa program dla danej tabeli.
int maxTimeInGraph = 0;

void ShowGraph_WithUpdate(void)
{
    ClearScreen();
    
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
    
    maxTimeInGraph = data[dataSize-1].time;
    UpdateDisplay(CY_EINK_PARTIAL, true);
    programStarted = true;
}

void ShowGraph(void) { 
    ClearScreen();
    
    /* Set font size, foreground and background colors */
    GUI_SetColor(GUI_BLACK);
    GUI_SetBkColor(GUI_WHITE);
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_SetTextStyle(GUI_TS_NORMAL);
    
    char xAxisLabels[8][6] = {"0", "60", "120", "180", "240", "300", "360", "420"};
    char yAxisLabels[6][4] = {"0", "60", "120", "180", "240", "300"};
    int xAxisTicks[8] = {30, 60, 90, 120, 150, 180, 210, 240};
    int yAxisTicks[6] = {20, 50, 80, 110, 140, 170};
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
    
    for (int i = 0; i < dataSize; i++) {
        data[i].temperature = table_data[which_data_table].data[i].temperature;
        data[i].time = table_data[which_data_table].data[i].time;
    }
    
    /* Display page title */
    GUI_SetFont(GUI_FONT_13B_1);
    GUI_SetTextAlign(GUI_TA_HCENTER);
    GUI_DispStringAt(table_data[which_data_table].name, 132, 5);
    
    /* Display table data */
    for (int i = 0; i < dataSize; i++)
    {           
        if(i == 0)
        {
            GUI_DispStringAt("Time (s)", 20, 30);
            GUI_DispStringAt("Temperature (C deg)", 100, 30);
            continue;
        }
        
        char timeString[10];
        char dataString[10];
        
        sprintf(timeString, "%d", data[i].time);
        sprintf(dataString, "%d", data[i].temperature);
        
        GUI_DispStringAt(timeString, 20, 50 + (i-1) * 20);
        GUI_DispStringAt(dataString, 100, 50 + (i-1) * 20);
    }

    /* Update the display */
    UpdateDisplay(CY_EINK_PARTIAL, true);
}

void AddNew()
{
    ClearScreen();
    
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
    GUI_DispStringAt("Add temperature points", 132, 5);
    
    int currentTemp = 25; // Początkowa wartość temperatury
    int currentTime = 0; // Początkowa wartość czasu
    char tempBuffer[32];   // Bufor na string wyświetlający temperaturę
    char timeBuffer[32];   // Bufor na string wyświetlający czas
    int currentIndex = 1;  // Bieżący indeks w tablicy data[]
    
    data[0].temperature = currentTemp;
    data[0].time = currentTime;
    
    // Wyświetlenie początkowej temperatury
    sprintf(tempBuffer, "Current Temperature: %dC", currentTemp);
    GUI_DispStringAt(tempBuffer, 10, 30);
    
    UpdateDisplay(CY_EINK_PARTIAL, true);

    // Pętla do wprowadzania wartości
    while (currentIndex < 6)
    {
        bool btn1_Pressed = false;
        bool btn3_Pressed = false;

        // Sprawdzanie stanu przycisków w pętli
        while (Status_Button1_Read() != 0 && Status_Button2_Read() != 0 && Status_Button3_Read() != 0); // Czekaj, aż przyciski zostaną wciśnięte
        
        // Sprawdzenie czy Button1 lub Button2 został wciśnięty
        while (Status_Button1_Read() == 0 || Status_Button2_Read() == 0)
        {
            if(Status_Button1_Read() == 0){
                currentTemp++;
            }
            else {
                currentTemp += 10;
            }
            
            if(currentTemp >= 300) {
                currentTemp = 25;
            }

            // Aktualizacja wyświetlanej wartości temperatury
            GUI_ClearRect(10, 30, 200, 50);  // Wyczyść poprzednią wartość temperatury
            sprintf(tempBuffer, "Current Temperature: %dC", currentTemp);
            GUI_DispStringAt(tempBuffer, 10, 30);  // Wyświetlenie nowej wartości temperatury
            UpdateDisplay(CY_EINK_PARTIAL, true);
        }
        
        // Sprawdzenie czy Button3 został wciśnięty
        while (Status_Button3_Read() == 0)
        {
            btn3_Pressed = true;  // Przycisk został wciśnięty
        }

        // Jeśli Button3 został wciśnięty i puszczony, dodaj temperaturę do tablicy
        if (btn3_Pressed)
        {
            data[currentIndex].temperature = currentTemp; // Zapis temperatury do tablicy

            // Wyświetlenie informacji o dodanej temperaturze
            sprintf(tempBuffer, "Added Temp %d: %dC", currentIndex, currentTemp);
            GUI_DispStringAt(tempBuffer, 10, 50 + (currentIndex * 20)); // Wyświetlenie na ekranie w różnych miejscach

            currentIndex++;  // Przejdź do kolejnego indeksu tablicy

            // Resetowanie temperatury do 25, jeśli chcesz, by po dodaniu każdej temperatury zaczynało się od nowa
            currentTemp = 25;

            // Aktualizacja wyświetlanej wartości temperatury
            GUI_ClearRect(10, 30, 200, 50);  // Wyczyść poprzednią wartość temperatury
            sprintf(tempBuffer, "Current Temperature: %dC", currentTemp);
            GUI_DispStringAt(tempBuffer, 10, 30);  // Wyświetlenie nowej wartości temperatury
            UpdateDisplay(CY_EINK_PARTIAL, true);
        }
    }
    
    ClearScreen();
    
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
    GUI_DispStringAt("Add time points", 132, 5);
    
    sprintf(timeBuffer, "Current Time: %ds", currentTime + 1);
    GUI_DispStringAt(timeBuffer, 10, 30);
    UpdateDisplay(CY_EINK_PARTIAL, true);
    
    currentIndex = 1;
    int lastCurrentTime = currentTime;
    
    while (currentIndex < 6)
    {
        bool btn1_Pressed = false;
        bool btn3_Pressed = false;
        
        if(currentTime == lastCurrentTime) {
            currentTime = lastCurrentTime + 1;
        }

        // Sprawdzanie stanu przycisków w pętli
        while (Status_Button1_Read() != 0 && Status_Button2_Read() != 0 && Status_Button3_Read() != 0); // Czekaj, aż przyciski zostaną wciśnięte
        
        // Sprawdzenie czy Button1 lub Button2 został wciśnięty
        while (Status_Button1_Read() == 0 || Status_Button2_Read() == 0)
        {
            if(Status_Button1_Read() == 0){
                currentTime++;
            }
            else {
                currentTime += 10;
            }
            
            if(currentTime >= 400) {
                currentTime = lastCurrentTime + 1;
            }

            // Aktualizacja wyświetlanej wartości czasu
            GUI_ClearRect(10, 30, 200, 50);  // Wyczyść poprzednią wartość czasu
            sprintf(timeBuffer, "Current Time: %ds", currentTime);
            GUI_DispStringAt(timeBuffer, 10, 30);  // Wyświetlenie nowej wartości czasu
            UpdateDisplay(CY_EINK_PARTIAL, true);
        }
        
        // Sprawdzenie czy Button3 został wciśnięty
        while (Status_Button3_Read() == 0)
        {
            btn3_Pressed = true;  // Przycisk został wciśnięty
        }

        // Jeśli Button3 został wciśnięty i puszczony, dodaj czas do tablicy
        if (btn3_Pressed)
        {
            data[currentIndex].time = currentTime;
            lastCurrentTime = currentTime;

            // Wyświetlenie informacji o dodanym czasie
            sprintf(timeBuffer, "Added Time %d: %ds", currentIndex, currentTime);
            GUI_DispStringAt(timeBuffer, 10, 50 + (currentIndex * 20)); // Wyświetlenie na ekranie w różnych miejscach

            currentIndex++;  // Przejdź do kolejnego indeksu tablicy

            // Aktualizacja wyświetlanej wartości czasu
            GUI_ClearRect(10, 30, 200, 50);  // Wyczyść poprzednią wartość czasu
            sprintf(timeBuffer, "Current Time: %ds", currentTime);
            GUI_DispStringAt(timeBuffer, 10, 30);  // Wyświetlenie nowej wartości czasu
            UpdateDisplay(CY_EINK_PARTIAL, true);
        }
    }
    
    ClearScreen();
    
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
    GUI_DispStringAt("Click one to break, click two to start.", 110, 5);
    GUI_DispStringAt("Click three to show graph.", 10, 25);
    UpdateDisplay(CY_EINK_PARTIAL, true);
    
    while(Status_Button1_Read() != 0 && Status_Button2_Read() != 0 && Status_Button3_Read() != 0);
    
    bool btn1_Pressed = false;
    bool btn2_Pressed = false;
    bool btn3_Pressed = false;
    
    while(Status_Button1_Read() == 0)
    {
        btn1_Pressed = true;
    }
    while (Status_Button2_Read() == 0)
    {
        btn2_Pressed = true;
    }
    while (Status_Button3_Read() == 0)
    {
        btn3_Pressed = true;
    }
    
    if(btn1_Pressed)
    {
        modeSwitch = 0;
        which_data_table = 0;
    }
    else if(btn2_Pressed)
    {
        modeSwitch = 1;
        which_data_table = 0;
    }
    else if(btn3_Pressed)
    {
        modeSwitch = 3;
        which_data_table = 0;
    }
}


void ShowAddNewPage(void)
{
    ClearScreen();
    
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
    GUI_DispStringAt("Users Profile - click one to add", 132, 5);

    /* Update the display */
    UpdateDisplay(CY_EINK_PARTIAL, true);
    
    while(Status_Button1_Read() != 0 && Status_Button2_Read() != 0);
    
    bool btn1_Pressed = false;
    bool btn2_Pressed = false;
    
    while(Status_Button1_Read() == 0)
    {
        btn1_Pressed = true;
    }
    while (Status_Button2_Read() == 0)
    {
        btn2_Pressed = true;
    }
    
    if(btn1_Pressed)
    {
        AddNew();
    }
    else if(btn2_Pressed)
    {
        modeSwitch = 0;
        which_data_table = 0;
    }
}

void TimerInterruptHandler(void)
{
    /* Clear the terminal count interrupt */
    Cy_TCPWM_ClearInterrupt(Timer_HW, Timer_CNT_NUM, CY_TCPWM_INT_ON_TC);
    static int counter = 0;
    
    if(programStarted)
    {
        int min = 0;
        int max = 300;
        const int seconds = 10;
        static int pointCounter = seconds;
    
        static int randomTemperature1 = 0;
        static int randomTemperature2 = 0;
            
        randomTemperature2 = (rand() % (max - min + 1)) + min;
            
        GUI_DrawLine((pointCounter-seconds) / 2 + 30, 150 - randomTemperature1/2, pointCounter / 2 + 30, 150 - randomTemperature2/2);

        //if(counter++ == 10){
            UpdateDisplay(CY_EINK_PARTIAL, true);
        //    counter = 0;
        //}
        randomTemperature1 = randomTemperature2;
        pointCounter+=seconds;
        
        if(Status_Button1_Read() == 0 || pointCounter >= maxTimeInGraph){
            programStarted = false;
            pointCounter = seconds;
            randomTemperature1 = 0;
            randomTemperature2 = 0;
            which_data_table = 0;
        }
    }
}

void Init_Interrupts()
{
    /* Initialize the interrupt vector table with the timer interrupt handler
    * address and assign priority. */
    Cy_SysInt_Init(&isrTimer_cfg, TimerInterruptHandler);
    NVIC_ClearPendingIRQ(isrTimer_cfg.intrSrc);/* Clears the interrupt */
    NVIC_EnableIRQ(isrTimer_cfg.intrSrc); /* Enable the core interrupt */
    __enable_irq(); /* Enable global interrupts. */
    
    /* Start the TCPWM component in timer/counter mode. The return value of the
     * function indicates whether the arguments are valid or not. It is not used
     * here for simplicity. */
    (void)Cy_TCPWM_Counter_Init(Timer_HW, Timer_CNT_NUM, &Timer_config);
    Cy_TCPWM_Enable_Multiple(Timer_HW, Timer_CNT_MASK); /* Enable the counter instance */
    
    /* Set the timer period in milliseconds. To count N cycles, period should be
     * set to N-1. */
    Cy_TCPWM_Counter_SetPeriod(Timer_HW, Timer_CNT_NUM, TIMER_PERIOD - 1);
    
    /* Trigger a software reload on the counter instance. This is required when 
     * no other hardware input signal is connected to the component to act as
     * a trigger source. */
    Cy_TCPWM_TriggerReloadOrIndex(Timer_HW, Timer_CNT_MASK); 
}

int main(void)
{
    Init_Interrupts();
    
    PWM_Start();
    /* Initialize emWin Graphics */
    GUI_Init();

    /* Start the eInk display interface and turn on the display power */
    Cy_EINK_Start(20);
    Cy_EINK_Power(1);
    
    for(;;)
    {
        if(!programStarted) {
            if(modeSwitch == 0){
                ShowTable();   
                WaitforSwitchPressAndRelease();
            }
            else if (modeSwitch == 1){
                ShowGraph_WithUpdate();
                WaitforSwitchPressAndRelease();
            }
            else if (modeSwitch == 2){
                ShowAddNewPage();
            }
            else if (modeSwitch == 3) {
                ShowGraph();
                WaitforSwitchPressAndRelease();   
            }
        }
    }
}

/* [] END OF FILE */