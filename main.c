#include <native/shuuga.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <Windows.h>
#include <cdio/logging.h>

void free_album(ALBUM *albums) {
    for (int i_album = 0; albums->current_album_num > i_album; ++i_album) {
        free(albums->album_countries[i_album]);
        free(albums->album_dates[i_album]);
        free(albums->album_names[i_album]);
        free(albums->album_artist[i_album]);
;
        for (int i_track = 0; i_track < albums->max_tracks; ++i_track) {
            free(albums->album_tracks[i_album].track_names[i_track]);
        }
    }

}

void free_toc(TOC *toc) {
    free(toc->discid);
    free_album(toc->albums);
}


void random_filename(char *base, int length, char *output) {
    char char_gen[] = "0123456789AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz";

    char *placeholder = malloc(sizeof(char) * 20);

    time_t i_time;
    srand((unsigned) time(&i_time));

    int queue = 0;
    while (--length >= 0) {
        int index = rand() % strlen(char_gen);

        placeholder[length] = char_gen[index];
        ++queue;
    }

    placeholder[queue] = '\0';

    sprintf(output, "%s%s", base, placeholder);



    free(placeholder);
}


static void log_handler(cdio_log_level_t level, const char *message) {
    puts(message);
}



TOC *init_rip(TOC *toc, char *output_folder) {
    cdio_loglevel_default = CDIO_LOG_ERROR;
    cdio_log_set_handler(log_handler);

    puts("Getting drive handle...");
    CdIo_t *cdio = cdio_open(0, DRIVER_DEVICE);

    puts("Getting TOC...");
    get_TOC_values(cdio, toc);
    puts("Getting discid...");
    get_disc_id(toc);
    puts("Generating temp file");
    char *filename = calloc(20 + strlen(output_folder) + 1, sizeof(char));
    random_filename(output_folder, 20, filename);

    puts("Retrieve disc info...");
    retrieve_xml_file(toc, filename);
    parse_xml_file(toc, filename);
    remove(filename);

    puts("Choose album");
    char album_choice;

    while (scanf("%s", &album_choice) == 1) {
        if (atoi(&album_choice) > toc->albums->current_album_num ||  atoi(&album_choice) <= 0) {
            puts("Invalid choice...");
            continue;
        }
        break;
    }

    puts("works");
    char *direcotry_path = calloc(strlen(output_folder) + strlen(toc->albums->album_names[atoi(&album_choice) - 1] + 3), sizeof(char));

    sprintf(direcotry_path, "%s%s", output_folder, toc->albums->album_names[atoi(&album_choice) - 1] );
    printf("Directory Path: %s\n", direcotry_path);

    CreateDirectoryA(direcotry_path, NULL);
    SetCurrentDirectoryA(direcotry_path);

    char *track_file = calloc(1024, sizeof(char));
    for (int i_track = 0; i_track < toc->last_track; ++i_track) {
        create_track_filename(toc, i_track, atoi(&album_choice) - 1, "output", track_file, 1024);
        write_wav_file(toc, cdio, i_track, STERO, track_file);
    }

    free(track_file);
    cdio_destroy(cdio);

    free(filename);
    free_toc(toc);

    return toc;
}


int main() {
    TOC toc;

    init_rip(&toc, "output/");

}