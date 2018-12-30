#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"

const char bibliotekaPath[] = "../biblioteka/";
const int gRecordsSize = 1000;

struct Record {
    char name[50];
    char fullPath[100];
};

struct Result {
    char name[50];
    int hits;
};

int initializeLibrary(struct Record library[], int* size) {
    int status = 0;
    DIR *biblioteka;
    struct dirent *dir;

    biblioteka = opendir(bibliotekaPath);

    if (biblioteka) {
        while ((dir = readdir(biblioteka)) != NULL && (*size) < gRecordsSize+1) {
            if (dir->d_type != DT_REG) continue; // katalogi nas nie interesuja

            strcpy(library->name, dir->d_name);
            strcpy(library->fullPath, bibliotekaPath);
            strcat(library->fullPath, dir->d_name);

            library++;
            (*size)++;
        }
        closedir(biblioteka);
    }

    return status;
}

int main(int argc, char *argv[]) {
    struct Record records[gRecordsSize];
    int recordsSize = 0;
    struct Result results[gRecordsSize];
    int resultsSize = 0;

    if ( argc != 2 ) {
        printf("Program potrzebuje frazy do wyszukiwania w bibliotece.\n");
        return EXIT_FAILURE;
    }

    clock_t beginTime = clock();

    initializeLibrary(records, &recordsSize);

    printf("Foogle - wyszukiwarka fraz w bibliotece\n");

    for (int i = 0; i < recordsSize; i++) {
        FILE* file = fopen(records[i].fullPath, "r");

        if (file != NULL) {
            char line[256];
            int count = 0;

            while (fgets(line, sizeof(line), file)) {
                if (strstr(line, argv[1])){
                    count++;
                }
            }

            if (count) {
                printf(KGRN "%s" KNRM " Hits: %d\n", records[i].fullPath, count);
            } else {
                printf(KRED "%s\n", records[i].fullPath);
            }

            fclose(file);
        }
    }

    clock_t endTime = clock();

    printf("Czas wykonania: %fs", (double)(endTime - beginTime) / CLOCKS_PER_SEC);

    return 0;
}