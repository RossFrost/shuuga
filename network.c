#include <expat/expat.h>
#include <curl/curl.h>
#include <native/shuuga.h>
#include <stdio.h>


size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *file) {
    size_t written = fwrite(ptr, size, nmemb, file);
    return written;
}


int retrieve_xml_file(TOC *toc, char *xml_filename) {
    char *request_url_base = "https://musicbrainz.org/ws/2/discid/";
    char * request_url = calloc(strlen(request_url_base) + 50, sizeof(char));
    sprintf(request_url, "%s%s?inc=recordings+artists", request_url_base, toc->discid);

    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();

    res = curl_easy_setopt(curl, CURLOPT_URL, request_url);
    
    if (res != CURLE_OK) {
        printf("Couldn't connect to '%s' \n CODE: %s \n", request_url, res);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 1;
    }

    FILE *file = fopen(xml_filename, "wb");

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "RossFrost/Shuuga");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        puts(curl_easy_strerror(res));
        return 1;
    }


    fclose(file);
    curl_easy_cleanup(curl);
    curl_global_cleanup();


    return 0;
}       
