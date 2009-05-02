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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <vorbis/vorbisfile.h>

#include "slimproto/slimproto.h"
#include "slimaudio/slimaudio.h"

#ifdef SLIMPROTO_DEBUG
  #define DEBUGF(...) if (slimaudio_decoder_debug) fprintf(stderr, __VA_ARGS__)
  #define VDEBUGF(...) if (slimaudio_decoder_debug_v) fprintf(stderr, __VA_ARGS__)
#else
  #define DEBUGF(...)
  #define VDEBUGF(...)
#endif

static size_t vorbis_read_func(void *ptr, size_t size, size_t nmemb, void *datasource);
static int vorbis_seek_func(void *datasource, ogg_int64_t offset, int whence);
static int vorbis_close_func(void *datasource);
static long vorbis_tell_func(void *datasource);



int slimaudio_decoder_vorbis_init(slimaudio_t *audio) {
	return 0;
}

void slimaudio_decoder_vorbis_free(slimaudio_t *audio) {
}

int slimaudio_decoder_vorbis_process(slimaudio_t *audio) {
	int err;
	
	assert(audio != NULL);

	DEBUGF("slimaudio_decoder_vorbis_process: start\n");
	
	ov_callbacks callbacks;
	callbacks.read_func = vorbis_read_func;
	callbacks.seek_func = vorbis_seek_func;
	callbacks.close_func = vorbis_close_func;
	callbacks.tell_func = vorbis_tell_func;	

	audio->decoder_end_of_stream = false;
	
    if ((err = ov_open_callbacks(audio, &audio->oggvorbis_file, NULL, 0, callbacks)) < 0) {
    	fprintf(stderr, "Error in ov_open_callbacks %i\n", err);
    	return -1;	
    }
	
	int bytes_read;
	int current_bitstream;
	char buffer[AUDIO_CHUNK_SIZE];
	
	do {
#ifdef __BIG_ENDIAN__
		bytes_read = ov_read(&audio->oggvorbis_file, buffer, AUDIO_CHUNK_SIZE, 1, 2, 1, &current_bitstream);
#else /* __LITTLE_ENDIAN__ */
		bytes_read = ov_read(&audio->oggvorbis_file, buffer, AUDIO_CHUNK_SIZE, 0, 2, 1, &current_bitstream);
#endif
		switch (bytes_read) {
		case OV_HOLE:
		case OV_EBADLINK:
	    	fprintf(stderr, "Error decoding vorbis stream\n");
			goto vorbis_error;
		
		case 0: // End of file	
			break;
			
		default:
			apply_replaygain(audio->replay_gain, buffer, bytes_read);
			slimaudio_buffer_write(audio->output_buffer, buffer, bytes_read);
		}

	} while (bytes_read > 0);
	
	if ((err = ov_clear(&audio->oggvorbis_file)) < 0) {
		fprintf(stderr, "Error in ov_clear %i\n", err);
		return -1;	
	}

	DEBUGF("slimaudio_decoder_vorbis_process: end\n");

	return 0;
	
vorbis_error:
	if ((err = ov_clear(&audio->oggvorbis_file)) < 0) {
		fprintf(stderr, "Error in ov_clear %i\n", err);
		return -1;	
	}

	return -1;
}


static size_t vorbis_read_func(void *ptr, size_t size, size_t nmemb, void *datasource) {
	slimaudio_t *audio = (slimaudio_t *) datasource;
	
	pthread_mutex_lock(&audio->decoder_mutex);

	if (audio->decoder_state != STREAM_PLAYING) {
		pthread_mutex_unlock(&audio->decoder_mutex);
		return 0;
	}

	pthread_mutex_unlock(&audio->decoder_mutex);
	
	if (audio->decoder_end_of_stream)
		return 0;
	
	int data_len = nmemb;
	slimaudio_buffer_status ok = slimaudio_buffer_read(audio->decoder_buffer, ptr, &data_len);
	if (ok == SLIMAUDIO_BUFFER_STREAM_END) {
		audio->decoder_end_of_stream = true;
	}
	
	return data_len;
}

static int vorbis_seek_func(void *datasource, ogg_int64_t offset, int whence) {
	return -1; /* stream is not seekable */
}

static int vorbis_close_func(void *datasource) {
	return 0;
}

static long vorbis_tell_func(void *datasource) {
	return 0;
}
