EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:MiscellaneousDevices
LIBS:high-level-schematic-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L R R?
U 1 1 597467F8
P 5150 2700
F 0 "R?" V 5250 2700 50  0001 C CNN
F 1 "10" V 5150 2700 50  0000 C CNN
F 2 "" V 5080 2700 50  0001 C CNN
F 3 "" H 5150 2700 50  0001 C CNN
	1    5150 2700
	0    1    1    0   
$EndComp
Text GLabel 4300 2900 0    60   Input ~ 0
Live_Input
Text GLabel 5600 2900 2    60   Input ~ 0
Live_Output
$Comp
L SWITCH_DPST SW?
U 1 1 5974691A
P 5200 3100
F 0 "SW?" H 5100 2800 60  0001 C CNN
F 1 "Bypass" H 5200 3000 60  0000 C CNN
F 2 "" H 5200 3100 60  0000 C CNN
F 3 "" H 5200 3100 60  0000 C CNN
	1    5200 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	4750 2900 4900 2900
Wire Wire Line
	4900 2700 4900 2900
Wire Wire Line
	4900 2900 4900 3100
Wire Wire Line
	4900 3100 5000 3100
Wire Wire Line
	5400 3100 5350 3100
Wire Wire Line
	5400 2700 5400 2900
Wire Wire Line
	5400 2900 5400 3100
Wire Wire Line
	5400 2900 5600 2900
Wire Wire Line
	5300 2700 5400 2700
Connection ~ 5400 2900
Wire Wire Line
	5000 2700 4900 2700
Connection ~ 4900 2900
Text Notes 4700 2600 0    60   ~ 0
High Power Resistor
$Comp
L SWITCH_DPST SW?
U 1 1 59746AF0
P 4600 2900
F 0 "SW?" H 4600 3050 60  0001 C CNN
F 1 "MCB" H 4600 2800 60  0000 C CNN
F 2 "" H 4600 2900 60  0000 C CNN
F 3 "" H 4600 2900 60  0000 C CNN
	1    4600 2900
	1    0    0    -1  
$EndComp
Wire Wire Line
	4300 2900 4400 2900
$EndSCHEMATC
