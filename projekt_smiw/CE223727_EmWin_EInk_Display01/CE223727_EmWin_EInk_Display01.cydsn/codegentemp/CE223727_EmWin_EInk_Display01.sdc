# THIS FILE IS AUTOMATICALLY GENERATED
# Project: C:\Users\rober\Desktop\SMIW_projekt\projekt_smiw\CE223727_EmWin_EInk_Display01\CE223727_EmWin_EInk_Display01.cydsn\CE223727_EmWin_EInk_Display01.cyprj
# Date: Wed, 28 Aug 2024 12:39:39 GMT
#set_units -time ns
create_clock -name {CyILO} -period 31250 -waveform {0 15625} [list [get_pins {ClockBlock/ilo}]]
create_clock -name {CyClk_LF} -period 31250 -waveform {0 15625} [list [get_pins {ClockBlock/lfclk}]]
create_clock -name {CyFLL} -period 10 -waveform {0 5} [list [get_pins {ClockBlock/fll}]]
create_clock -name {CyClk_HF0} -period 10 -waveform {0 5} [list [get_pins {ClockBlock/hfclk_0}]]
create_clock -name {CyClk_Fast} -period 10 -waveform {0 5} [list [get_pins {ClockBlock/fastclk}]]
create_clock -name {CyClk_Peri} -period 10 -waveform {0 5} [list [get_pins {ClockBlock/periclk}]]
create_generated_clock -name {CyClk_Slow} -source [get_pins {ClockBlock/periclk}] -edges {1 3 5} [list [get_pins {ClockBlock/slowclk}]]
create_generated_clock -name {EINK_Clock} -source [get_pins {ClockBlock/periclk}] -edges {1 50001 100001} [list [get_pins {ClockBlock/ff_div_11}]]
create_generated_clock -name {Clock_3} -source [get_pins {ClockBlock/periclk}] -edges {1 101 201} [list [get_pins {ClockBlock/ff_div_12}]]
create_generated_clock -name {Clock_1} -source [get_pins {ClockBlock/periclk}] -edges {1 101 201} [list [get_pins {ClockBlock/ff_div_13}]]
create_generated_clock -name {CY_EINK_SPIM_SCBCLK} -source [get_pins {ClockBlock/periclk}] -edges {1 2 3} [list [get_pins {ClockBlock/ff_div_6}]]
create_clock -name {CyPeriClk_App} -period 10 -waveform {0 5} [list [get_pins {ClockBlock/periclk_App}]]
create_clock -name {CyIMO} -period 125 -waveform {0 62.5} [list [get_pins {ClockBlock/imo}]]


# Component constraints for C:\Users\rober\Desktop\SMIW_projekt\projekt_smiw\CE223727_EmWin_EInk_Display01\CE223727_EmWin_EInk_Display01.cydsn\TopDesign\TopDesign.cysch
# Project: C:\Users\rober\Desktop\SMIW_projekt\projekt_smiw\CE223727_EmWin_EInk_Display01\CE223727_EmWin_EInk_Display01.cydsn\CE223727_EmWin_EInk_Display01.cyprj
# Date: Wed, 28 Aug 2024 12:39:23 GMT
