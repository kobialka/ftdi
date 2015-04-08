/*
 ============================================================================
 Name        : unix_fd_FTDI_start_v1.c
 Author      : Michał Kobiałka
 Version     :
 Copyright   : Your copyright notice
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


// STAŁE
#define BAUDRATE B9600
#define MAX_STRING_LENGTH 50


// ZMIENNE GLOBALNE
int data;
int fd_FTDI;
struct termios tio,backup;
clock_t clkMainTaskStart;
char pcTransmitBuff[MAX_STRING_LENGTH];
char pcRecieveBuff[MAX_STRING_LENGTH];
char pcInputBuff[MAX_STRING_LENGTH];
char cCommandCounter;
char cCharCounter;
FILE *pDataFile;


// DEKLARACJE FUNKCJI
int MainTaskWait_ms(clock_t MainTaskPeriod);
int fd_FTDI_Init(void);





// PĘTLA GŁÓWNA
int main(int argc, char** arg){
	if (fd_FTDI_Init() < 0){
		return(-1);
	}

	pDataFile = fopen("transmisja.txt","w");
	if(pDataFile == NULL){;
		printf("\nNie udało się otworzyć pliku dla zapisu danych\n");
		return (-1);
	}
	else{
		printf("\nUdało się otworzyć plik dla zapisu danych\n");
	}
	cCommandCounter = 0;

	while ( cCommandCounter != 6){		// kończy pracę po wysłaniu cCommandCounter stringów.
		long int read_result = 0;
		clkMainTaskStart = clock();

		// Narazie czyścimy RxBuff w pętli
		for(cCharCounter = 0; cCharCounter < MAX_STRING_LENGTH; cCharCounter++){
			pcRecieveBuff[cCharCounter] = 0x00;
		}


		fgets(pcInputBuff,MAX_STRING_LENGTH, stdin);
		ReplaceCharactersInString(pcInputBuff,'\n',0x00);
		CopyString(pcInputBuff,pcTransmitBuff);

		// Wysyłamy każdy znak osobno.
		for(cCharCounter = 0; pcTransmitBuff[cCharCounter] != 0x00; cCharCounter++ ){
			write(fd_FTDI,pcTransmitBuff,1);
		}

		MainTaskWait_ms(300);

		// Trzeba zrobić poprawny odczyt.
		read_result = read(fd_FTDI, pcRecieveBuff,MAX_STRING_LENGTH);


		// Wypisuje różne zmienna do terminala i pliku.
		printf("\nTxBuff: %s",pcTransmitBuff);
		printf("\nRxBuff: %s",pcRecieveBuff);
		printf("\nread_resx: %x",read_result);
		fputs("\n|start|",pDataFile);
		fputs(pcTransmitBuff,pDataFile);
		fputs("|stop|",pDataFile);
		cCommandCounter++;
	}
	close(pDataFile);
	if(tcsetattr(fd_FTDI, TCSANOW, &backup) == -1) {
		printf("serial_open(): unable to restore old attributes\n");
		return (-1);
	}
	return 0;
}


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


// INICJALIZACJA urządzenia /dev/ttyUSB
int fd_FTDI_Init(void){
	//First we open the usbdevice (trying to find the chip

	if ( ( fd_FTDI = open( "/dev/ttyUSB0", O_RDWR| O_NOCTTY| O_NDELAY )) < 0 ) {
		fprintf(stderr,"Can't open dev/ttyUSB0\n");
		if ( ( fd_FTDI = open( "/dev/ttyUSB1", O_RDWR| O_NOCTTY| O_NDELAY )) < 0 ) {
			fprintf(stderr,"Can't open dev/ttyUSB1 either\n");
			return(-1);
		}
	}
	if ( tcgetattr( fd_FTDI, &tio ) == -1 )				// zapisujemy ustawienia otwartego portu do struktury tio.
	{
		fprintf(stderr,"Can't properly open dev/ttyUSB0\n");
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
