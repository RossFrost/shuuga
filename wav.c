#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <native/shuuga.h>
#include <discid/discid.h>
#include <fileapi.h>


int create_track_filename(TOC *toc, int track_num,  int album_num, char *base, char *output, int length) {
    if (toc->albums == NULL) {
        puts("Album hasn't be initated");
        return 1;
    }

    if (track_num > toc->last_track) {
        puts("Track is greater than track max");
        return 1;
    }
    
    if (album_num > toc->albums->current_album_num) {
        puts ("Album is greater than album max");
        return 1;
    }

    snprintf(output, length, "%s - %s.wav", toc->albums->album_tracks[album_num].track_names[track_num], 
                                                                         toc->albums->album_artist[album_num]
                );

    return 0;
}


int write_wav_file(TOC *toc, CdIo_t *disc, char index, short channel, char *filename) {
    if (index >= toc->track_LSN_max_index) {
        puts("Track index out of range...");
        return 1;
    }

    uint32_t num_sectors = toc->last_track_LSN[index] - toc->first_track_LSN[index]; 
    
    puts(filename);
    FILE *file = fopen(filename, "wb");

    if (!file) {
        perror("fopen");
        return 1;
    }

    int file_size = (num_sectors * CD_FRAMESIZE_RAW + 36);
    int subchunk1_size = 16;
    short pcm_format = 1;
    int sample_rate = 44100; 
    int byte_rate = sample_rate * channel * 16 / 8;
    short block_align = channel * 16 / 8;
    short bits_per_sample = 16;
    int raw_audio_size = num_sectors * CD_FRAMESIZE_RAW;
    char *RIFF = "RIFF";
    char *WAVE = "WAVE";
    char *FMT  = "fmt ";
    char *data = "data";


    fwrite(RIFF, 4, 1, file); 
    fwrite(&file_size, 4, 1, file); 
    fwrite(WAVE, 4, 1, file); 
    fwrite(FMT, 4, 1, file);
    fwrite(&subchunk1_size, 4, 1, file); 
    fwrite(&pcm_format, 2, 1, file); 
    fwrite(&channel, 2, 1, file); 
    fwrite(&sample_rate, 4, 1, file); 
    fwrite(&byte_rate, 4, 1, file); 
    fwrite(&block_align, 2, 1, file); 
    fwrite(&bits_per_sample, 2, 1, file); 
    fwrite(data, 4, 1, file); 
    fwrite(&raw_audio_size, 4, 1, file); 
    
    fflush(file);

    for (lsn_t i_sector = toc->first_track_LSN[index]; i_sector <= toc->last_track_LSN[index]; ++i_sector) {
        void *buffer[CD_FRAMESIZE_RAW];
        cdio_read_audio_sector(disc, buffer, i_sector);
        fwrite(buffer, CD_FRAMESIZE_RAW, 1, file);
        fflush(file);
    }

    fclose(file);

    return 0;
}

//Get TOC
int get_TOC_values(CdIo_t *disc, TOC *toc) {
    toc->first_track = cdio_get_first_track_num(disc);
    toc->last_track = cdio_get_last_track_num(disc);

    if (
        toc->first_track == CDIO_INVALID_TRACK || 
        toc->last_track == CDIO_INVALID_TRACK
        ) {
        return 1;
    }
    
    char track_index = 0;
    for (char i_track = toc->first_track; i_track < toc->last_track + 1; ++i_track) {
        toc->first_track_LSN[track_index] = cdio_get_track_lsn(disc, i_track);

        if (toc->first_track_LSN[track_index] == CDIO_INVALID_LSN) {
            puts("Invalid sector address... Skipping");
            continue;
        }
        toc->last_track_LSN[track_index] = cdio_get_track_last_lsn(disc, i_track);

        ++track_index;
    }

    toc->track_LSN_max_index = track_index;

    return 0;
}

//Make sure to close CDIO first
void get_disc_id(TOC *toc) {
    toc->discid = calloc(29, sizeof(char));

    DiscId discid = discid_new();
    discid_read_sparse(discid, discid_get_default_device(), 0);
    toc->discid = discid_get_id(discid);
    discid_free(discid);

    printf("Discid: %s\n", toc->discid);
}
