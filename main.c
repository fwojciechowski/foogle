#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"

char bibliotekaPath[200];
const int gRecordsSize = 1200; // maksymalna liczba rekordow do wykorzystania

struct Record {
    char name[100];
    char fullPath[200];
};

struct Result {
    char name[100];
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
    } else {
        status = 1;
    }

    return status;
}

int main(int argc, char *argv[]) {
    struct Record records[gRecordsSize];
    int recordsSize = 0;
    struct Result results[gRecordsSize];
    struct Result *r = results;
    int resultsSize = 0;

    if ( argc != 3 ) {
        printf("Program potrzebuje lokalizacji biblioteki oraz frazy do wyszukiwania w bibliotece.\n\n");
        printf("Przykład:\n./foogle ./biblioteka pomidor\n");
        return EXIT_FAILURE;
    }

    strcpy(bibliotekaPath, argv[1]);
    if (bibliotekaPath[ strlen(bibliotekaPath) - 1 ] != '/') {
        strcat(bibliotekaPath, "/");
    }

    if (initializeLibrary(records, &recordsSize) == 1) {
        printf("Błąd przy inicjalizacji biblioteki. Prawdopodobnie program nie odnalazł podanej ścieżki.\n\n");
        return EXIT_FAILURE;
    }

    clock_t beginTime = clock();

    printf("foogle - wyszukiwarka fraz w bibliotece\n\n");

    printf("Szukanie frazy: " KYEL "%s" KNRM "\n\n", argv[2]);

    printf("Przeszukiwanie %d rekordów...\n\n", recordsSize);

    // Glowna petla for do przyszlego zrownoleglenia.
    for (int i = 0; i < recordsSize; i++) {
        FILE* file = fopen(records[i].fullPath, "r");

        if (file != NULL) {
            char line[256];
            int count = 0;

            while (fgets(line, sizeof(line), file)) {
                if (strstr(line, argv[2])){
                    count++;
                }
            }

            if (count) {
                r->hits = count;
                strcpy(r->name, records[i].name);
                resultsSize++;
                r++;
            }

            fclose(file);
        }
    }

    // Czesc przekazujaca wyniki dalej (w tej chwili wydruk na ekran)
    if (resultsSize == 0) {
        printf(KRED "Brak wyników.\n\n" KNRM);
    } else {
        for (int i = 0; i < resultsSize; i++) {
            printf(KGRN "%s" KNRM " Hits: %d\n", results[i].name, results[i].hits);
        }
    }

    clock_t endTime = clock();

    printf("\nCzas wykonania: %fs\n", (double)(endTime - beginTime) / CLOCKS_PER_SEC);

    return 0;
}