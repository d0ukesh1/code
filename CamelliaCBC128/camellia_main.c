/*
 *СДЕЛАНО В ДВА ФАЙЛА ДЛЯ УДОБСТВА, КОМПИЛИРОВАТЬ ВМЕСТЕ
 *СИНТАКСИС:
 *  gcc -c *.c
 *  gcc -o camellia camelia_main.o camellia_func.o
 *  ./camellia [encrypt|decrypt] [keylength] [key.file] [input.file] [output.file]
 * ВОТ ТАК:
 *  ./camellia encrypt 128 key.doc file.doc file.doc.enc
 *  ./camellia decrypt 128 key.doc file.doc.enc file.doc
 */

#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define CAMELLIA_TABLE_BYTE_LEN 272
#define CAMELLIA_TABLE_WORD_LEN (CAMELLIA_TABLE_BYTE_LEN / 4)   
typedef unsigned int KEY_TABLE_TYPE[CAMELLIA_TABLE_WORD_LEN];

/* дублируем структуру нашего ключа */
struct camellia_key_st 
{
    union{
        double d;   
        KEY_TABLE_TYPE rd_key;
        } u;
    int grand_rounds;
};
typedef struct camellia_key_st CAMELLIA_KEY;

#define BS 16 // Camellia128b / 16B размер блока

CAMELLIA_KEY key;

unsigned char iv[16];
random_iv(char iv[16]);

//расшифрование
int decrypt(int keylen, int infd, int outfd) {
    unsigned char inbuff[BS], outbuf[BS];
    int n = 0, outbufready = 0, prev_n = 0, padding = 0;

    while (1) {
        if (n) {
            bzero(outbuf, BS);
            camellia_cbc_alrorithm(inbuff, outbuf, BS, &key, iv, 0);
            //на случай ошибки
            outbufready = 1;
        }

        bzero(inbuff, BS);
        if ((n = read(infd, inbuff, BS)) == -1) {
            perror("read error");
            break;
        }

        if (!outbufready)
            continue;

        if (n && write(outfd, outbuf, BS) == -1)
            perror("write error");
        else if (!n) {
            padding = outbuf[BS - 1];
            if (BS - padding > 0 && write(outfd, outbuf, BS - padding) == -1)
                perror("write error");
            break;
        }

        prev_n = n;
    }

    return 1;
}
//зашифрование
int encrypt(int keylen, int infd, int outfd) {
    unsigned char inbuf[BS], outbuf[BS];
    int n = 0, i = 0;

    while (1) {
        bzero(inbuf, BS);
        if ((n = read(infd, inbuf, BS)) == -1) {
            perror("read error");
            break;
        }
        // если нужен отступ
        for(i = n; i < BS; i++)
            inbuf[i] = BS - n;

        bzero(outbuf, BS);
        camellia_cbc_alrorithm(inbuf, outbuf, BS, &key, iv, 1);
        
        if (write(outfd, outbuf, BS) == -1)
            perror("write error");

        if (n < BS)
            break;
    }

    return 1;
}

//объединяем и получаем результат
int main (int argc, char *argv[]) {
    int flags1 = 0, flags2 = 0, outfd = 0, infd = 0,keyfd=0, mode = 0;

    if (argv[1] != NULL) {
        if (strcmp(argv[1], "encrypt") == 0)
            mode = 1;

        if (strcmp(argv[1], "decrypt") == 0)
            mode = 2;

        //если пустое
        if (argv[2] == NULL || argv[3] == NULL || argv[4] == NULL)
            mode = 0;
    }
//если пустое
    if (mode == 0) {
        printf("Bad or incomplete parameters\n");
        printf("Syntax: encrypt/decrypt keylength infile outfile\n");
        return 1;
    }
//флаги
    flags1 = O_RDONLY;
    flags2 = O_RDONLY | O_WRONLY | O_CREAT;
//считываем агрументы
    int keylen = 0;
    sscanf(argv[2], "%d", &keylen);
    if (keylen != 128 && keylen != 192 && keylen != 256) {
        printf("Error: key length must be one of: 128, 192 or 256.\n");
        return 1;
    }
    if ((keyfd = open(argv[3], flags1, S_IRUSR | S_IWUSR)) == -1)
        perror("open input file error");
        
    if ((infd = open(argv[4], flags1, S_IRUSR | S_IWUSR)) == -1)
        perror("open input file error");

    if ((outfd = open(argv[5], flags2, S_IRUSR | S_IWUSR)) == -1)
        perror("open output file error");
//очищаем файл
    ftruncate(outfd, 0);
//берём ключ    
    char KEYFD[16];
    FILE *keyfile;
    keyfile = fopen(argv[3], "r");
    fgets(KEYFD, 16, keyfile);
    puts(KEYFD);
//передаем ключ
    set_key(KEYFD, keylen, &key);
//значение аргументов = режим
    if (mode == 1)
        encrypt(keylen, infd, outfd);
    else
        decrypt(keylen, infd, outfd);
//закрываем файлы
    close(keyfile);
    close(infd);
    fsync(outfd);
    close(outfd);
    
    return 0;
}