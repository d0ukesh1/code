#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <openssl/rand.h>
#include <time.h>

#define TABLE_SIZE (1 << BLOCK_SIZE) 
#define BLOCK_SIZE 8               
#define OUTPUT "output.txt"         

#define GET_FIRST_4_BITS(input)  (((input) >> 12) & 0xF)
#define GET_SECOND_4_BITS(input) (((input) >> 8) & 0xF)
#define GET_THIRD_4_BITS(input)  (((input) >> 4) & 0xF)
#define GET_FOURTH_4_BITS(input) ((input) & 0xF)

#define SET_FIRST_4_BITS(input, value)  (((input) & 0x0FFF) | (((value) & 0xF) << 12))
#define SET_SECOND_4_BITS(input, value) (((input) & 0xF0FF) | (((value) & 0xF) << 8))
#define SET_THIRD_4_BITS(input, value)  (((input) & 0xFF0F) | (((value) & 0xF) << 4))
#define SET_FOURTH_4_BITS(input, value) (((input) & 0xFFF0) | ((value) & 0xF))

#define GET_FIRST_8_BITS(input)  (((input) >> 56) & 0xFF)
#define GET_SECOND_8_BITS(input) (((input) >> 48) & 0xFF)
#define GET_THIRD_8_BITS(input)  (((input) >> 40) & 0xFF)
#define GET_FOURTH_8_BITS(input) (((input) >> 32) & 0xFF)
#define GET_FIFTH_8_BITS(input)  (((input) >> 24) & 0xFF)
#define GET_SIXTH_8_BITS(input)  (((input) >> 16) & 0xFF)
#define GET_SEVENTH_8_BITS(input) (((input) >> 8) & 0xFF)
#define GET_EIGHTH_8_BITS(input) ((input) & 0xFF)

#define SET_FIRST_8_BITS(input, value)  (((input) & 0x00FFFFFFFFFFFFFF) | (((uint64_t)(value) & 0xFF) << 56))
#define SET_SECOND_8_BITS(input, value) (((input) & 0xFF00FFFFFFFFFFFF) | (((uint64_t)(value) & 0xFF) << 48))
#define SET_THIRD_8_BITS(input, value)  (((input) & 0xFFFF00FFFFFFFFFF) | (((uint64_t)(value) & 0xFF) << 40))
#define SET_FOURTH_8_BITS(input, value) (((input) & 0xFFFFFF00FFFFFFFF) | (((uint64_t)(value) & 0xFF) << 32))
#define SET_FIFTH_8_BITS(input, value)  (((input) & 0xFFFFFFFF00FFFFFF) | (((uint64_t)(value) & 0xFF) << 24))
#define SET_SIXTH_8_BITS(input, value)  (((input) & 0xFFFFFFFFFF00FFFF) | (((uint64_t)(value) & 0xFF) << 16))
#define SET_SEVENTH_8_BITS(input, value) (((input) & 0xFFFFFFFFFFFF00FF) | (((uint64_t)(value) & 0xFF) << 8))
#define SET_EIGHTH_8_BITS(input, value) (((input) & 0xFFFFFFFFFFFFFF00) | ((uint64_t)(value) & 0xFF))

uint8_t table[TABLE_SIZE][TABLE_SIZE];

uint8_t secure_random_byte() {
    uint8_t byte;
    if (RAND_bytes(&byte, 1) != 1) {
        fprintf(stderr, "Error: random byte creation error with OpenSSL\n");
        exit(EXIT_FAILURE);
    }
    return byte;
}

void secure_shuffle(uint8_t *array, uint16_t n) {
    if (n > 1) {
        for (uint16_t i = 0; i < n - 1; i++) {
            uint16_t j = i + secure_random_byte() % (n - i);
            uint8_t temp = array[j];
            array[j] = array[i];
            array[i] = temp;
        }
    }
}

uint8_t F_r(uint8_t x, uint8_t y, uint8_t table[TABLE_SIZE][TABLE_SIZE]) {
    return table[y][x];
}

uint8_t F_l(uint8_t x, uint8_t y, uint8_t table[TABLE_SIZE][TABLE_SIZE]) {
    return table[x][y];
}

uint16_t ROUND4B(uint16_t input, uint8_t table[TABLE_SIZE][TABLE_SIZE]) {
    
    uint16_t output = 0;
    uint8_t tmp_bits = 0; 

    output = SET_SECOND_4_BITS(output, F_l(GET_FIRST_4_BITS(input),GET_SECOND_4_BITS(input),table));
    
    output = SET_THIRD_4_BITS(output,F_l(GET_SECOND_4_BITS(output),GET_THIRD_4_BITS(input),table));
    
    output = SET_FOURTH_4_BITS(output,F_l(GET_THIRD_4_BITS(output),GET_FOURTH_4_BITS(input),table));
    
    tmp_bits = GET_THIRD_4_BITS(output);
    output = SET_THIRD_4_BITS(output,F_r(tmp_bits,GET_FOURTH_4_BITS(output),table));
    
    tmp_bits = GET_SECOND_4_BITS(output);
    output = SET_SECOND_4_BITS(output,F_r(tmp_bits,GET_THIRD_4_BITS(output),table));
    
    output = SET_FIRST_4_BITS(output,F_r(GET_FIRST_4_BITS(input),GET_SECOND_4_BITS(output),table));
    
    tmp_bits = GET_FIRST_4_BITS(output);
    output = SET_FIRST_4_BITS(output,F_r(tmp_bits,GET_FOURTH_4_BITS(output),table));
    
    tmp_bits = GET_SECOND_4_BITS(output);
    output = SET_SECOND_4_BITS(output,F_r(tmp_bits,GET_FOURTH_4_BITS(output),table));
    
    tmp_bits = GET_THIRD_4_BITS(output);
    output = SET_THIRD_4_BITS(output,F_r(tmp_bits,GET_FOURTH_4_BITS(output),table));
    
    return output;
}

uint64_t ROUND8B(uint64_t input, uint8_t table[TABLE_SIZE][TABLE_SIZE]) {
    
    uint64_t output = 0;
    uint8_t tmp_bits = 0;

    output = SET_SECOND_8_BITS(output, F_l(GET_FIRST_8_BITS(input), GET_SECOND_8_BITS(input), table));
    
    output = SET_THIRD_8_BITS(output, F_l(GET_SECOND_8_BITS(output), GET_THIRD_8_BITS(input), table));
    
    output = SET_FOURTH_8_BITS(output, F_l(GET_THIRD_8_BITS(output), GET_FOURTH_8_BITS(input), table));

    output = SET_FIFTH_8_BITS(output, F_l(GET_FOURTH_8_BITS(output), GET_FIFTH_8_BITS(input), table));

    output = SET_SIXTH_8_BITS(output, F_l(GET_FIFTH_8_BITS(output), GET_SIXTH_8_BITS(input), table));

    output = SET_SEVENTH_8_BITS(output, F_l(GET_SIXTH_8_BITS(output), GET_SEVENTH_8_BITS(input), table));

    output = SET_EIGHTH_8_BITS(output, F_l(GET_SEVENTH_8_BITS(output),GET_EIGHTH_8_BITS(input), table));

    tmp_bits = GET_SEVENTH_8_BITS(output);
    output = SET_SEVENTH_8_BITS(output, F_r(tmp_bits, GET_EIGHTH_8_BITS(output),table));
    
    tmp_bits = GET_SIXTH_8_BITS(output);
    output = SET_SIXTH_8_BITS(output, F_r(tmp_bits, GET_SEVENTH_8_BITS(output), table));
    
    tmp_bits = GET_FIFTH_8_BITS(output);
    output = SET_FIFTH_8_BITS(output, F_r(tmp_bits, GET_SIXTH_8_BITS(output), table));
    
    tmp_bits = GET_FOURTH_8_BITS(output);
    output = SET_FOURTH_8_BITS(output, F_r(tmp_bits, GET_FIFTH_8_BITS(output), table));
    
    tmp_bits = GET_THIRD_8_BITS(output);
    output = SET_THIRD_8_BITS(output, F_r(tmp_bits, GET_FOURTH_8_BITS(output), table));
    
    tmp_bits = GET_SECOND_8_BITS(output);
    output = SET_SECOND_8_BITS(output, F_r(tmp_bits, GET_THIRD_8_BITS(output), table));
    
    output = SET_FIRST_8_BITS(output, F_r(GET_FIRST_8_BITS(input), GET_SECOND_8_BITS(output), table));
    
    tmp_bits = GET_FIRST_8_BITS(output);
    output = SET_FIRST_8_BITS(output, F_r(tmp_bits, GET_EIGHTH_8_BITS(output), table));
    
    tmp_bits = GET_SECOND_8_BITS(output);
    output = SET_SECOND_8_BITS(output, F_r(tmp_bits, GET_EIGHTH_8_BITS(output), table));
    
    tmp_bits = GET_THIRD_8_BITS(output);
    output = SET_THIRD_8_BITS(output, F_r(tmp_bits, GET_EIGHTH_8_BITS(output), table));
    
    tmp_bits = GET_FOURTH_8_BITS(output);
    output = SET_FOURTH_8_BITS(output, F_r(tmp_bits, GET_EIGHTH_8_BITS(output), table));

    tmp_bits = GET_FIFTH_8_BITS(output);
    output = SET_FIFTH_8_BITS(output, F_r(tmp_bits, GET_EIGHTH_8_BITS(output), table));

    tmp_bits = GET_SIXTH_8_BITS(output);
    output = SET_SIXTH_8_BITS(output, F_r(tmp_bits, GET_EIGHTH_8_BITS(output), table));

    tmp_bits = GET_SEVENTH_8_BITS(output);
    output = SET_SEVENTH_8_BITS(output, F_r(tmp_bits, GET_EIGHTH_8_BITS(output), table));
    
    return output;
}

void to_binary_string_4bit(uint8_t value, char *buffer, size_t buffer_size) {
    if (buffer_size < 5) { 
        fprintf(stderr, "Error: buffer overflow. Need at least 5 bytes.\n");
        exit(EXIT_FAILURE);
    }
    buffer[4] = '\0'; 
    for (int i = 3; i >= 0; i--) {
        buffer[i] = (value & 1) ? '1' : '0';
        value >>= 1;
    }
}

void to_binary_string_8bit(uint8_t value, char *buffer, size_t buffer_size) {
    if (buffer_size < 9) {
        fprintf(stderr, "Error: buffer overflow. Need at least 9 bytes.\n");
        exit(EXIT_FAILURE);
    }
    buffer[8] = '\0';
    for (int i = 7; i >= 0; i--) {
        buffer[i] = (value & 1) ? '1' : '0';
        value >>= 1;
    }
}

void to_binary_string_16bit(uint16_t value, char *buffer, size_t buffer_size) {
    if (buffer_size < 17) { 
        fprintf(stderr, "Error: binary output is broken.\n");
        exit(EXIT_FAILURE);
    }
    buffer[16] = '\0'; 
    for (int i = 15; i >= 0; i--) {
        buffer[i] = (value & 1) ? '1' : '0';
        value >>= 1;
    }
}


void to_binary_string_64bit(uint64_t value, char *buffer, size_t buffer_size) {
    if (buffer_size < 65) {
        fprintf(stderr, "Error: buffer overflow. Need at least 65 bytes.\n");
        exit(EXIT_FAILURE);
    }
    buffer[64] = '\0';
    for (int i = 63; i >= 0; i--) {
        buffer[i] = (value & 1) ? '1' : '0';
        value >>= 1;
    }
}

void table_and_bf_64b(const char *filename, uint8_t table[TABLE_SIZE][TABLE_SIZE]) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: cant open file %s for write\n", filename);
        exit(EXIT_FAILURE);
    }

    char binary_input[65], binary_output[65];
    char binary_table_entry[9];

    fprintf(file, "|  F table (binary)  |\n");
    for (uint16_t i = 0; i < TABLE_SIZE; i++) {
        for (uint16_t j = 0; j < TABLE_SIZE; j++) {
            to_binary_string_8bit(table[i][j], binary_table_entry, sizeof(binary_table_entry));
            fprintf(file, "%s ", binary_table_entry);
        }
        fprintf(file, "\n");
    }

    fprintf(file, "\n| Input (binary)  | Output (binary) |\n");

    for (uint64_t input = 0; input <= 0xFFFFFFFFFFFFFFFF; input++) {
        uint64_t output = ROUND8B(input, table);

        to_binary_string_64bit(input, binary_input, sizeof(binary_input));
        to_binary_string_64bit(output, binary_output, sizeof(binary_output));

        fprintf(file, " %s | %s\n", binary_input, binary_output);
        if (input == 0xFFFFFFFFFFFFFFFF) break;
    }

    fclose(file);
    printf("Table and BF written to %s\n", filename);
}

void table_and_bf_16b(const char *filename, uint8_t table[TABLE_SIZE][TABLE_SIZE]) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: cant open file %s for write\n", filename);
        exit(EXIT_FAILURE);
    }

    char binary_input[17], binary_output[17];
    char binary_table_entry[5];

    fprintf(file, "|  F table (binary)  |\n");
    for (uint16_t i = 0; i < TABLE_SIZE; i++) {
        for (uint16_t j = 0; j < TABLE_SIZE; j++) {
            to_binary_string_4bit(table[i][j], binary_table_entry, sizeof(binary_table_entry));
            fprintf(file, "%s ", binary_table_entry);
        }
        fprintf(file, "\n");
    }

    fprintf(file, "\n| Input (binary)  | Output (binary) |\n");

    for (uint16_t input = 0; input <= 0xFFFF; input++) {
        uint16_t output = ROUND4B(input, table);

        to_binary_string_16bit(input, binary_input, sizeof(binary_input));
        to_binary_string_16bit(output, binary_output, sizeof(binary_output));

        fprintf(file, " %s | %s\n", binary_input, binary_output);
        if (input == 0xFFFF) break;
    }

    fclose(file);
    printf("Table and BF written to %s\n", filename);
}

void analyze_balance(const char *filename, uint8_t table[TABLE_SIZE][TABLE_SIZE]) {
    FILE *file = fopen(filename, "a");
    if (!file) {
        fprintf(stderr, "Error: can't open file %s for balance check\n", filename);
        exit(EXIT_FAILURE);
    }

    fprintf(file, "\n| Balance Analysis |\n");

    uint32_t count_zeros = 0;
    uint32_t count_ones = 0;

    for (uint16_t input = 0; input <= 0xFFFF; input++) {
        uint16_t output = ROUND4B(input, table);
        for (int bit = 0; bit < 16; bit++) {
            if (output & (1 << bit)) {
                count_ones++;
            } else {
                count_zeros++;
            }
        }
        if(input == 0xFFFF) break;
    }

    fprintf(file, "Number of zeros: %u\n", count_zeros);
    fprintf(file, "Number of ones: %u\n", count_ones);

    if (count_zeros == count_ones) {
        fprintf(file, "The function is balanced.\n");
    } else {
        fprintf(file, "The function is NOT balanced.\n");
    }

    fclose(file);
    printf("Balance analysis written to %s\n", filename);
}

void fwht(int *a, int n) {
    for (int len = 1; len < n; len <<= 1) {
        for (int i = 0; i < n; i += 2 * len) {
            for (int j = 0; j < len; ++j) {
                int u = a[i + j];
                int v = a[i + j + len];
                a[i + j] = u + v;
                a[i + j + len] = u - v;
            }
        }
    }
}

int nonlinearity_bit(int *truthTable, int n) {
    int size = 1 << n;
    int *spectrum = (int *)malloc(size * sizeof(int));
    if (!spectrum) {
        perror("Error: buffer overflow.");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < size; ++i) {
        spectrum[i] = truthTable[i] ? 1 : -1;
    }

    fwht(spectrum, size);

    int maxWalsh = 0;
    for (int i = 0; i < size; ++i) {
        int absValue = abs(spectrum[i]);
        if (absValue > maxWalsh) {
            maxWalsh = absValue;
        }
    }

    free(spectrum);
    return (1 << (n - 1)) - (maxWalsh / 2);
}

int hamming_weight(int x) {
    int weight = 0;
    while (x) {
        weight += x & 1;
        x >>= 1;
    }
    return weight;
}

void analyze_correlation_immunity(const char *filename, uint8_t table[TABLE_SIZE][TABLE_SIZE]) {
    FILE *file = fopen(filename, "a");
    if (!file) {
        fprintf(stderr, "Error: can't open file %s for correlation immunity analysis\n", filename);
        exit(EXIT_FAILURE);
    }

    fprintf(file, "\n| Correlation Immunity Analysis |\n");

    int n = 16;
    int size = 1 << n;

    for (int bit = 0; bit < 16; ++bit) {
        int *truthTable = (int *)malloc(size * sizeof(int));
        if (!truthTable) {
            perror("Error: Memory allocation failed\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < size; ++i) {
            uint16_t output = ROUND4B((uint16_t)i, table);
            truthTable[i] = (output >> bit) & 1;
        }

        for (int i = 0; i < size; ++i) {
            truthTable[i] = truthTable[i] ? 1 : -1;
        }

        fwht(truthTable, size);

        int max_order = 0;
        for (int i = 0; i < size; i++) {
            int weight = hamming_weight(i);
            if (truthTable[i] == 0 && weight > max_order) {
                max_order = weight;
            }
        }

        fprintf(file, "Correlation immunity order for bit %d: %d\n", bit, max_order);

        free(truthTable);
    }

    fclose(file);
    printf("Correlation immunity analysis written to %s\n", filename);
}

void analyze_nonlinearity(const char *filename, uint8_t table[TABLE_SIZE][TABLE_SIZE]) {
    FILE *file = fopen(filename, "a");
    if (!file) {
        fprintf(stderr, "Error: can't open file %s for nonlinearity analysis\n", filename);
        exit(EXIT_FAILURE);
    }

    fprintf(file, "\n| Nonlinearity Analysis |\n");

    int n = 16;
    int size = 1 << n;

    for (int bit = 0; bit < 16; ++bit) {
        int *truthTable = (int *)malloc(size * sizeof(int));
        if (!truthTable) {
            perror("Error: Memory allocation failed\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < size; ++i) {
            uint16_t output = ROUND4B((uint16_t)i, table);
            truthTable[i] = (output >> bit) & 1;
        }

        int nonlinearity = nonlinearity_bit(truthTable, n);
        fprintf(file, "Nonlinearity for bit %d: %d\n", bit, nonlinearity);
        free(truthTable);
    }

    fclose(file);
    printf("Nonlinearity analysis written to %s\n", filename);
}

void analyze_avalanche_criteria(const char *filename, uint8_t table[TABLE_SIZE][TABLE_SIZE]) {
    FILE *file = fopen(filename, "a");
    if (!file) {
        fprintf(stderr, "Error: can't open file %s for avalanche analysis\n", filename);
        exit(EXIT_FAILURE);
    }

    fprintf(file, "\n| Avalanche and Strict Avalanche Criteria Analysis |\n");

    uint16_t input, flipped_input, output, flipped_output;
    uint16_t diff_output;
    int ac_total[16] = {0};
    int sac_total[16][16] = {0};

    for (input = 0; input <= 0xFFFF; ++input) {
        output = ROUND4B(input, table);

        for (int bit = 0; bit < 16; ++bit) {
            flipped_input = input ^ (1 << bit);
            flipped_output = ROUND4B(flipped_input, table);

            diff_output = output ^ flipped_output;

            for (int out_bit = 0; out_bit < 16; ++out_bit) {
                if (diff_output & (1 << out_bit)) {
                    ac_total[out_bit]++;
                }
            }

            for (int out_bit = 0; out_bit < 16; ++out_bit) {
                if (diff_output & (1 << out_bit)) {
                    sac_total[bit][out_bit]++;
                }
            }
        }

        if (input == 0xFFFF) break;
    }

    fprintf(file, "\nAvalanche Criterion (AC):\n");
    for (int out_bit = 0; out_bit < 16; ++out_bit) {
        double ac_percentage = (double)ac_total[out_bit] / (16.0 * 65536.0) * 100.0;
        fprintf(file, "Output bit %d: %.2f%%\n", out_bit, ac_percentage);
    }

    fprintf(file, "\nStrict Avalanche Criterion (SAC):\n");
    for (int in_bit = 0; in_bit < 16; ++in_bit) {
        for (int out_bit = 0; out_bit < 16; ++out_bit) {
            double sac_percentage = (double)sac_total[in_bit][out_bit] / 65536.0 * 100.0;
            fprintf(file, "Input bit %d -> Output bit %d: %.2f%%\n", in_bit, out_bit, sac_percentage);
        }
    }

    fclose(file);
    printf("Avalanche and SAC analysis written to %s\n", filename);
}

void analyze_resiliency(const char *filename, uint8_t table[TABLE_SIZE][TABLE_SIZE]) {
    FILE *file = fopen(filename, "a");
    if (!file) {
        fprintf(stderr, "Error: can't open file %s for resiliency analysis\n", filename);
        exit(EXIT_FAILURE);
    }

    fprintf(file, "\n| Resiliency Analysis |\n");

    int n = 16;
    int size = 1 << n;

    for (int bit = 0; bit < 16; ++bit) {
        int *truthTable = (int *)malloc(size * sizeof(int));
        if (!truthTable) {
            perror("Error: Memory allocation failed\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < size; ++i) {
            uint16_t output = ROUND4B((uint16_t)i, table);
            truthTable[i] = (output >> bit) & 1;
        }

        for (int i = 0; i < size; ++i) {
            truthTable[i] = truthTable[i] ? 1 : -1;
        }

        fwht(truthTable, size);

        int t = n;
        for (int i = 0; i < size; i++) {
            if (truthTable[i] != 0) {
                int weight = hamming_weight(i);
                if (weight < t) {
                    t = weight - 1;
                }
            }
        }

        fprintf(file, "Resiliency order for bit %d: %d\n", bit, t);

        free(truthTable);
    }

    fclose(file);
    printf("Resiliency analysis written to %s\n", filename);
}

void analyze_propagation_criterion(const char *filename, uint8_t table[TABLE_SIZE][TABLE_SIZE]) {
    FILE *file = fopen(filename, "a");
    if (!file) {
        fprintf(stderr, "Error: can't open file %s for propagation criterion analysis\n", filename);
        exit(EXIT_FAILURE);
    }

    fprintf(file, "\n| Propagation Criterion Analysis |\n");

    int n = 16;
    int size = 1 << n;

    for (int bit = 0; bit < 16; ++bit) {
        int *truthTable = (int *)malloc(size * sizeof(int));
        if (!truthTable) {
            perror("Error: Memory allocation failed\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < size; ++i) {
            uint16_t output = ROUND4B((uint16_t)i, table);
            truthTable[i] = (output >> bit) & 1;
        }

        int count = 0;
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < n; ++j) {
                int flipped_i = i ^ (1 << j);
                if (truthTable[i] != truthTable[flipped_i]) {
                    count++;
                }
            }
        }

        double propagation_ratio = (double)count / (size * n);
        fprintf(file, "Propagation criterion for bit %d: %.2f\n", bit, propagation_ratio);

        free(truthTable);
    }

    fclose(file);
    printf("Propagation criterion analysis written to %s\n", filename);
}

int compute_algebraic_degree(uint8_t *truth_table, int size) {
    uint8_t *poly = malloc(size * sizeof(uint8_t));
    if (!poly) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < size; i++) {
        poly[i] = truth_table[i];
    }

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if ((i & j) == i) {
                poly[j] ^= poly[i];
            }
        }
    }

    int max_degree = 0;
    for (int i = 0; i < size; i++) {
        if (poly[i] != 0) {
            int degree = __builtin_popcount(i);
            if (degree > max_degree) {
                max_degree = degree;
            }
        }
    }

    free(poly);
    return max_degree;
}

void analyze_algebraic_immunity(const char *filename, uint8_t table[TABLE_SIZE][TABLE_SIZE]) {
    FILE *file = fopen(filename, "a");
    if (!file) {
        fprintf(stderr, "Error: can't open file %s for algebraic immunity analysis\n", filename);
        exit(EXIT_FAILURE);
    }

    fprintf(file, "\n| Algebraic Immunity Analysis |\n");

    int n = 16;
    int size = 1 << n; 

    for (int bit = 0; bit < n; bit++) {
        uint8_t *truth_table = malloc(size * sizeof(uint8_t));
        if (!truth_table) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            fclose(file);
            return;
        }

        for (uint16_t input = 0; input <= 0xFFFF; input++) {
            uint16_t output = ROUND4B(input, table);
            truth_table[input] = (output >> bit) & 1;
            if(input == 0xFFFF) break;
        }

        int degree = compute_algebraic_degree(truth_table, size);
        int algebraic_immunity = (n - degree - 1) / 2;
        fprintf(file, "Output bit %d: Algebraic immunity = %d\n", bit, algebraic_immunity);

        free(truth_table);
    }

    fclose(file);
    printf("Algebraic immunity analysis written to %s\n", filename);
}

void encrypt_file(const char *filename, uint8_t table[TABLE_SIZE][TABLE_SIZE]) {
    FILE *file = fopen(filename, "rb+");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    long padded_size = (file_size + sizeof(uint64_t) - 1) / sizeof(uint64_t) * sizeof(uint64_t);
    uint64_t *buffer = calloc(padded_size / sizeof(uint64_t), sizeof(uint64_t));
    if (!buffer) {
        perror("Memory allocation error");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    size_t read_size = fread(buffer, 1, file_size, file);
    if (read_size != file_size) {
        perror("File reading error");
        free(buffer);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    clock_t start_time = clock();

    for (long i = 0; i < padded_size / sizeof(uint64_t); i++) {
        buffer[i] = ROUND8B(buffer[i], table);
    }

    clock_t end_time = clock();
    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    fseek(file, 0, SEEK_SET);
    size_t written_size = fwrite(buffer, 1, padded_size, file);
    if (written_size != padded_size) {
        perror("File writing error");
    }

    free(buffer);
    fclose(file);

    double file_size_MB = (double)file_size / (1024 * 1024);
    double encryption_speed = file_size_MB / time_spent;

    printf("File '%s' successfully encrypted in %.3f seconds.\n", filename, time_spent);
    printf("Encryption speed: %.3f MB/s.\n", encryption_speed);
}

/* gcc -o main main.c -lssl -lcrypto*/ 
int main() {

    for (uint16_t i = 0; i < TABLE_SIZE; i++) {
        table[0][i] = (uint8_t)i;
    }

    for (uint16_t i = 0; i < TABLE_SIZE; i++) {
        for (uint16_t j = 0; j < TABLE_SIZE; j++) {
            table[i][j] = table[0][j];
        }
        secure_shuffle(table[i], TABLE_SIZE);
    }
    printf("Table size: %d byte\n", (int)sizeof(table));

    if (BLOCK_SIZE == 4){
        printf("Test module ON\n");
        table_and_bf_16b(OUTPUT,table);
        analyze_balance(OUTPUT, table);
        analyze_nonlinearity(OUTPUT, table);
        analyze_avalanche_criteria(OUTPUT, table);
        analyze_correlation_immunity(OUTPUT, table);
        analyze_resiliency(OUTPUT,table);
        analyze_propagation_criterion(OUTPUT, table);
        analyze_algebraic_immunity(OUTPUT, table);
    } 
    else if(BLOCK_SIZE == 8){
        const char *filename = "test.txt";
        encrypt_file(filename, table);
    } 
    else printf("Incorrect BLOCK_SIZE!");

    printf("Press any key to esc...\n");

    getchar();
    return 0;
}