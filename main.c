#include <curl/curl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

const char *filename_from_url(const char *url);

typedef struct {
    const char *url;
    const char *dest_filename;
    const char *dest_path;
    // protocol, what else?
} Download;

typedef struct {
    Download *queue; // implement a queue for this?
} DownloadBatch;

size_t write_to_file(char *ptr, size_t size, size_t nmemb, void *userdata) {
    return fwrite(ptr, size, nmemb, userdata);
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("Usage:\n\t%s <url> <filename>\n\n", argv[0]);
        exit(1);
    }

    CURL *curl;
    CURLcode res;
    FILE *writefile;
    const char *writefile_name;

    if(argc == 2)
        writefile_name = filename_from_url(argv[1]);
    else
        writefile_name = argv[2];

    writefile = fopen(writefile_name, "wx");
    if(errno == EEXIST) {
        fprintf(stderr, "Error: Could not open %s -- Already exists\n", writefile_name);
        exit(1);
    } else if(!writefile) {
        fprintf(stderr, "Error: Could not open %s\n", writefile_name);
        exit(1);
    }
    
    if(curl_global_init(CURL_GLOBAL_DEFAULT)) {
        fprintf(stderr, "Error: Could not initialize libcurl\n");
        exit(1);
    }
    curl = curl_easy_init();
    if(!curl) {
        fprintf(stderr, "Error: Could not get libcurl easy handle\n");
        curl_global_cleanup();
        exit(1);
    }

    curl_easy_setopt(curl, CURLOPT_URL, argv[1]);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_file);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, writefile);
    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

const char *filename_from_url(const char *url) {
    int i = strlen(url);
    while(url[i] != '/') {
        i--;
    }
    return &url[i+1];
}
