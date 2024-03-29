/*
 *   SlimProtoLib Copyright (c) 2004,2006 Richard Titmuss
 *   				2008-2011 Ralph Irving
 *
 *   This file is part of SlimProtoLib.
 *
 *   SlimProtoLib is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   SlimProtoLib is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with SlimProtoLib; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef _SQUEEZESLAVE_H_
#define _SQUEEZESLAVE_H_

#if defined(DAEMONIZE) && defined(__WIN32__)
#error "DAEMONIZE not supported on windows version of squeezeslave."
#endif

#if defined(EMPEG) && !defined(RENICE)
#error "RENICE must be defined for empeg version of squeezeslave."
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>

#ifdef DAEMONIZE
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#endif

#include <errno.h>

#ifdef INTERACTIVE
#include <sys/types.h>
#ifdef __WIN32__
#include <sys/fcntl.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#define MSG_DONTWAIT (0)
#else
#include <sys/socket.h>
#endif
#include <sys/time.h>
#include <unistd.h>
#include <locale.h>
#include <ctype.h>
#include <curses.h>
#include <fcntl.h>
#include <lirc/lirc_client.h>
#ifndef __WIN32__
#include <netinet/tcp.h> 
#endif
#endif

#include "slimproto/slimproto.h"
#include "slimaudio/slimaudio.h"

#define RETRY_DEFAULT	5
#define LINE_COUNT	2
#define OPTLEN		96
#define SLIMPROTOCOL_PORT	3483
#ifdef EMPEG
#define PLAYER_TYPE	13
#define FIRMWARE_VERSION	1
#define PA_FRAMES_PER_BUFFER	1152
#define PA_NUM_BUFFERS		0
#else
#define PLAYER_TYPE	8
#define FIRMWARE_VERSION	10
#define PA_FRAMES_PER_BUFFER	(44100/5)	/* XXX FIXME (samplerate/5) */
#define PA_NUM_BUFFERS		3
#endif

int connect_callback(slimproto_t *, bool, void *);
PaDeviceIndex GetAudioDevices(PaDeviceIndex, char*, char*, bool, bool);
int parse_macaddress(char *, const char *);
void print_version(void);
void print_help(void);
void exit_handler(int signal_number);
void restart_handler(int signal_number);

#ifdef DAEMONIZE
void init_daemonize();
void daemonize(char *);
#endif

#define packN4(ptr, off, v) { ptr[off] = (char)(v >> 24) & 0xFF; ptr[off+1] = (v >> 16) & 0xFF; ptr[off+2] = (v >> 8) & 0xFF; ptr[off+3] = v & 0xFF; }
#define packC(ptr, off, v) { ptr[off] = v & 0xFF; }
#define packA4(ptr, off, v) { strncpy((char*)(&ptr[off]), v, 4); }

#ifdef EMPEG
bool empeg_powered(void);
int empeg_getmac(char * mac_addr);
void empeg_init(void);
void close_lcd(void);
long empeg_getkey(void);
long empeg_getircode(long key);
int empeg_idle(void);
void empeg_poweroff(void);
void empeg_geteq_fromfile(void);
void empeg_puteq_tofile(void);
int empeg_vfd_callback(slimproto_t *, const unsigned char *, int, void *);
int empeg_vfdbrt_callback(slimproto_t *, const unsigned char *, int, void *);
int empeg_aude_callback(slimproto_t *, const unsigned char *, int, void *);

struct empeg_state_t
{
   char dummy[8];
   long signature;
   bool power_on;
   char last_server[INET_ADDRSTRLEN];
};
#endif

#ifdef INTERACTIVE
void toggle_handler(int);
int vfd_callback(slimproto_t *, const unsigned char *, int, void *);
void receive_display_data(unsigned short *, int);
bool charisok(unsigned char);
void show_display_buffer(char *);
void makeprintable(unsigned char *);
unsigned char printable(unsigned char);
void set_lcd_line(int, char *, int);
unsigned long getircode(int);
void exitcurses(void);
void initcurses(void);
int read_lirc(void);
void init_lcd (void);
void init_lirc(void);
void send_lcd(char*, int);
bool read_lcd(void);
int setNonblocking(int);
void close_lirc(void);
void close_lcd(void);
#ifdef __WIN32__
int inet_pton (int, const char *, void *);
#endif
#endif

#endif /* _SQUEEZESLAVE_H_ */
