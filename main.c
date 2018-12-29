#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"

const char bibliotekaPath[] = "../biblioteka/";

int main(int argc, char *argv[]) {
    DIR *biblioteka;
    struct dirent *dir;

    if ( argc != 2 ) {
        printf("Program potrzebuje frazy do wyszukiwania w bibliotece.\n");
        return EXIT_FAILURE;
    }

    clock_t beginTime = clock();

    biblioteka = opendir(bibliotekaPath);

    printf("Foogle - wyszukiwarka fraz w bibliotece\n");

    if (biblioteka) {
        while ((dir = readdir(biblioteka)) != NULL) {
            if (dir->d_type == DT_DIR) continue; // . i .. nas nie interesuja

            char filePath[100];
            strcpy(filePath, bibliotekaPath);

            char const* const fileName = strcat(filePath, dir->d_name);
            //printf("\x1B[31m" "%s\n" "\x1B[0m", fileName);

            FILE* file = fopen(fileName, "r");

            if (file != NULL) {
                char line[256];
                int count = 0;

                while (fgets(line, sizeof(line), file)) {
                    if (strstr(line, argv[1])){
                        //fprintf(stdout, "%s", line);
                        count++;
                    }
                }

                if (count) {
                    printf(KGRN "%s" KNRM " Hits: %d\n", dir->d_name, count);
                } else {
                    printf(KRED "%s\n", dir->d_name);
                }

                fclose(file);
            }
        }
        closedir(biblioteka);
    }

    clock_t endTime = clock();

    printf("Czas wykonania: %fs", (double)(endTime - beginTime) / CLOCKS_PER_SEC);

    return 0;
}