#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>

// Definicja kolorow do terminala
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"

char bibliotekaPath[300];
const int gRecordsSize = 3000; // maksymalna liczba rekordow do wykorzystania

// Struktura do trzymania nazw plikow z biblioteki oraz sciezki do nich.
struct Record {
    char name[200];
    char fullPath[300];
};

// Struktura do trzymania wynikow - nazwa pliku oraz ilosc wystapien frazy w danym pliku.
struct Result {
    char name[200];
    int hits;
};

enum foogleMpiTags {
    AVAILABILITY,
    WORK,
    RESULTS
};

enum workStatus {
    THERE_IS_NO_WORK = 0,
    THERE_IS_WORK = 1
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
void findPhrase(const struct Record record, struct Result* r, const char phrase[]) {
    char line[256];
    int count = 0;
    FILE* file = fopen(record.fullPath, "r");

    if (file != NULL) {
        while (fgets(line, sizeof(line), file)) {
            if (strstr(line, phrase)){
                count++;
            }
        }

        r->hits = count;
        strcpy(r->name, record.name);

        fclose(file);
    }

}

int main(int argc, char *argv[]) {
    struct Record records[gRecordsSize];  // tablica zawierajaca dane o plikach z biblioteki
    int recordsSize, currentRecordNumber = 0;
    struct Result results[gRecordsSize];  // tablica na wyniki poszukiwan
    struct Result *r = results;
    int resultsSize = 0;
    int searchedRecords = 0;              // ile rekordów zostało przeszukanych
    char phrase[256];                     // szukana fraza
    int mynum, nprocs, i, closedProcesses = 0;
    int isThereWork = THERE_IS_WORK;                  // bool określający istnienie pracy


    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mynum);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    // Sprawdzamy, czy lokalizacja biblioteki oraz szukana fraza zostaly podane.
    if ( argc != 3 ) {
        printf("Program potrzebuje lokalizacji biblioteki oraz frazy do wyszukiwania w bibliotece.\n\n");
        printf("Przykład:\n./foogle ./biblioteka pomidor\n");

        MPI_Finalize();
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

        MPI_Finalize();
        return EXIT_FAILURE;
    }

    printf(KGRN "Proces %d z %d gotowy.\n" KNRM, mynum, nprocs);

    if (mynum == 0) {
        printf("foogle - wyszukiwarka fraz w bibliotece\n\n");
        printf("Szukanie frazy: " KYEL "%s" KNRM "\n\n", phrase);
        printf("Przeszukiwanie %d rekordów...\n\n", recordsSize);
    }

    double beginTime = MPI_Wtime();

    // Serce programu - wyszukiwanie frazy w bibliotece.

    while (isThereWork == THERE_IS_WORK) {
        MPI_Status status;
        int receiveBuff, sendBuff = 0;
        int resultBuff[2];

        if (mynum == 0) {
            // Majster thread
            int flag = 1;

            // Sprawdz czy wciaz jest praca do wykonania
            if (searchedRecords >= recordsSize && closedProcesses == nprocs - 1) isThereWork = THERE_IS_NO_WORK;

             do {
                // Sprawdz czy ktos pyta o prace
                MPI_Iprobe( MPI_ANY_SOURCE, AVAILABILITY, MPI_COMM_WORLD, &flag, &status );

                if (flag) {
                    //printf("Proces %d szuka roboty...\n", status.MPI_SOURCE);

                    MPI_Recv(&receiveBuff, 1, MPI_INT, status.MPI_SOURCE, AVAILABILITY, MPI_COMM_WORLD, &status);

                    if (searchedRecords < recordsSize) {
                        // wciaz jest praca, przekaz ja
                        sendBuff = THERE_IS_WORK;
                    } else {
                        // juz nie ma plikow do przeszukania - daj znac procesowi, zeby skonczyl dzialanie
                        printf("Nie ma pracy dla procesu %d.\n", status.MPI_SOURCE);
                        sendBuff = THERE_IS_NO_WORK;
                        closedProcesses++;
                    }

                    MPI_Send(&sendBuff, 1, MPI_INT, status.MPI_SOURCE, AVAILABILITY, MPI_COMM_WORLD);

                    if (sendBuff == THERE_IS_WORK) {
                        // proces czeka na prace, wyslij mu nr rekordu
                        sendBuff = currentRecordNumber;
                        MPI_Send(&sendBuff, 1, MPI_INT, status.MPI_SOURCE, WORK, MPI_COMM_WORLD);
                        //printf("Wyslano prace procesowi %d.\n", status.MPI_SOURCE);
                        currentRecordNumber++;
                    }

                }
            } while (flag);

            do {
                // Sprawdz czy ktos ma gotowe wyniki
                MPI_Iprobe( MPI_ANY_SOURCE, RESULTS, MPI_COMM_WORLD, &flag, &status );

                if (flag) {
                    MPI_Recv(&resultBuff, 2, MPI_INT, status.MPI_SOURCE, RESULTS, MPI_COMM_WORLD, &status);

                    if (resultBuff[1] > 0) {
                        // Bylo trafienie, zapiszmy wynik
                        r->hits = resultBuff[1];
                        strcpy(r->name, records[resultBuff[0]].name);
                        resultsSize++;
                        r++;
                    }

                    searchedRecords++;
                }
            } while (flag);



        } else {
            // Worker thread

            // Powiedz, ze jestes gotowy na prace i sprawdz, czy jest ona dostepna
            sendBuff = 1;

            MPI_Sendrecv(&sendBuff, 1, MPI_INT, 0, AVAILABILITY,
                         &receiveBuff, 1, MPI_INT, 0, AVAILABILITY,
            MPI_COMM_WORLD, &status);

            if (receiveBuff == THERE_IS_WORK) {
                struct Result result;


                MPI_Recv(&receiveBuff, 1, MPI_INT, 0, WORK, MPI_COMM_WORLD, &status);

                //printf("Proces %d otrzymal %s.\n", mynum, records[receiveBuff].name);

                findPhrase(records[receiveBuff], &result, phrase);

                resultBuff[0] = receiveBuff;
                resultBuff[1] = result.hits;

                MPI_Send(&resultBuff, 2, MPI_INT, status.MPI_SOURCE, RESULTS, MPI_COMM_WORLD);
            } else {
                // nie ma pracy, zakoncz dzialanie
                isThereWork = THERE_IS_NO_WORK;
            }
        }
    }

    //findPhrase(records, &recordsSize, r, &resultsSize, phrase);

    double endTime = MPI_Wtime();

    // Czesc przekazujaca wyniki dalej (w tej chwili wydruk na ekran)

    if (mynum == 0) {
        if (resultsSize == 0) {
            printf(KRED "Brak wyników.\n\n" KNRM);
        } else {
            for (int i = 0; i < resultsSize; i++) {
                printf(KGRN "%s" KNRM " Hits: %d\n", results[i].name, results[i].hits);
            }
        }

        printf("\nCzas szukania: %fs\n", (endTime - beginTime));
    }


    printf(KRED "Proces %d konczy dzialanie.\n" KNRM, mynum);
    //if (mynum == 0)
        MPI_Finalize();

    //return 0;
    exit(0);
}