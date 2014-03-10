#define __AVR__ 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "sim_avr.h"
#include "sim_elf.h"
#include "sim_vcd_file.h"
#include "avr_mcu_section.h"
#include "avr_ioport.h"

#define F_CPU 16000000

/*
 * /home/glecuyer/Packages/simavr-simavr/simavr/sim/sim_core.c patched !
 * extern void avr_core_custum_watch_write(avr_t *avr, uint16_t addr, uint8_t v) __attribute__((weak)); 
 Line 118:
 * 	if ( avr_core_custum_watch_write )
		avr_core_custum_watch_write(avr, addr, v);
 * 
 * See /usr/lib/avr/include/avr/iom328p.h
 */

//void avr_core_custum_watch_write(avr_t *avr, uint16_t addr, uint8_t v) __attribute__((weak))
void avr_core_custum_watch_write(avr_t *avr, uint16_t addr, uint8_t v)
{
	//printf( "[%04X]=%02X\n", (int)addr, (int)v );
	if ( (int)addr == 0x00C6 )  // UDR0
	{
		//printf( "[%04X]=%02X\n", (int)addr, (int)v );
		printf( "%c", (int)v );
	}
	
	//if ( (int)addr == 0x002B )  // PORTD
	//{	
	//	printf( "[%04X]=%02X\n", (int)addr, (int)v );
	//}
}


avr_vcd_t vcd_file;
avr_t * avr;

void sig_int( int sign )
{
	printf("signal caught, simavr terminating\n");
	if (avr)
		avr_terminate(avr);
	exit(0);
}


int main( int argc, char * argv[] )
{
	extern void foo() __attribute__((weak)); 
	
	int state;
	elf_firmware_t f;
	char * fname = "../ServoVanne/ServoVanne.elf";

	if ( argc > 1 )
	{
		fname = argv[1];
	}


	printf( "Simulating %s\n", fname );
	
	elf_read_firmware(fname, &f);
	strcpy(f.mmcu, "atmega328p");
	f.frequency = F_CPU;

	avr = avr_make_mcu_by_name(f.mmcu);
	avr_init(avr);
	avr_load_firmware(avr, &f);

	if ( argc < 3 )
	{
		avr_vcd_init(avr, "portb_output.vcd", &vcd_file, 100);

		avr_vcd_add_signal(&vcd_file,
		avr_io_getirq(
			avr, AVR_IOCTL_IOPORT_GETIRQ('B'),
			IOPORT_IRQ_PIN_ALL),
			8,
			"portb"
		);

		avr_vcd_start(&vcd_file);
		while(avr->cycle < 1000000000)
			state = avr_run(avr);

		avr_vcd_stop(&vcd_file);
		avr_vcd_close(&vcd_file);
	}
	else
	{

		avr->gdb_port = 1234;
		avr->state = cpu_Stopped;
		avr_gdb_init(avr);

		signal(SIGINT, sig_int);
		signal(SIGTERM, sig_int);

		for (;;) 
		{
			int state = avr_run(avr);
			if ( state == cpu_Done || state == cpu_Crashed)
				break;
		}

	}

	avr_terminate(avr);

	return 0;
}
