# Quine–McCluskey 최소화 알고리즘 구현 상세 기술 문서

이 문서는 minterm들을 이용해 불리언 함수를 최소화하는 Quine–McCluskey 알고리즘의 핵심 로직만을 상세히 설명합니다. 입출력 및 예외 처리 관련 내용은 제외하였으며, 실제 알고리즘 구현의 동작에 집중합니다.  

**[Read the English version of README](README.md)**

## 프로젝트 구조

### 1. 동적 배열 관리

**add_term 함수**

해당 함수는 동적으로 할당된 배열에 새 항(term)을 추가합니다. 핵심 로직은 다음과 같습니다:
- 배열이 꽉 찼는지 확인한다.
- 배열의 용량을 2배로 확장하면서 재할당(realloc)한다.
- 주어진 비트 패턴(`bits`)과 마스크(`mask`), 그리고 결합 여부를 나타내는 플래그(`combined`, 0으로 초기화)를 설정한다.
- 배열에 항이 추가될 때마다 요소 개수를 증가시킨다.

**코드 샘플:**

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

### 2. 중복 항 검사

**already_exists 함수**

새로운 항을 추가하기 전에, 동일한 `bits`와 `mask`를 가진 항이 이미 존재하는지 확인합니다. 이를 통해 결합 과정에서 중복 항이 생성되는 것을 방지합니다.

**코드 샘플:**

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

### 3. Prime Implicant 계산

**compute_prime_implicants 함수**

이 함수는 입력된 minterm들로부터 prime implicant들을 도출하는 핵심 로직을 담고 있습니다.

- **초기화:**  
  각 minterm을 마스크 없이 초기 항(`current_terms`)으로 구성합니다.

- **반복 결합:**  
  중첩 루프를 사용하여 모든 항 쌍을 비교합니다.  
  두 항이 같은 mask를 가지며, 정확히 한 비트만 다른 경우(즉, `diff & (diff - 1) == 0` 검사):
  - 두 항 모두를 결합된 것으로 표시합니다.
  - 두 항의 `bits`를 AND하여 결합된 항을 생성하고, 차이나는 비트를 mask에 추가하여 업데이트합니다.
  - 중복되지 않도록 확인한 후 임시 배열(`next_terms`)에 추가합니다.

- **수집:**  
  결합되지 않은 항은 prime implicant로 수집됩니다.

- **반복 수행:**  
  새로운 항 집합(`next_terms`)이 생성될 때마다 과정을 반복하여 더 이상 결합할 수 없을 때까지 진행합니다.

**코드 개요:**

```c
static int compute_prime_implicants(const int minterms[], int num_minterms, term_t** prime_implicants_ptr)
{
    // 1단계: minterm들을 이용하여 초기 current_terms 배열 구성.
    // 2단계: 중첩 루프를 통해 각 항 쌍을 비교하여 한 비트만 다른 항 결합.
    // 3단계: 결합되지 않은 항은 prime implicant 배열에 추가.
    // 4단계: 새로운 항 집합을 생성하면서 결합 과정을 반복.
}
```

### 4. 항 커버 판단 및 출력

**covers_term 함수**

해당 함수는 항이 주어진 minterm을 커버하는지 판단합니다. 마스크된 비트를 제외하고 실제 중요한 비트들만 비교하여 일치하는지를 확인합니다.

**코드:**

```c
static int covers_term(const term_t* term, int minterm)
{
    return ((minterm & ~(term->mask)) == (term->bits & ~(term->mask)));
}
```

**print_term_expression 함수**

이 함수는 항을 사람이 읽기 쉬운 Sum of Products (SOP) 형태로 출력합니다.
- 가장 상위 변수부터 순회하며, 마스크되지 않은 변수에 대해 처리합니다.
- 해당 비트가 1이면 그대로, 0이면 보수를 나타내도록 출력합니다.
- 만약 출력할 변수가 하나도 없으면 항이 상수 1임을 의미하여 “1”을 출력합니다.

**코드:**

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

**print_term_with_parens 함수**

이 함수는 항의 출력을 괄호로 감싸서 가독성을 높입니다.

**코드:**

```c
static void print_term_with_parens(const term_t* term, int num_vars)
{
    printf("(");
    print_term_expression(term, num_vars);
    printf(")");
}
```

### 5. 전체 최소화 로직

**perform_quine_mccluskey 함수**

전체 알고리즘의 흐름은 다음과 같습니다:
- **Prime Implicant 도출:**  
  입력된 minterm들을 기반으로 `compute_prime_implicants`를 호출하여 모든 prime implicant들을 생성합니다.
- **Essential (유일하게 커버되는) 항 선택:**  
  각 minterm을 유일하게 커버하는 항을 선택합니다.
- **그리디 방식:**  
  남은 미커버 minterm들을 보완하기 위해, 아직 커버되지 않은 minterm들을 가장 많이 포함하는 항을 선택합니다.
- **최종 출력:**  
  최종 최소화된 불리언 함수를 SOP 형태로 출력합니다.

**코드 개요:**

```c
void perform_quine_mccluskey(const int minterms[], int num_minterms, int num_vars)
{
    // 1단계: prime implicant 계산.
    // 2단계: Unique하게 커버되는 essential 항 선택.
    // 3단계: 남은 minterm들을 보완하기 위한 그리디 선택.
    // 4단계: 최종 최소화된 SOP 형태로 결과 출력.
    // (필요한 동적 메모리는 적절히 해제됨)
}
```