/*
 *   SlimProtoLib Copyright (c) 2004,2006 Richard Titmuss
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

#include "squeezeslave.h"

extern char* version;
extern int revision;

int parse_macaddress(char *macaddress, const char *str) {
	char *ptr;
	int i;
	
	for (i=0; i<6; i++) {		
		macaddress[i] = (char) strtol (str, &ptr, 16);
		if (i == 5 && *ptr == '\0')
			return 0;
		if (*ptr != ':')
			return -1;
			
		str = ptr+1;
	}	
	return -1;
}

#ifdef __WIN32__
#ifndef EAFNOSUPPORT
# define EAFNOSUPPORT EINVAL
#endif

#ifndef NS_INADDRSZ
# define NS_INADDRSZ      4
#endif
#ifndef NS_IN6ADDRSZ
# define NS_IN6ADDRSZ    16
#endif
#ifndef NS_INT16SZ 
# define NS_INT16SZ      2
#endif

/*
 * WARNING: Don't even consider trying to compile this on a system where
 * sizeof(int) < 4.  sizeof(int) > 4 is fine; all the world's not a VAX.
 */

static int inet_pton4 (const char *src, unsigned char *dst);
#if HAVE_IPV6
static int inet_pton6 (const char *src, unsigned char *dst);
#endif

/* int
 * inet_pton(af, src, dst)
 *	convert from presentation format (which usually means ASCII printable)
 *	to network format (which is usually some kind of binary format).
 * return:
 *	1 if the address was valid for the specified address family
 *	0 if the address wasn't valid (`dst' is untouched in this case)
 *	-1 if some other error occurred (`dst' is untouched in this case, too)
 * author:
 *	Paul Vixie, 1996.
 */
int inet_pton (int af, const char *src, void *dst)
{
  switch (af)
    {
    case AF_INET:
      return (inet_pton4 (src, dst));

#if HAVE_IPV6
    case AF_INET6:
      return (inet_pton6 (src, dst));
#endif

    default:
      errno = EAFNOSUPPORT;
      return (-1);
    }
  /* NOTREACHED */
}

/* int
 * inet_pton4(src, dst)
 *	like inet_aton() but without all the hexadecimal, octal (with the
 *	exception of 0) and shorthand.
 * return:
 *	1 if `src' is a valid dotted quad, else 0.
 * notice:
 *	does not touch `dst' unless it's returning 1.
 * author:
 *	Paul Vixie, 1996.
 */
static int
inet_pton4 (const char *src, unsigned char *dst)
{
  int saw_digit, octets, ch;
  unsigned char tmp[NS_INADDRSZ], *tp;

  saw_digit = 0;
  octets = 0;
  *(tp = tmp) = 0;
  while ((ch = *src++) != '\0')
    {

      if (ch >= '0' && ch <= '9')
	{
	  unsigned new = *tp * 10 + (ch - '0');

	  if (saw_digit && *tp == 0)
	    return (0);
	  if (new > 255)
	    return (0);
	  *tp = new;
	  if (!saw_digit)
	    {
	      if (++octets > 4)
		return (0);
	      saw_digit = 1;
	    }
	}
      else if (ch == '.' && saw_digit)
	{
	  if (octets == 4)
	    return (0);
	  *++tp = 0;
	  saw_digit = 0;
	}
      else
	return (0);
    }
  if (octets < 4)
    return (0);
  memcpy (dst, tmp, NS_INADDRSZ);
  return (1);
}

#if HAVE_IPV6

/* int
 * inet_pton6(src, dst)
 *	convert presentation level address to network order binary form.
 * return:
 *	1 if `src' is a valid [RFC1884 2.2] address, else 0.
 * notice:
 *	(1) does not touch `dst' unless it's returning 1.
 *	(2) :: in a full address is silently ignored.
 * credit:
 *	inspired by Mark Andrews.
 * author:
 *	Paul Vixie, 1996.
 */
static int
inet_pton6 (const char *src, unsigned char *dst)
{
  static const char xdigits[] = "0123456789abcdef";
  unsigned char tmp[NS_IN6ADDRSZ], *tp, *endp, *colonp;
  const char *curtok;
  int ch, saw_xdigit;
  unsigned val;

  tp = memset (tmp, '\0', NS_IN6ADDRSZ);
  endp = tp + NS_IN6ADDRSZ;
  colonp = NULL;
  /* Leading :: requires some special handling. */
  if (*src == ':')
    if (*++src != ':')
      return (0);
  curtok = src;
  saw_xdigit = 0;
  val = 0;
  while ((ch = tolower (*src++)) != '\0')
    {
      const char *pch;

      pch = strchr (xdigits, ch);
      if (pch != NULL)
	{
	  val <<= 4;
	  val |= (pch - xdigits);
	  if (val > 0xffff)
	    return (0);
	  saw_xdigit = 1;
	  continue;
	}
      if (ch == ':')
	{
	  curtok = src;
	  if (!saw_xdigit)
	    {
	      if (colonp)
		return (0);
	      colonp = tp;
	      continue;
	    }
	  else if (*src == '\0')
	    {
	      return (0);
	    }
	  if (tp + NS_INT16SZ > endp)
	    return (0);
	  *tp++ = (u_char) (val >> 8) & 0xff;
	  *tp++ = (u_char) val & 0xff;
	  saw_xdigit = 0;
	  val = 0;
	  continue;
	}
      if (ch == '.' && ((tp + NS_INADDRSZ) <= endp) &&
	  inet_pton4 (curtok, tp) > 0)
	{
	  tp += NS_INADDRSZ;
	  saw_xdigit = 0;
	  break;		/* '\0' was seen by inet_pton4(). */
	}
      return (0);
    }
  if (saw_xdigit)
    {
      if (tp + NS_INT16SZ > endp)
	return (0);
      *tp++ = (u_char) (val >> 8) & 0xff;
      *tp++ = (u_char) val & 0xff;
    }
  if (colonp != NULL)
    {
      /*
       * Since some memmove()'s erroneously fail to handle
       * overlapping regions, we'll do the shift by hand.
       */
      const int n = tp - colonp;
      int i;

      if (tp == endp)
	return (0);
      for (i = 1; i <= n; i++)
	{
	  endp[-i] = colonp[n - i];
	  colonp[n - i] = 0;
	}
      tp = endp;
    }
  if (tp != endp)
    return (0);
  memcpy (dst, tmp, NS_IN6ADDRSZ);
  return (1);
}
#endif
#endif /* __WIN32__ */

void print_version(void) {
	fprintf(stdout, "squeezeslave %s-%d %s %s\n", version, revision, __DATE__, __TIME__);
	fprintf(stdout, "compile flags: ");
#if defined(__APPLE__) && defined(__MACH__)
	fprintf(stdout, "osx ");
#elif defined(__WIN32__)
	fprintf(stdout, "windows ");
#elif defined(sun)
	fprintf(stdout, "solaris ");
#elif defined(__FreeBSD__)
	fprintf(stdout, "freebsd ");
#elif defined(__OpenBSD__)
	fprintf(stdout, "openbsd ");
#elif defined(__NetBSD__)
	fprintf(stdout, "netbsd ");
#else
	fprintf(stdout, "linux ");
#endif
#ifdef __BIG_ENDIAN__
	fprintf(stdout, "bigendian ");
#endif
#ifndef PORTAUDIO_DEV
	fprintf(stdout, "portaudio:1810 ");
#else
	fprintf(stdout, "portaudio:%d ", Pa_GetVersion());
#endif
#ifdef PADEV_ASIO
	fprintf(stdout, "asio ");
#endif
#ifdef PADEV_WASAPI
	fprintf(stdout, "wasapi ");
#endif
#ifdef SLIMPROTO_DEBUG
	fprintf(stdout, "debug ");
#endif
#ifdef USE_SIGNALS_FOR_RESTART
	fprintf(stdout, "signals ");
#endif
#ifdef GETOPT_SUPPORTS_OPTIONAL
	fprintf(stdout, "getopt ");
#endif
#ifdef BSD_THREAD_LOCKING
	fprintf(stdout, "altlock ");
#endif
#ifdef INTERACTIVE
	fprintf(stdout, "interactive ");
#endif
#ifdef EMPEG
	fprintf(stdout, "empeg ");
#endif
#ifdef DAEMONIZE
	fprintf(stdout, "daemon ");
#endif
#ifdef TREMOR_DECODER
	fprintf(stdout, "tremor ");
#endif
#ifdef AAC_DECODER
	fprintf(stdout, "aac ");
#endif
#ifdef WMA_DECODER
	fprintf(stdout, "wma ");
#endif
#ifdef SLIMPROTO_RENICE
	fprintf(stdout, "renice ");
#endif
#ifdef SLIMPROTO_ZONES
	fprintf(stdout, "zones ");
#endif

	fprintf(stdout, "\n");

	fprintf(stdout, "buffer sizes: decoder %u output %u bytes\n",DECODER_BUFFER_SIZE, OUTPUT_BUFFER_SIZE);
	fprintf(stdout, "\n");

	fprintf(stdout, "Copyright 2004-2013 Richard Titmuss, Duane Paddock.\n");

	fprintf (stdout,
	"This is free software; see the source for copying conditions. There is NO\n"
	"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
	);
}

void print_help(void) {
	print_version();      
	fprintf(stdout,
"\n"
"squeezeslave [options] [<server address>]\n"
"The Squeezebox Server address defaults to 127.0.0.1.\n"
"Options:\n"
"-h, --help                  Prints this message.\n"
"-a,                         Sets the amplitude of a high-frequency tone\n"
"--predelay_amplitude <val>  produced during the predelay (see --predelay).\n"
"                            The frequency is set at the source's sampling\n"
"                            rate/2 and the amplitude is in absolute value.\n"
"                            For 16-bit sources, the max is 32767, but values\n"
"                            below 10 are likely to work.  The goal is to\n"
"                            produce an inaudible signal that will cause DACs\n"
"                            to wake-up and lock before actual samples are\n"
"                            played out.  If the DAC locks using only silence,\n"
"                            do not use this option (it will default to 0).\n"
"-k, --keepalive <sec>       Controls how frequently squeezeslave sends a\n"
"                            alive signal to Squeezebox Server.  6.5.x servers\n"
"                            need this to avoid dropping the player's\n"
"                            connection.  By default, the implementation\n"
"                            chooses the right value: 10s for a >=6.5.x server\n"
"                            and 0s for a <6.5.x server, which means no\n"
"                            keepalive.\n"
"-T, --threshold_override    Ignore autostart threshold for ogg and mp3.\n"
"-O,                         Ignore output threshold when buffering and use\n"
"--output_threshold <bytes>  specified value in bytes.\n"
#ifdef __WIN32__
"-H, --highpriority          Change process priority class to high.\n"
#ifdef PADEV_WASAPI
"-S, --shared                Use shared mode for a WasApi device.\n"
"                            Settings in Control Panel for a shared device\n"
"                            must be set to 16-bit, 44100 Hz (CD Quality).\n"
"                            Ignored for Asio and Direct Sound devices.\n"
#endif
#endif
#if defined(SLIMPROTO_RENICE) && !defined(EMPEG)
"-N, --renice                Increase process priority, root access required.\n"
#endif
#ifdef INTERACTIVE
#ifndef __WIN32__
"-l, --lcd                   Enable LCDd (lcdproc) text display.\n"
"                            Requires LCDd running on local host.\n"
"-C, --lcdc                  Enable old LCDd (lcdproc<0.5.4) text display.\n"
"-i, --lirc                  Enable lirc remote control support.\n"
"                            Requires lirc running on local host.\n"
"-c, --lircrc <filename>     Location of lirc client configuration file.\n"
"                            Default: ~/.lircrc\n"
#endif
"-D, --display               Enable slimp3 style text display and\n"
"                            keyboard input.\n"
"                            Keys: 0-9:             0-9\n"
"                                  Insert or I      Add\n"
"                                  Cursor Keys      Arrows\n"
"                                  >,<              Fwd,Rew\n"
"                                  Home or H        Home\n"
"                                  End or N         Now Playing\n"
"                                  Space or P       Pause\n"
"                                  Enter            Play\n"
"                                  Q                Quit\n" 
"                                  R                Repeat\n" 
"                                  S                Shuffle\n" 
"                                  ?                Search\n"
"                                  b                Browse\n"
"                                  F                Favourites\n"
"                                  %%                Size\n"
"                                  Z                Sleep\n"
"                                  +,-              Vol up,down\n"
"-w, --width <chars>         Set the display width to <chars> characters\n"
#ifndef __WIN32__
"                            If using LCDd, width is detected.\n"
#endif
#endif
"-F, --discovery             Discover server IP automatically.\n"
#ifdef DAEMONIZE
"-M, --daemonize <logfile>   Run squeezeslave as a daemon.\n"
"                            Messages written to specified file.\n"
"                            Not supported with lirc and display modes.\n"
#endif
"-L, --list                  List available audio devices and exit.\n"
"-I, --findservers           List servers found via UDP discovery and exit.\n"
"-m, --mac <mac_address>     Sets the mac address for this instance.\n"
"                            Use the colon-separated notation.\n"
"                            The default is 00:00:00:00:00:01.\n"
"                            Squeezebox Server uses this value to distinguish\n"
"                            multiple instances, allowing per-player settings.\n"
"-n, --name \"<device_name>\"  Sets the output device by name.\n"
"                            The output device names can be found with -L.\n"
#ifdef PORTAUDIO_DEV
"-t,                         Match device name specified with -n/--name to only\n"
"--audiotype \"<pahostapi>\"   this portaudio hostapi name, otherwise ignored.\n"
"-y, --latency <msec>        Modify the default latency for the audio device.\n"
"                            Useful if you experience drop outs during playback.\n"
"                            Values between 80-200 ms are recommended.\n"
#else
"-g, --paframes              Set the number of frames per buffer that portaudio\n"
"                            will use.  Default is %d.\n"
"-j, --pabuffers             Set the number of internal buffers for portaudio.\n"
"                            Use 0 to tell portaudio to decide how many buffers.\n"
"                            Default is %d.\n"
#endif
"-o, --output <device_id>    Sets the output device id.\n"
"                            The output device id can be found with -L.\n"
"-P, --port <portnumber>     Sets the Squeezebox Server port number.\n"
"                            The default port is %d.\n"
"-p, --predelay <msec>       Sets a delay before any playback is started.  This\n"
"                            is useful if the DAC used for output is slow to\n"
"                            wake-up/lock, causing the first few samples to be\n"
"                            dropped.\n"
#ifdef EMPEG
"-Q, --puteq                 Reads the empeg equalizer from the DSP and writes\n"
"                            it to eq.dat\n"
"-q, --geteq                 Reads the eq.dat file and configures the empeg eq\n"
#endif
"-R, --retry                 Causes the program to retry connecting to\n"
"                            Squeezebox Server until it succeeds or is stopped\n"
"                            using SIGTERM or keyboard entry.\n"
"                            If the connection to Squeezebox Server is lost, the\n"
"                            program will poll it until it restarts.  --retry\n"
"                            enables retry with a %d second delay between\n"
"                            attempts.\n"
"-r <sec>, --intretry <sec>  For a different retry interval use -r and the\n"
"                            desired interval in seconds. (ie. -r10)\n"
"                            Retry interval range is 1-120 seconds.\n"
"-V, --version               Prints the squeezeslave version.\n"
#ifndef PORTAUDIO_DEV
"-v, --volume <on|sw|off>    Enables/disables volume changes done by\n"
#else
"-v, --volume <sw|off>       Enables/disables volume changes done by\n"
#endif
"                            Squeezebox Server during its operation, such as\n"
"                            when changing the volume through the web interface\n"
"                            or when applying replay gain.  Defaults to "
#ifdef EMPEG 
"on"
#else
"sw"
#endif
".\n"
#ifndef PORTAUDIO_DEV
"                                  on:  volume changes performed on device.\n"
#endif
"                                  sw:  volume changes performed in software.\n"
"                                  off: volume changes ignored.\n"
#ifdef SLIMPROTO_DEBUG
"-Y, --debuglog <logfile>    Redirect debug output from stderr to <logfile>.\n"
#endif
#ifdef SLIMPROTO_ZONES
"-z, --zone <x/y>            Pairs surround sound speakers into stereo zones.\n"
"                            Requires 5.1 or better amplifier and sound card.\n"
#endif
"-d, --debug <trace_name>    Turns on debug tracing for the specified level.\n"
"                            The option can be used multiple times to enable\n"
"                            multiple levels.\n"
"                            Available levels:\n"
#ifndef SLIMPROTO_DEBUG
"                            (disabled, recompile with \"-DSLIMPROTO_DEBUG\")\n"
#endif
"                                  all\n"
"                                  slimproto\n"
"                                  slimaudio\n"
"                                  slimaudio_buffer\n"
"                                  slimaudio_buffer_v\n"
"                                  slimaudio_decoder\n"
"                                  slimaudio_decoder_r\n"
"                                  slimaudio_decoder_v\n"
"                                  slimaudio_http\n"
"                                  slimaudio_http_v\n"
"                                  slimaudio_output\n"
"                                  slimaudio_output_v\n",
#ifndef PORTAUDIO_DEV
PA_FRAMES_PER_BUFFER,
PA_NUM_BUFFERS,
#endif
SLIMPROTOCOL_PORT,
RETRY_DEFAULT);
}

