/*
 ============================================================================
 Name        : unix_ftdi_start_v1.c
 Author      : Michał Kobiałka
 Version     :
 Copyright   : Your copyright notice
 Description : Komunikacja mikroprocesora z komputerem za pośrednictwem konwertera USB<->RS232 FT232 firmy FTDI.
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <time.h>			// clock()
#include <fcntl.h>			// open()
#include <termios.h>		// struktura terminala
#include <unistd.h>



// STAŁE
#define BAUDRATE B9600



// ZMIENNE GLOBALNE
int data;
int fd;
char a=1;
int rec;
float frec;
unsigned char r,s;
struct termios tio,backup;
clock_t clkMainTaskStartClock;



// DEKLARACJE FUNKCJI
void Delay_us( clock_t clkMikroSek );
void Delay_ms( clock_t clkMiliSek );
int MainTaskWait_ms(clock_t MainTaskPeriod);
int FTDI_Init(void);





// PĘTLA GŁÓWNA
int main(int argc, char** arg){
	int iMeasureCounter = 0;
	FILE *pDataFile;
	clock_t clkClockTemp;


	if (FTDI_Init() < 0){
		return(-1);
	}

	pDataFile = fopen("dane.csv","w");
	if(pDataFile == NULL){;
		printf("\nNie udało się otworzyć pliku dla zapisu danych\n");
		return (-1);
	}
	else{
		printf("\nUdało się otworzyć plik dla zapisu danych\n");
	}
	data=0;
	rec=0;

	printf("\nPomiar napięcie z czujnika przyśpieszenia.\n");
	printf("   x,y,z - wybór kanałów\n");
	printf("   q - zakończenie pracy");

	clkClockTemp = clock();
	while ( (1 == (100 > iMeasureCounter)) && (a != 'q') ){
		clkMainTaskStartClock = clock();

		a=getc(stdin);

		write(fd,&a,1);
		if(0 > MainTaskWait_ms(5)){
					printf("\n BŁĄD: Czas wykonywania pętli głównej jest większy od okresu zdefiniowanego w arumencie funkcji 'MainTaskWait_ms()'. \n");
					return (-1);
				}
		if((-1) != read(fd,&rec,1)){
			printf("rec =  %c\n",a);
			printf("rec =  %c\n",rec);

			iMeasureCounter++;
		}


	}
	close(pDataFile);


	if(tcsetattr(fd, TCSANOW, &backup) == -1) {
		printf("serial_open(): unable to restore old attributes\n");
		return (-1);
	}
	return 0;
}

// INICJALIZACJA urządzenia /dev/ttyUSB
int FTDI_Init(void){
	//First we open the usbdevice (trying to find the chip

	if ( ( fd = open( "/dev/ttyUSB0", O_RDWR| O_NOCTTY| O_NDELAY )) < 0 ) {
		fprintf(stderr,"Can't open dev/ttyUSB0\n");
		if ( ( fd = open( "/dev/ttyUSB1", O_RDWR| O_NOCTTY| O_NDELAY )) < 0 ) {
			fprintf(stderr,"Can't open dev/ttyUSB1 either\n");
			return(-1);
		}
	}
	if ( tcgetattr( fd, &tio ) == -1 )				// zapisujemy ustawienia otwartego portu do struktury tio.
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
	//tio.c_lflag = tio.c_lflag | ICANON;
	tio.c_lflag  = tio.c_lflag | (FLUSHO);					// FLUSHO - obecnie nie używana.
	tio.c_iflag &=~ (ICRNL|BRKINT|IXON|IXOFF|ISTRIP|INPCK);			// ICRNL=0 - nie przekształcaj znaku CR w NL; BRKINT=0 - nazgłoszenie ^C wysyła sygnał SIGINT; IXON - nvm;
	tio.c_iflag  = tio.c_iflag | (IGNBRK|IMAXBEL|IXANY);
	tio.c_cflag &= ~(CSIZE| PARENB);
	tio.c_cflag  = tio.c_cflag | (CS8 |CREAD| CLOCAL);
	tio.c_oflag &= ~(OPOST);
	tio.c_cc[VMIN]=2;  // Only 1 char to read!
	tio.c_cc[VTIME]=0;

	if (tio.c_lflag & (ECHO|ICANON|ISIG|IEXTEN))
	    printf("serial_open(): unable to set terminalmode -lflag ");
	if ( tio.c_iflag & (ICRNL|BRKINT|IXON|ISTRIP|INPCK))
	     printf("serial_open(): unable to set terminalmode -iflag \n");


	// now clean the  line ...
	if(tcflush(fd, TCIOFLUSH) == -1) {
		printf("serial_open(): unable to flush data\n");
		return (-1);
	}
	// now clean the  line ...
	if(tcflow(fd, TCIOFLUSH) == -1) {
		printf("serial_open(): unable to flush data\n");
		return (-1);
	}

	// ... and activate the settings for the port
	if(tcsetattr(fd, TCSANOW, &tio) == -1) {
		printf("serial_open(): unable to set new attributes\n");
		return (-1);
	}
	printf("Sucessfully opened device for read and write\n");
	return 0;
}


//  CZEKANIE W PĘTLI OKREŚLONĄ ILOŚĆ MILISEKUND ALBO MIKROSEKUND  //
void Delay_us( clock_t clkMikroSek ){
   clock_t clkClockTemp;

   clkClockTemp = clock() + clkMikroSek;
   while( clkClockTemp > clock()){};
}

void Delay_ms( clock_t clkMiliSek ){
   clock_t  clkClockTemp;

   clkClockTemp = clock() + clkMiliSek*1000;
   while( clkClockTemp > clock()){};
}

int MainTaskWait_ms(clock_t clkMainTaskPeriod_ms){
	clock_t clkMainTaskEndClock;

	clkMainTaskEndClock = clkMainTaskStartClock + clkMainTaskPeriod_ms*1000;
	if(clkMainTaskEndClock < clock()){
		return (-1);						// czas wykonania pętli głównej 'while' jest wykracza poza zdefiniowany okres.
	}
	else{
		while(clkMainTaskEndClock > clock()){};
		return 0;
	}
}
