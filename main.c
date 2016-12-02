#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char SPLITTERS[] = {' ', '\t'};
char * NO_TYPE_NAME = "no_type";
char * FLOAT_TYPE_NAME = "float";
char * INT_TYPE_NAME = "int";
char * STR_TYPE_NAME = "str";

int is_splitter(char ch){
    for (int i=0; i < sizeof(SPLITTERS); i++){
        if (ch == SPLITTERS[i]){
            return 1;
        }
    }
    return 0;
}

struct Result{
    char type;
    char * type_name;
    void * buffer;
    int count;
    int step;
    int capacity;
};

void feel_result(struct Result * result){
    result->type = 0;
    result->count = 0;
    result->step = 0;
    result->buffer = NULL;
    result->type_name = NO_TYPE_NAME;
    result->capacity = 0;
}
//инициализация результата
void init_result(struct Result * result, int type, char * type_name, int count, int step, void * data) {
    result->type = type;
    result->type_name = type_name;
    result->count = count;
    result->step = step;
    int size = count * step;
    result->capacity = size * 4;
    result->buffer = malloc(result->capacity);
    memcpy(result->buffer, data, size);
}

// clear result
void free_result(struct Result * result){
    free(result->buffer);
    result->buffer = NULL;
    result->type = 0;
    result->count = 0;
    result->step = 0;
    result->capacity = 0;
}

//добавление в буффер рузультата
void append_data(struct Result * result, int count, void * data){
    void * old_buffer = result->buffer;
    int buffer_size = result->count * result->step;
    int data_size = count * result->step;

    //Достаточно данных в буффере
    if (buffer_size + data_size < result->capacity) {
        memcpy(((char*)result->buffer) + buffer_size, data, data_size);
    //выделем больше данных 
    } else {
        result->capacity = (buffer_size + data_size) * 2;
        result->buffer = malloc(result->capacity);
        memcpy(result->buffer, old_buffer, buffer_size);
        memcpy(((char*)result->buffer) + buffer_size, data, data_size);
        //чистим старый буфер
        free(old_buffer);
    }
    result->count += count;
}

//convert data
int convert_data(struct Result * result, void * buffer, int size) {
    //int1
    int int_value;
    float float_value;

    if (result->type == 1) {
        if (sscanf(buffer, "%i", &int_value) != 1) {
            fprintf(stderr, "\"%s\" cannot convet to int\n", (char *)buffer);
            return 1;
        }
        append_data(result, 1, &int_value);
    //float
    } else if (result->type == 2) {
        if (sscanf(buffer, "%f", &float_value) != 1) {
            fprintf(stderr, "\"%s\" cannot convet to float\n", (char *)buffer);
            return 1;
        }
        append_data(result, 1, &float_value);
    //string
    } else {
        append_data(result, size, buffer);
    }
    return 0;
}

// определение типа данных
int get_data_type(struct Result * result, void * buffer, int size) {
    int int_value;
    float float_value;
    // тип float
    if ((strchr(buffer, '.') != 0 || strchr(buffer, ',') != 0) && sscanf(buffer, "%f", &float_value) == 1){
        init_result(result, 2, FLOAT_TYPE_NAME, 1, sizeof(float), &float_value);
    // тип int
    } else if (sscanf(buffer, "%i", &int_value) == 1) {
        init_result(result, 1, INT_TYPE_NAME, 1, sizeof(int), &int_value);
    // тип строка
    } else {
        init_result(result, 3, STR_TYPE_NAME, size, 1, buffer);
    }
    return 0;
}

// чтение потока и парсинг данных
int read_stream(struct Result * result) {
    char buffer[200];
    char * pointer = buffer;
    char * end_of_buffer = &buffer[198];
    char ch;
    int res;

    while ((ch = getchar()) != EOF) {
        *(pointer++) = ch;
        //раздеитель или буффер пареполнен
        if (is_splitter(ch) || pointer == end_of_buffer) {
            *pointer = 0;
            //определяем тип
            if (!result->type) {
                if ((res = get_data_type(result, buffer, pointer - buffer)))
                    return res;
            //парсим данные заданного типа
            } else {
                //парсим или строки или даннын
                if (result->type == 3 || (pointer - buffer) > 1) {
                    if ((res = convert_data(result, buffer, pointer - buffer)))
                        return res;
                }
            }
            pointer = buffer;
        //завершение ввода
        } else if (ch == '\r' || ch == '\n') {
            if (result->type == 0)
                return get_data_type(result, buffer, (pointer - 1) - buffer);
            else
                return convert_data(result, buffer, (pointer - 1) - buffer);
        }
    }
    return 0;
}

int main()
{
    struct Result result;
    feel_result(&result);
    while (1) {
        printf("input array:\n");
        if (!read_stream(&result)) {
            printf("result type: %s count: %i size: %i\n", result.type_name, result.count, result.step * result.count);

            // обнулим строку
            if (result.type == 1) {
                for (int i = 0; i < result.count; i++) {
                    printf("%i ", ((int *)result.buffer)[i]);
                }
                printf("\n");
            } else if (result.type == 2) {
                for (int i = 0; i < result.count; i++) {
                    printf("%f ", ((float *)result.buffer)[i]);
                }
                printf("\n");
            } else {
                if (result.type == 3) {
                    char ch = 0;
                    append_data(&result, 1, &ch);
                }
                printf("\"%s\"\n", (char *)result.buffer);
            }
        }
        printf("for try again press 'enter' for exit press any key\n");
        if (getchar() != 10){
            return 0;
        }

        free_result(&result);
    }
    return 0;
}
