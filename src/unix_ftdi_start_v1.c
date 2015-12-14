/*
 ============================================================================
 Name        : unix_fd_FTDI_start_v1.c
 Author      : Michał Kobiałka
 Version     :
 Copyright   : All rights reserved
 Description : Komunikacja mikroprocesora z komputerem za pośrednictwem konwertera USB<->RS232 FT232 firmy fd_FTDI.
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <time.h>			// clock()
#include <fcntl.h>			// open()
#include <termios.h>		// struktura terminala
#include <unistd.h>
#include "../inc/string_m.h"


// ==========================================
// STAŁE
#define BAUDRATE B9600
#define MAX_STRING_LENGTH 15000
#define TIMEOUT	3
#define MAX_COMMAND_NR 10


// ==========================================
// ZMIENNE GLOBALNE
int data;
int fd_FTDI;
struct termios tio,backup;
clock_t clkMainTaskStart;		// us
time_t timeTimeoutStart;		// s
char pcTransmitBuff[150];
char pcRecieveBuff[MAX_STRING_LENGTH];
char pcInputBuff[150];
char cCommandCounter, cTempVar;
long int liCharCounter;
char cRecievedValue;
FILE *pDataFile;


// ==========================================
// DEKLARACJE FUNKCJI
int MainTaskWait_ms(clock_t MainTaskPeriod);
int fd_FTDI_Init(void);


// ==========================================
// PĘTLA GŁÓWNA
int main(int argc, char** arg){
	if (fd_FTDI_Init() < 0){
		return(-1);
	}

	pDataFile = fopen("transmisja.txt","w");
	if(pDataFile == NULL){
		printf("\nNie udało się otworzyć pliku dla zapisu danych\n");
		return (-1);
	}
	else{
		printf("\nUdało się otworzyć plik dla zapisu danych\n");
	}
	cCommandCounter = 0;
	while (1){   // KONIEC PRACY PO wpisaniu exit
		clkMainTaskStart = clock();

		fgets(pcInputBuff,MAX_STRING_LENGTH, stdin);
		CopyString(pcInputBuff,pcTransmitBuff);
		if( EQUAL == eCompareString("exit\n",pcTransmitBuff) ) {
			goto EXIT;
		}
		// Wysyłamy każdy znak osobno.
		for(liCharCounter = 0; pcTransmitBuff[liCharCounter] != 0; liCharCounter++ ){};
		write(fd_FTDI,pcTransmitBuff,liCharCounter);

		cRecievedValue = 0;
		liCharCounter = 0;
		timeTimeoutStart = time(NULL);
		do{
			if( 0 < read(fd_FTDI, &cRecievedValue,1) ){
				pcRecieveBuff[liCharCounter] = cRecievedValue;
				liCharCounter++;
			}

		}while( (cRecievedValue != 0x0a) && (time(NULL) < timeTimeoutStart + TIMEOUT) );
		pcRecieveBuff[liCharCounter] = 0;

		// Wypisuje różne zmienna do terminala i pliku.
		printf("\nTxBuff: %s",pcTransmitBuff);
		printf("\nRxBuff: %s",pcRecieveBuff);

		fputs("Tx: ",pDataFile);
		fputs(pcTransmitBuff,pDataFile);
		fputs("Rx: ",pDataFile);
		fputs(pcRecieveBuff,pDataFile);
		fputs("\n\n",pDataFile);
		cCommandCounter++;
	}

	EXIT:
	close(pDataFile);

	if(tcsetattr(fd_FTDI, TCSANOW, &backup) == -1) {
		printf("serial_open(): unable to restore old attributes\n");
		return (-1);
	}
	return 0;
}


// ==========================================
//
int MainTaskWait_ms(clock_t clkMainTaskPeriod_ms){
	clock_t clkMainTaskEnd;

	clkMainTaskEnd = clkMainTaskStart + clkMainTaskPeriod_ms*1000;
	if(clkMainTaskEnd < clock()){
		return (-1);						// czas wykonania pętli głównej 'while' jest wykracza poza zdefiniowany okres.
	}
	else{
		usleep(clkMainTaskEnd-clock());
		return 0;
	}
}

// ==========================================
// INICJALIZACJA urządzenia /dev/ttyUSB
int fd_FTDI_Init(void){
	//First we open the usbdevice (trying to find the chip

	if ( ( fd_FTDI = open( "/dev/ttyUSB0", O_RDWR| O_NOCTTY| O_NDELAY )) < 0 ) {
		fprintf(stderr,"Can't open dev/ttyUSB0\n");
		if ( ( fd_FTDI = open( "/dev/ttyUSB1", O_RDWR| O_NOCTTY| O_NDELAY )) < 0 ) {
			fprintf(stderr,"Can't open dev/ttyUSB1 \n");
			if ( ( fd_FTDI = open( "/dev/FT232", O_RDWR| O_NOCTTY| O_NDELAY )) < 0 ) {
					fprintf(stderr,"Can't open dev/FT232 either\n");
					return(-1);
			}
		}
	}
	if ( tcgetattr( fd_FTDI, &tio ) == -1 )				// zapisujemy ustawienia otwartego portu do struktury tio.
	{
		fprintf(stderr,"Can't properly open\n");
	};
	backup=tio;							// zapisujemy kopię ustawień portu.
	// Setting baudrate
	if(cfsetispeed(&tio, BAUDRATE) == -1) {
		printf("serial_open(): unable to set input-baudrate\n");
		return (-1);
	}
	if(cfsetospeed(&tio, BAUDRATE) == -1) {
		printf("serial_open(): unable to set output-baudrate\n");
		return (-1);
	}

	// Setting the rawmode
	tio.c_lflag &=~(ECHO|ICANON|ISIG|IEXTEN);				// ECHO=0 - echo wyłączone; ICANON=0 - Noncanonical input processing; ISIG=0 - nvm.
	tio.c_lflag  = tio.c_lflag | (FLUSHO);					// FLUSHO - obecnie nie używana.
	tio.c_iflag &=~ (ICRNL|BRKINT|IXON|IXOFF|ISTRIP|INPCK);			// ICRNL=0 - nie przekształcaj znaku CR w NL; BRKINT=0 - nazgłoszenie ^C wysyła sygnał SIGINT; IXON - nvm;
	tio.c_iflag  = tio.c_iflag | (IGNBRK|IMAXBEL|IXANY);
	tio.c_cflag &= ~(CSIZE| PARENB);
	tio.c_cflag  = tio.c_cflag | (CS8 |CREAD| CLOCAL);
	tio.c_oflag &= ~(OPOST);
	tio.c_cc[VMIN]=0;  // Only 1 char to read!
	tio.c_cc[VTIME]=0;

	if (tio.c_lflag & (ECHO|ICANON|ISIG|IEXTEN))
	    printf("serial_open(): unable to set terminalmode -lflag ");
	if ( tio.c_iflag & (ICRNL|BRKINT|IXON|ISTRIP|INPCK))
	     printf("serial_open(): unable to set terminalmode -iflag \n");


	// now clean the  line ...
	if(tcflush(fd_FTDI, TCIOFLUSH) == -1) {
		printf("serial_open(): unable to flush data\n");
		return (-1);
	}
	// now clean the  line ...
	if(tcflow(fd_FTDI, TCIOFLUSH) == -1) {
		printf("serial_open(): unable to flush data\n");
		return (-1);
	}

	// ... and activate the settings for the port
	if(tcsetattr(fd_FTDI, TCSANOW, &tio) == -1) {
		printf("serial_open(): unable to set new attributes\n");
		return (-1);
	}
	printf("Sucessfully opened device for read and write\n");
	return 0;
}
