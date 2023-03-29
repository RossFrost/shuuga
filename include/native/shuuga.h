#pragma once
#include <cdio/cdio.h>

#define CDROM_DRIVE_CODE 5

#define CD 0
#define DVD 1
#define BLURAY 2

#define MONO 1
#define STERO 2

typedef struct {
    char xml_element_name[200];
    char previous_xml_element_name[200];
    int xml_depth;
} XML_ELEMENT;

typedef struct {
    char *track_names[99];
} TRACK;

typedef struct {
    int album_select;
    int current_album_num;
    char *album_names[99];
    char *album_countries[99];
    char *album_dates[99];
    char *album_artist[99];
    int current_track_num;
    int max_tracks;
    TRACK album_tracks[99];
    XML_ELEMENT element_data;
} ALBUM;


typedef struct {
    track_t first_track;
    track_t last_track;
    lba_t first_track_LSN[99];
    lba_t last_track_LSN[99];
    char track_LSN_max_index;
    char *discid;
    ALBUM *albums;
} TOC;



int write_wav_file(TOC *toc, CdIo_t *disc, char index, short channel, char *filename);
int get_TOC_values(CdIo_t *disc, TOC *toc);
void get_disc_id(TOC *toc);
int retrieve_xml_file(TOC *toc, char *xml_filename);
int parse_xml_file(TOC *toc, char *filename);
int create_track_filename(TOC *toc, int track_num,  int album_num, char *base, char *output, int length);