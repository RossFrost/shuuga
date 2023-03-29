#include <native/shuuga.h>
#include <expat/expat.h>
#include <stdio.h>
#include <string.h>


static void XMLCALL  start_element(void *structure, const char *name, const char **attributes) {
    ALBUM *albums = (ALBUM *) structure;
    ++(albums->element_data.xml_depth);
    size_t length;

    strncpy(albums->element_data.previous_xml_element_name, albums->element_data.xml_element_name, 200);
    length = strnlen(albums->element_data.xml_element_name, 200);
    albums->element_data.previous_xml_element_name[length] = '\0';

    strncpy(albums->element_data.xml_element_name, name, 200);
    length = strnlen(name, 200);
    albums->element_data.xml_element_name[length] = '\0';

    if (strncmp(albums->element_data.xml_element_name, "release", 200) == 0) {
        ++albums->current_album_num;
        albums->current_track_num = 0;
    }

}


static void XMLCALL  end_element(void *structure, const char *name) {
    ALBUM *albums = (ALBUM *) structure;
    --(albums->element_data.xml_depth);
}


static void XMLCALL  char_data(void *structure, const char *string, int length) {
    ALBUM *albums = (ALBUM *) structure;

    char *element_data = malloc(length * sizeof(char));
    strncpy(element_data, string, length);
    element_data[length] = '\0';

    if (albums->element_data.xml_depth > 4) {
        if (strncmp(albums->element_data.xml_element_name, "title", 200) == 0 && albums->element_data.xml_depth == 5) {
            albums->album_names[albums->current_album_num - 1] = element_data;
        }

        else if (strncmp(albums->element_data.xml_element_name, "country", 200) == 0) {
            albums->album_countries[albums->current_album_num - 1] = element_data;
        }

        else if (strncmp(albums->element_data.xml_element_name, "date", 200) == 0) {
            albums->album_dates[albums->current_album_num - 1] = element_data;
        }

        else if (strncmp(albums->element_data.xml_element_name, "name", 200) == 0 && strncmp("artist", albums->element_data.previous_xml_element_name, 200) == 0) {
            albums->album_artist[albums->current_album_num - 1] = element_data;
            puts(element_data);
        }

        else if (strncmp(albums->element_data.xml_element_name, "title", 200) == 0 && albums->element_data.xml_depth > 6) {
            if (albums->current_track_num < albums->max_tracks && albums->current_album_num < 100) {
                albums->album_tracks[albums->current_album_num - 1].track_names[albums->current_track_num] = element_data;
                ++albums->current_track_num;
            }
        }
    }

    else {
        free(element_data);
    }
}


int parse_xml_file(TOC *toc, char *filename) {
    XML_Parser parser = XML_ParserCreate("UTF-8");
    ALBUM albums;


    albums.element_data.xml_depth = 0;
    albums.current_album_num = 0;
    albums.max_tracks = toc->last_track;

    XML_SetElementHandler(parser, start_element, end_element);
    XML_SetCharacterDataHandler(parser, char_data);
    XML_SetUserData(parser, &albums);

    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        XML_ParserFree(parser);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(sizeof(char) * file_size);
    fread(buffer, sizeof(char), file_size, file);
    fclose(file);

    XML_Parse(parser, buffer, file_size, 1);
    XML_ParserFree(parser);

    ALBUM *p_album = &albums;
    toc->albums = p_album;

    for (int i_album = 0; toc->albums->current_album_num > i_album; ++i_album) {
        printf("%i)  %s : %s : %s : %s \n", i_album + 1, toc->albums->album_artist[i_album], toc->albums->album_names[i_album], toc->albums->album_countries[i_album], toc->albums->album_dates[i_album]);
     }  

    return 0;
}
