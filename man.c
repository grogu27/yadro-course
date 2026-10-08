#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>

char* read_file(const char *path_to_input_file, int *cols, int *rows );
void write_file();
void fill_matrix();
void discrete_convolution();
int read_argv(const int argc, char **argv, char **path_to_input_file, char **path_to_out_file); //Функция выделяет память. Надо вызвать free.

int main(int argc, char** argv){
    char *path_to_input_file = NULL;
    char *path_to_out_file = NULL;
    int rows;
    int cols;
    int res_read_argv;
    res_read_argv = read_argv(argc, argv, &path_to_input_file, &path_to_out_file);
    if (res_read_argv == 10){
        fprintf(stderr, "Usage: %s -i <path_to_input_file> -o <path_to_out_file>\n", argv[0]);
        exit(10);
    }
    //printf("output: %s\ninput: %s\n", path_to_out_file, path_to_input_file);

    char buf = read_file(path_to_input_file, &cols, &rows);
    printf("cols: %d, rows: %d\n" );
    return 0;
}

char* read_file(const char *path_to_input_file, int *cols, int *rows){
    struct stat st;
    if (stat(path_to_input_file, &st) == -1){
        perror("stat");
        exit(EXIT_FAILURE);
    }
    if (S_ISREG(st.st_mode) != 1){
        fprintf(stderr, "%s is not regular file", path_to_input_file);
        exit(EXIT_FAILURE);
    }
    char *buffer = malloc(st.st_size);
    if(buffer == NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    FILE* fd = fopen(path_to_input_file, "rb");
    if (fd == NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    if(fread(buffer, 1, st.st_size, fd) != st.st_size){
        perror("fread");
        exit(EXIT_FAILURE);
    }
    fclose(fd);
    return buffer;
}


int read_argv(const int argc, char **argv, char **path_to_input_file, char **path_to_out_file){
    if (argc != 5) {
        return 10;
    }
    for (int i = 1; i < argc; i++){
        if (strcmp(argv[i], "-i") == 0){
            if (i + 1 >= argc) return 10;
            *path_to_input_file = argv[++i];
        }
        else if (strcmp(argv[i], "-o") == 0){
            if (i + 1 >= argc) return 10;
            *path_to_out_file = argv[++i];
        }
        else
            return 10;
    }
    if (*path_to_input_file == NULL || *path_to_out_file == NULL)
        return 10;
    return 0;
}
