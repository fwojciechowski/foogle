#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <omp.h>

// Definicja kolorow do terminala
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"

char bibliotekaPath[200];
const int gRecordsSize = 3000; // maksymalna liczba rekordow do wykorzystania

// Struktura do trzymania nazw plikow z biblioteki oraz sciezki do nich.
struct Record {
    char name[100];
    char fullPath[200];
};

// Struktura do trzymania wynikow - nazwa pliku oraz ilosc wystapien frazy w danym pliku.
struct Result {
    char name[100];
    int hits;
};

// Inicjalizacja biblioteki
// Funkcja zczytuje z podanej lokalizacji liste plikow, w ktorym odbeda sie poszukiwania zadanej frazy
int initializeLibrary(struct Record library[], int* size) {
    int status = 0;
    DIR *biblioteka;
    struct dirent *dir;

    biblioteka = opendir(bibliotekaPath);

    if (biblioteka) {
        while ((dir = readdir(biblioteka)) != NULL && (*size) < gRecordsSize) {
            if (dir->d_type != DT_REG) continue; // interesujace sa tylko zwykle pliki

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

// Znajdywanie frazy
// Glowna petla programu. Na podstawie bazy plikow i frazy funkcja przeszukuje biblioteke i zwraca wynik
// do tablicy struktur Result (poprzez wskaznik r).
void findPhrase(const struct Record records[], const int* recordsSize, struct Result* r, int* resultsSize, const char phrase[]) {
    char line[256];
    int count;
    FILE* file;
#pragma omp parallel for shared(r, resultsSize, records, phrase) private(file, line, count) num_threads(4)
    for (int i = 0; i < *recordsSize; i++) {

        count = 0;

        file = fopen(records[i].fullPath, "r");

        if (file != NULL) {
            while (fgets(line, sizeof(line), file)) {
                if (strstr(line, phrase)){
                    count++;
                }
            }

            if (count) {
#pragma omp critical
                {
                    r->hits = count;
                    strcpy(r->name, records[i].name);
                    (*resultsSize)++;
                    r++;
                };
            }

            fclose(file);
        }
    }
}

int main(int argc, char *argv[]) {
    struct Record records[gRecordsSize];  // tablica zawierajaca dane o plikach z biblioteki
    int recordsSize = 0;
    struct Result results[gRecordsSize];  // tablica na wyniki poszukiwan
    struct Result *r = results;
    int resultsSize = 0;
    char phrase[256];                     // szukana fraza

    // Sprawdzamy, czy lokalizacja biblioteki oraz szukana fraza zostaly podane.
    if ( argc != 3 ) {
        printf("Program potrzebuje lokalizacji biblioteki oraz frazy do wyszukiwania w bibliotece.\n\n");
        printf("Przykład:\n./foogle ./biblioteka pomidor\n");
        return EXIT_FAILURE;
    }

    strcpy(bibliotekaPath, argv[1]);
    if (bibliotekaPath[ strlen(bibliotekaPath) - 1 ] != '/') {
        strcat(bibliotekaPath, "/");
    }

    strcpy(phrase, argv[2]);

    // Biblioteka wymaga inicjalizacji tj. utworzenia listy znajdujacych sie w niej plikow.
    if (initializeLibrary(records, &recordsSize) == 1) {
        printf("Błąd przy inicjalizacji biblioteki. Prawdopodobnie program nie odnalazł podanej ścieżki.\n\n");
        return EXIT_FAILURE;
    }

    printf("foogle - wyszukiwarka fraz w bibliotece\n\n");
    printf("Szukanie frazy: " KYEL "%s" KNRM "\n\n", phrase);
    printf("Przeszukiwanie %d rekordów...\n\n", recordsSize);

    double beginTime = omp_get_wtime();

    // Serce programu - wyszukiwanie frazy w bibliotece.
    findPhrase(records, &recordsSize, r, &resultsSize, phrase);

    double endTime = omp_get_wtime();

    // Czesc przekazujaca wyniki dalej (w tej chwili wydruk na ekran)
    if (resultsSize == 0) {
        printf(KRED "Brak wyników.\n\n" KNRM);
    } else {
        for (int i = 0; i < resultsSize; i++) {
            printf(KGRN "%s" KNRM " Hits: %d\n", results[i].name, results[i].hits);
        }
    }

    printf("\nCzas szukania: %fs\n", (endTime - beginTime));

    return 0;
}