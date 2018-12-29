#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char bibliotekaPath[] = "../biblioteka/";

int main(int argc, char *argv[]) {
    DIR *biblioteka;
    struct dirent *dir;

    if ( argc != 2 ) {
        printf("Program potrzebuje frazy do wyszukiwania w bibliotece.\n");
        return EXIT_FAILURE;
    }

    biblioteka = opendir(bibliotekaPath);

    printf("Foogle - wyszukiwarka fraz w bibliotece\n");

    if (biblioteka) {
        while ((dir = readdir(biblioteka)) != NULL) {
            if (dir->d_type == DT_DIR) continue; // . i .. nas nie interesuja

            char filePath[100];
            strcpy(filePath, bibliotekaPath);

            char const* const fileName = strcat(filePath, dir->d_name);
            printf("%s\n", fileName);

            FILE* file = fopen(fileName, "r");

            if (file != NULL) {
                char line[256];

                while (fgets(line, sizeof(line), file)) {
                    if (strstr(line, argv[1])){
                        fprintf(stdout, "%s", line);
                    }
                }
                fclose(file);
            }
        }
        closedir(biblioteka);
    }

    return 0;
}