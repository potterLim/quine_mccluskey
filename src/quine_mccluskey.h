#ifndef QUINE_MCCLUSKEY_H
#define QUINE_MCCLUSKEY_H

#define MAX_VARIABLES 16

typedef struct term {
    int bits;
    int mask;
    int combined;
} term_t;

void perform_quine_mccluskey(const int minterms[], int num_minterms, int num_vars);

#endif /* QUINE_MCCLUSKEY_H */
