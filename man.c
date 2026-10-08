#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
//#include <sys/types.h>

typedef struct {
    unsigned char **data;
    int rows;
    int cols;
} matrix_t;

typedef struct {
    char **data;
    int rows;
    int cols;
} matrix_t_for_d;

char* read_file(const char *path_to_input_file );
void write_file(const char *path_to_output_file, const matrix_t *a, const matrix_t *b, const matrix_t *c);
void fill_matrix(char *buf, matrix_t *a, matrix_t *b, matrix_t *c, matrix_t_for_d *d);
void discrete_convolution(matrix_t *a, matrix_t *b, matrix_t *c, matrix_t_for_d *d);
int read_argv(const int argc, char **argv, char **path_to_input_file, char **path_to_out_file); //Функция выделяет память. Надо вызвать free.
unsigned char** matrix_create(int rows, int cols);
char** matrix_d_create(int rows, int cols);
void matrix_free(matrix_t *m);
void matrix_print(matrix_t *m);
void matrix_d_free(matrix_t_for_d *m);

int main(int argc, char** argv){
    char *path_to_input_file = NULL;
    char *path_to_out_file = NULL;
    matrix_t a, b, c;
    matrix_t_for_d d;
    int res_read_argv;
    res_read_argv = read_argv(argc, argv, &path_to_input_file, &path_to_out_file);
    if (res_read_argv == 10){
        fprintf(stderr, "Usage: %s -i <path_to_input_file> -o <path_to_out_file>\n", argv[0]);
        exit(10);
    }
    //printf("output: %s\ninput: %s\n", path_to_out_file, path_to_input_file);

    char *buf = read_file(path_to_input_file);
    fill_matrix(buf, &a, &b, &c, &d);
    free(buf);
    
    discrete_convolution(&a, &b, &c, &d);
    write_file(path_to_out_file, &a, &b, &c);

    matrix_free(&a);
    matrix_free(&b);
    matrix_free(&c);
    matrix_d_free(&d);
    return 0;
}

unsigned char** matrix_create(int rows, int cols){
    
        unsigned char **data = (unsigned char **)malloc(rows * sizeof (unsigned char *));  
        if(data == NULL){ 
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        for(int i = 0; i < rows; i++){
            data[i] = (unsigned char *)calloc(cols, sizeof(unsigned char));
            if(data[i] == NULL){
                perror("calloc");
                for (int k = 0; k < i; k++){  
                    free(data[k]);
                    free(data);
                    exit(EXIT_FAILURE);
                }
            }
        }
        return data;
}

char** matrix_d_create(int rows, int cols){
    
         char **data = (char **)malloc(rows * sizeof (char *));  
        if(data == NULL){ 
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        for(int i = 0; i < rows; i++){
            data[i] = (char *)calloc(cols, sizeof(char));
            if(data[i] == NULL){
                perror("calloc");
                for (int k = 0; k < i; k++){  
                    free(data[k]);
                    free(data);
                    exit(EXIT_FAILURE);
                }
            }
        }
        return data;
}

void matrix_free(matrix_t *m) {
    if (!m || !m->data) return;
    for (int i = 0; i < m->rows; i++)
        free(m->data[i]);
    free(m->data);
    m->data = NULL;
    m->rows = m->cols = 0;
}

void matrix_d_free(matrix_t_for_d *m) {
    if (!m || !m->data) return;
    for (int i = 0; i < m->rows; i++)
        free(m->data[i]);
    free(m->data);
    m->data = NULL;
    m->rows = m->cols = 0;
}

void matrix_print(matrix_t *m){
    printf("rows: %d, cols: %d\n", m->rows, m->cols);
    for(int i = 0; i < m->rows; i++){
        for(int j = 0; j < m->cols; j++){
            printf("%d ", m->data[i][j]);
        }
        printf("\n");
    }
}

void matrix_d_print(matrix_t_for_d *m){
    printf("rows: %d, cols: %d\n", m->rows, m->cols);
    for(int i = 0; i < m->rows; i++){
        for(int j = 0; j < m->cols; j++){
            printf("%d ", m->data[i][j]);
        }
        printf("\n");
    }
}

void fill_matrix(char *buf, matrix_t *a, matrix_t *b, matrix_t *c, matrix_t_for_d *d){
    int H, W;
    memcpy(&H, buf,     sizeof(int));
    memcpy(&W, buf + 4, sizeof(int));

    a->rows = b->rows = c->rows = H;
    a->cols = b->cols = c->cols = W;

    a->data = matrix_create(H, W);
    b->data = matrix_create(H, W);
    c->data = matrix_create(H, W);

    unsigned char *abc = (unsigned char *)buf + 2 * sizeof(int);   // начало A1

    for (int i = 0; i < H; i++){
        for (int j = 0; j < W; j++){
            int k = i * W + j;
            a->data[i][j] = (unsigned char)abc[k * 3 + 0];   // A_k
            b->data[i][j] = (unsigned char)abc[k * 3 + 1];   // B_k
            c->data[i][j] = (unsigned char)abc[k * 3 + 2];   // C_k
        }
    }

    //Читаем DH и DW (short, 2 байта каждое) 
    unsigned char *dhead = abc + H * W * 3;

    short DH, DW;
    memcpy(&DH, dhead,     sizeof(short));
    memcpy(&DW, dhead + 2, sizeof(short));

    d->rows = DH;
    d->cols = DW;

    d->data = matrix_d_create(DH, DW);

    char *ddata = (char*)dhead + 2 * sizeof(short);    // после DH и DW

    for (int i = 0; i < DH; i++){
        for (int j = 0; j < DW; j++){
            d->data[i][j] = (char)ddata[i * DW + j];
        }
    }


    // matrix_print(a);
    // matrix_print(b);
    // matrix_print(c);
    // matrix_d_print(d);
}

void write_file(const char *path_to_output_file, const matrix_t *a, const matrix_t *b, const matrix_t *c){
    FILE *fd = fopen(path_to_output_file, "wb");
    if(fd == NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    unsigned char *buff = (unsigned char *)malloc(sizeof(unsigned char) * 3 * a->cols * a->rows);
    if(buff == NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    int H = a->rows;
    int W = a->cols;
    fwrite(&H, sizeof(int), 1, fd);
    fwrite(&W, sizeof(int), 1, fd);
    int index = 0;
    for(int i = 0; i < a->rows; i++){
        for(int j = 0; j < a->cols; j++){
            buff[index++] = a->data[i][j];
            buff[index++] = b->data[i][j];
            buff[index++] = c->data[i][j];
        }
    }
    if(fwrite(buff, 1, 3 * a->cols * a->rows, fd) !=  3 * a->cols * a->rows){
        perror("fwrite");
        free(buff);
        fclose(fd);
        exit(EXIT_FAILURE);
    }

    free(buff);
    fclose(fd);
}

char* read_file(const char *path_to_input_file){
    struct stat st;
    if (stat(path_to_input_file, &st) == -1){
        perror("stat");
        exit(EXIT_FAILURE);
    }
    if (!S_ISREG(st.st_mode)){
        fprintf(stderr, "%s is not regular file", path_to_input_file);
        exit(EXIT_FAILURE);
    }
    if (st.st_size == 0) { fprintf(stderr, "empty file\n"); exit(EXIT_FAILURE); }
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
    //printf("size: %lld\n", ( long long)st.st_size);
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


// Время - O(H*W * D*H * D*W)  Память -  O(3*H*W) или O(H*W)   дополнительно хранится три матрицы для результата a_out, b_out, c_out.
void discrete_convolution(matrix_t *a, matrix_t *b, matrix_t *c, matrix_t_for_d *d){  
    int rH = d->rows / 2;
    int rW = d->cols / 2;

    // Так называемые временные матрицы для результата — исходные нужны целиком,
    matrix_t a_out, b_out, c_out;

    a_out.rows = a->rows; 
    b_out.rows = b->rows;  
    c_out.rows = c->rows;  
    c_out.cols = c->cols;
    b_out.cols = b->cols;
    a_out.cols = a->cols;

    a_out.data = matrix_create(a_out.rows, a_out.cols);
    b_out.data = matrix_create(b_out.rows, b_out.cols);
    c_out.data = matrix_create(c_out.rows, c_out.cols);

    for (int i = 0; i < a->rows; i++){
        for (int j = 0; j < a->cols; j++){
            int sum_a = 0, sum_b = 0, sum_c = 0;

            for (int di = -rH; di <= rH; di++){
                for (int dj = -rW; dj <= rW; dj++){
                    int si = i + di;
                    int sj = j + dj;

                    // элементы за пределами матрицы = 0  то пропускаем
                    if (si < 0 || si >= a->rows || sj < 0 || sj >= a->cols)
                        continue;

                    // ядро знаковое то приводим к int как signed char
                    int d_val = (int)d->data[di + rH][dj + rW];

                    sum_a += (int)a->data[si][sj] * d_val;
                    sum_b += (int)b->data[si][sj] * d_val;
                    sum_c += (int)c->data[si][sj] * d_val;
                }
            }

            // Постобработка
            if (sum_a > 255) sum_a %= 251;
            else if (sum_a < 0) sum_a = (-sum_a) % 241;

            if (sum_b > 255) sum_b %= 251;
            else if (sum_b < 0) sum_b = (-sum_b) % 241;

            if (sum_c > 255) sum_c %= 251;
            else if (sum_c < 0) sum_c = (-sum_c) % 241;

            a_out.data[i][j] = (unsigned char)sum_a;
            b_out.data[i][j] = (unsigned char)sum_b;
            c_out.data[i][j] = (unsigned char)sum_c;
        }
    }

    matrix_free(a);
    matrix_free(b);
    matrix_free(c);

    *a = a_out;
    *b = b_out;
    *c = c_out;
}

