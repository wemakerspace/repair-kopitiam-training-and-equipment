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
LIBS:d2450
LIBS:MiscellaneousDevices
LIBS:m1363
LIBS:short-circuit-limiter-cache
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
L SWITCH_DPST SW?
U 1 1 59215214
P 1950 3500
F 0 "SW?" H 2500 3300 60  0000 C CNN
F 1 "SWITCH_DPST" V 2050 2850 60  0000 C CNN
F 2 "" H 1950 3500 60  0000 C CNN
F 3 "" H 1950 3500 60  0000 C CNN
	1    1950 3500
	0    1    1    0   
$EndComp
$Comp
L SWITCH_DPST SW?
U 2 1 5921527A
P 2150 3500
F 0 "SW?" H 1550 3600 60  0000 C CNN
F 1 "SWITCH_DPST" H 2250 3800 60  0000 C CNN
F 2 "" H 2150 3500 60  0000 C CNN
F 3 "" H 2150 3500 60  0000 C CNN
	2    2150 3500
	0    1    1    0   
$EndComp
$Comp
L D2450 U?
U 1 1 592152E3
P 3500 3600
F 0 "U?" H 3210 3510 50  0000 L CNN
F 1 "D2450" H 3090 3700 50  0000 L CNN
F 2 "SSR" H 3670 3480 50  0001 L CIN
F 3 "" V 3500 3605 50  0001 L CNN
	1    3500 3600
	1    0    0    -1  
$EndComp
$Comp
L Transformer_1P_1S T?
U 1 1 5921548A
P 4300 4050
F 0 "T?" H 4300 4300 50  0000 C CNN
F 1 "Transformer_1P_1S" H 4300 3750 50  0000 C CNN
F 2 "" H 4300 4050 50  0001 C CNN
F 3 "" H 4300 4050 50  0001 C CNN
	1    4300 4050
	0    -1   -1   0   
$EndComp
$Comp
L M1363 P?
U 1 1 5921583E
P 7250 3800
F 0 "P?" H 7250 3570 60  0000 C CNN
F 1 "M1363" H 7380 4030 39  0000 C CNN
F 2 "" H 7250 3770 60  0001 C CNN
F 3 "" H 7250 3770 60  0001 C CNN
	1    7250 3800
	1    0    0    -1  
$EndComp
$Comp
L Buzzer BZ?
U 1 1 595793F2
P 900 5400
F 0 "BZ?" H 1050 5450 50  0000 L CNN
F 1 "Buzzer" H 1050 5350 50  0000 L CNN
F 2 "" V 875 5500 50  0001 C CNN
F 3 "" V 875 5500 50  0001 C CNN
	1    900  5400
	1    0    0    -1  
$EndComp
$EndSCHEMATC
