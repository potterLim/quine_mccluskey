#include <stdio.h>
#include <stdlib.h>

#include "quine_mccluskey.h"

static void add_term(term_t** array, int* count, int* capacity, int bits, int mask)
{
    int new_capacity;
    term_t* temp;

    if (*count >= *capacity) {
        new_capacity = *capacity * 2;
        temp = (term_t*)realloc(*array, new_capacity * sizeof(term_t));
        if (temp == NULL) {
            perror("Memory reallocation error");
            exit(EXIT_FAILURE);
        }

        *array = temp;
        *capacity = new_capacity;
    }

    (*array)[*count].bits = bits;
    (*array)[*count].mask = mask;
    (*array)[*count].combined = 0;
    ++*count;
}

static int already_exists(term_t* terms, int term_count, int bits, int mask)
{
    int i;

    for (i = 0; i < term_count; ++i) {
        if (terms[i].bits == bits && terms[i].mask == mask) {
            return 1;
        }
    }

    return 0;
}

static int compute_prime_implicants(const int minterms[], int num_minterms, term_t** prime_implicants_ptr)
{
    term_t* current_terms;
    int current_capacity = 16;
    int current_count = 0;
    int i, j, diff;
    term_t* next_terms;
    int next_capacity = 16;
    int next_count;
    term_t* prime_implicants;
    int prime_capacity = 16;
    int prime_count = 0;

    current_terms = (term_t*)malloc(current_capacity * sizeof(term_t));
    if (current_terms == NULL) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < num_minterms; ++i) {
        add_term(&current_terms, &current_count, &current_capacity, minterms[i], 0);
    }

    prime_implicants = (term_t*)malloc(prime_capacity * sizeof(term_t));
    if (prime_implicants == NULL) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }

    while (current_count > 0) {
        next_terms = (term_t*)malloc(next_capacity * sizeof(term_t));
        if (next_terms == NULL) {
            perror("Memory allocation error");
            exit(EXIT_FAILURE);
        }

        next_count = 0;
        for (i = 0; i < current_count; ++i) {
            for (j = i + 1; j < current_count; ++j) {
                if (current_terms[i].mask == current_terms[j].mask) {
                    diff = current_terms[i].bits ^ current_terms[j].bits;
                    if (diff && ((diff & (diff - 1)) == 0)) {
                        current_terms[i].combined = 1;
                        current_terms[j].combined = 1;
                        if (!already_exists(next_terms, next_count, current_terms[i].bits & current_terms[j].bits, current_terms[i].mask | diff)) {
                            add_term(&next_terms, &next_count, &next_capacity, current_terms[i].bits & current_terms[j].bits, current_terms[i].mask | diff);
                        }
                    }
                }
            }
        }

        for (i = 0; i < current_count; ++i) {
            if (!current_terms[i].combined) {
                if (!already_exists(prime_implicants, prime_count, current_terms[i].bits, current_terms[i].mask)) {
                    add_term(&prime_implicants, &prime_count, &prime_capacity, current_terms[i].bits, current_terms[i].mask);
                }
            }
        }

        free(current_terms);
        current_terms = next_terms;
        current_count = next_count;
        current_capacity = next_capacity;
    }

    *prime_implicants_ptr = prime_implicants;
    return prime_count;
}

static int covers_term(const term_t* term, int minterm)
{
    return ((minterm & ~(term->mask)) == (term->bits & ~(term->mask)));
}

static void print_term_expression(const term_t* term, int num_vars)
{
    int i;
    int printed = 0;
    int bit_mask;

    for (i = num_vars - 1; i >= 0; --i) {
        bit_mask = 1 << i;
        if (!(term->mask & bit_mask)) {
            if (printed) {
                printf("⋅");
            }

            if (term->bits & bit_mask) {
                printf("x%d", i);
            } else {
                printf("x%d'", i);
            }
            printed = 1;
        }
    }

    if (!printed) {
        printf("1");
    }
}

static void print_term_with_parens(const term_t* term, int num_vars)
{
    printf("(");
    print_term_expression(term, num_vars);
    printf(")");
}

void perform_quine_mccluskey(const int minterms[], int num_minterms, int num_vars)
{
    term_t* prime_implicants;
    int num_prime;
    int i, j;
    int* selected;
    int* covered;
    int all_covered;
    int best_index;
    int best_count;
    int count;
    int first;

    num_prime = compute_prime_implicants(minterms, num_minterms, &prime_implicants);
    selected = (int*)malloc(num_prime * sizeof(int));
    if (selected == NULL) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < num_prime; ++i) {
        selected[i] = 0;
    }

    covered = (int*)malloc(num_minterms * sizeof(int));
    if (covered == NULL) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < num_minterms; ++i) {
        covered[i] = 0;
    }

    for (i = 0; i < num_minterms; ++i) {
        count = 0;
        best_index = -1;
        for (j = 0; j < num_prime; ++j) {
            if (covers_term(&prime_implicants[j], minterms[i])) {
                ++count;
                best_index = j;
            }
        }

        if (count == 1 && best_index >= 0) {
            selected[best_index] = 1;
            for (int k = 0; k < num_minterms; ++k) {
                if (covers_term(&prime_implicants[best_index], minterms[k])) {
                    covered[k] = 1;
                }
            }
        }
    }

    while (1) {
        all_covered = 1;
        for (i = 0; i < num_minterms; ++i) {
            if (!covered[i]) {
                all_covered = 0;
                break;
            }
        }
        if (all_covered) {
            break;
        }

        best_index = -1;
        best_count = 0;

        for (i = 0; i < num_prime; ++i) {
            if (!selected[i]) {
                count = 0;
                for (j = 0; j < num_minterms; ++j) {
                    if (!covered[j] && covers_term(&prime_implicants[i], minterms[j])) {
                        ++count;
                    }
                }
                if (count > best_count) {
                    best_count = count;
                    best_index = i;
                }
            }
        }

        if (best_index >= 0) {
            selected[best_index] = 1;
            for (i = 0; i < num_minterms; ++i) {
                if (!covered[i] && covers_term(&prime_implicants[best_index], minterms[i])) {
                    covered[i] = 1;
                }
            }
        } else {
            break;
        }
    }

    printf("=== Minimized Result (SOP form) ===\n");
    printf("F(");
    for (i = num_vars - 1; i >= 0; --i) {
        printf("x%d", i);
        if (i > 0) {
            printf(", ");
        }
    }
    printf(") = ∑m(");
    for (i = 0; i < num_minterms; ++i) {
        printf("%d", minterms[i]);
        if (i < num_minterms - 1) {
            printf(", ");
        }
    }
    printf(")\n\n");
    printf("F = ");
    first = 1;
    for (i = 0; i < num_prime; ++i) {
        if (selected[i]) {
            if (!first) {
                printf(" + ");
            }
            print_term_with_parens(&prime_implicants[i], num_vars);
            first = 0;
        }
    }
    printf("\n");
    
    free(prime_implicants);
    prime_implicants = NULL;
    free(selected);
    selected = NULL;
    free(covered);
    covered = NULL;
}
