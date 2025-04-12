# Detailed Technical Documentation for Quine–McCluskey Minimization

This project implements the Quine–McCluskey algorithm for minimizing Boolean expressions given as minterms. This document explains only the core algorithmic logic, excluding input/output and exception handling details.

**[한국어 버전 README 읽기](README_ko.md)**

## Project Structure

### 1. Dynamic Array Management

**add_term Function**

This function dynamically adds a new term to an expanding array. Its core logic is to:
- Check if the current array is full.
- Reallocate the array to double its capacity if needed.
- Initialize the new term with the given bit pattern (`bits`), mask (`mask`), and a flag (`combined`) set to 0.
- Increment the array count.

**Code Sample:**

```c
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
```

### 2. Duplicate Term Checking

**already_exists Function**

Before adding a new combined term, this function checks whether a term with the same `bits` and `mask` already exists. This prevents redundant entries during the combination process.

**Code Sample:**

```c
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
```

### 3. Prime Implicant Computation

**compute_prime_implicants Function**

This is the core algorithm that computes the prime implicants from the given minterms.

- **Initialization:**  
  The algorithm first creates an initial working set (`current_terms`) from the input minterms. Each minterm is treated as a term with no masked bits.

- **Iterative Combination:**  
  It iteratively compares every pair of terms in the working set. If two terms have the same mask and differ in exactly one bit (checked by `(diff & (diff - 1)) == 0`), they are combined:
  - Both terms are marked as combined.
  - A new term is produced where its `bits` field is the bitwise AND of the two terms, and its `mask` is updated to include the differing bit.
  - The new term is added to a temporary array after checking for duplicates with the `already_exists` function.

- **Collection:**  
  Terms from the current set that were not combined in the iteration are collected as prime implicants.

- **Iteration:**  
  This process is repeated with the newly formed terms until no further combinations are possible.

**Code Outline:**

```c
static int compute_prime_implicants(const int minterms[], int num_minterms, term_t** prime_implicants_ptr)
{
    // Initialize current_terms from minterms.
    // For each pair in current_terms:
    //    If they differ in one bit, mark as combined and add the combined term to next_terms.
    // Collect uncombined terms as prime implicants.
    // Replace current_terms with next_terms and repeat until no new combinations are made.
}
```

### 4. Term Coverage and Expression Output

**covers_term Function**

Determines if a given term covers a specific minterm by comparing only the significant bits (ignoring masked positions).

**Code:**

```c
static int covers_term(const term_t* term, int minterm)
{
    return ((minterm & ~(term->mask)) == (term->bits & ~(term->mask)));
}
```

**print_term_expression Function**

This function converts a term into a human-readable Sum of Products (SOP) form by:
- Iterating over variables from the most significant to the least.
- For each unmasked bit, it prints the literal (e.g., `x3` if the bit is 1; `x3'` if 0).
- If no literal is printed (i.e. all bits are masked), it prints “1”, indicating a constant true.

**Code:**

```c
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
```

**print_term_with_parens Function**

A helper utility that wraps the output of a term in parentheses for improved clarity.

**Code:**

```c
static void print_term_with_parens(const term_t* term, int num_vars)
{
    printf("(");
    print_term_expression(term, num_vars);
    printf(")");
}
```

### 5. Overall Minimization Process

**perform_quine_mccluskey Function**

This function ties together the entire core algorithm:
- It computes all prime implicants via the `compute_prime_implicants` function.
- It selects essential prime implicants (those that uniquely cover a minterm).
- For any remaining uncovered minterms, it uses a greedy strategy to select the prime implicant covering the most.
- It then outputs the final minimized Boolean expression in SOP form.

**Code Outline:**

```c
void perform_quine_mccluskey(const int minterms[], int num_minterms, int num_vars)
{
    // 1. Compute prime implicants.
    // 2. Select essential prime implicants.
    // 3. Use a greedy approach for the remaining uncovered minterms.
    // 4. Output the final minimized expression in SOP format.
    // (Dynamic memory is freed accordingly.)
}
```