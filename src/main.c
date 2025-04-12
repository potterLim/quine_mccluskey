#include <stdio.h>
#include <stdlib.h>

#include "quine_mccluskey.h"

static int read_input(int** out_minterms, int* out_num_variables);

int main(void)
{
    int num_variables;
    int* minterms;
    int minterm_count;
    minterm_count = read_input(&minterms, &num_variables);
    if (minterm_count <= 0) {
        printf("No minterms to minimize.\n");
        return 0;
    }
    perform_quine_mccluskey(minterms, minterm_count, num_variables);
    free(minterms);
    minterms = NULL;
    return 0;
}

static int read_input(int** out_minterms, int* out_num_variables)
{
    int input_value;
    int max_minterm;
    int max_minterm_count;
    int valid_count = 0;
    int* temp;

    printf("Number of variables (1 ~ %d): ", MAX_VARIABLES);
    if (scanf("%d", out_num_variables) != 1) {
        return 0;
    }
    if (*out_num_variables <= 0 || *out_num_variables > MAX_VARIABLES) {
        return 0;
    }

    max_minterm = (1 << *out_num_variables) - 1;
    max_minterm_count = max_minterm + 1;

    temp = (int*)malloc(max_minterm_count * sizeof(int));
    if (temp == NULL) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }

    printf("Enter minterms (0 ~ %d, max %d values, end with -1):\n", max_minterm, max_minterm_count);

    while (valid_count < max_minterm_count) {
        printf("minterm #%d: ", valid_count + 1);
        if (scanf("%d", &input_value) != 1) {
            break;
        }
        if (input_value == -1) {
            break;
        }
        if (input_value < 0 || input_value > max_minterm) {
            printf("Invalid minterm: %d (must be in range 0 ~ %d)\n", input_value, max_minterm);
            continue;
        }
        temp[valid_count] = input_value;
        valid_count++;
    }

    *out_minterms = temp;
    return valid_count;
}

