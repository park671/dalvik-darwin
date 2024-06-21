//
// Created by Park Yu on 2024/6/21.
//

#include "safe_iop.h"
#include <stdbool.h>
#include <stddef.h>

/* Define the safe addition function */
bool safe_add(int *ptr, int a, int b) {
    if (ptr != NULL) {
        *ptr = a + b;
    }
    return true;
}

/* Define the safe addition function for three integers */
bool safe_add3(int *ptr, int a, int b, int c) {
    int r = 0;
    safe_add(&r, a, b);
    safe_add(ptr, r, c);
    return true;
}

/* Define the safe addition function for four integers */
bool safe_add4(int *ptr, int a, int b, int c, int d) {
    int r = 0;
    safe_add(&r, a, b);
    safe_add(&r, r, c);
    safe_add(ptr, r, d);
    return true;
}

/* Define the safe addition function for five integers */
bool safe_add5(int *ptr, int a, int b, int c, int d, int e) {
    int r = 0;
    safe_add(&r, a, b);
    safe_add(&r, r, c);
    safe_add(&r, r, d);
    safe_add(ptr, r, e);
    return true;
}

/* Define the safe subtraction function */
bool safe_sub(int *ptr, int a, int b) {
    if (ptr != NULL) {
        *ptr = a - b;
    }
    return true;
}

/* Define the safe subtraction function for three integers */
bool safe_sub3(int *ptr, int a, int b, int c) {
    int r = 0;
    safe_sub(&r, a, b);
    safe_sub(ptr, r, c);
    return true;
}

/* Define the safe subtraction function for four integers */
bool safe_sub4(int *ptr, int a, int b, int c, int d) {
    int r = 0;
    safe_sub(&r, a, b);
    safe_sub(&r, r, c);
    safe_sub(ptr, r, d);
    return true;
}

/* Define the safe subtraction function for five integers */
bool safe_sub5(int *ptr, int a, int b, int c, int d, int e) {
    int r = 0;
    safe_sub(&r, a, b);
    safe_sub(&r, r, c);
    safe_sub(&r, r, d);
    safe_sub(ptr, r, e);
    return true;
}

/* Define the safe multiplication function */
bool safe_mul(int *ptr, int a, int b) {
    if (ptr != NULL) {
        *ptr = a * b;
    }
    return true;
}

/* Define the safe multiplication function for three integers */
bool safe_mul3(int *ptr, int a, int b, int c) {
    int r = 0;
    safe_mul(&r, a, b);
    safe_mul(ptr, r, c);
    return true;
}

/* Define the safe multiplication function for four integers */
bool safe_mul4(int *ptr, int a, int b, int c, int d) {
    int r = 0;
    safe_mul(&r, a, b);
    safe_mul(&r, r, c);
    safe_mul(ptr, r, d);
    return true;
}

/* Define the safe multiplication function for five integers */
bool safe_mul5(int *ptr, int a, int b, int c, int d, int e) {
    int r = 0;
    safe_mul(&r, a, b);
    safe_mul(&r, r, c);
    safe_mul(&r, r, d);
    safe_mul(ptr, r, e);
    return true;
}

/* Define the safe division function */
bool safe_div(int *ptr, int a, int b) {
    if (ptr != NULL && b != 0) {
        *ptr = a / b;
    }
    return true;
}

/* Define the safe division function for three integers */
bool safe_div3(int *ptr, int a, int b, int c) {
    int r = 0;
    safe_div(&r, a, b);
    safe_div(ptr, r, c);
    return true;
}

/* Define the safe division function for four integers */
bool safe_div4(int *ptr, int a, int b, int c, int d) {
    int r = 0;
    safe_div(&r, a, b);
    safe_div(&r, r, c);
    safe_div(ptr, r, d);
    return true;
}

/* Define the safe division function for five integers */
bool safe_div5(int *ptr, int a, int b, int c, int d, int e) {
    int r = 0;
    safe_div(&r, a, b);
    safe_div(&r, r, c);
    safe_div(&r, r, d);
    safe_div(ptr, r, e);
    return true;
}

/* Define the safe modulus function */
bool safe_mod(int *ptr, int a, int b) {
    if (ptr != NULL && b != 0) {
        *ptr = a % b;
    }
    return true;
}

/* Define the safe modulus function for three integers */
bool safe_mod3(int *ptr, int a, int b, int c) {
    int r = 0;
    safe_mod(&r, a, b);
    safe_mod(ptr, r, c);
    return true;
}

/* Define the safe modulus function for four integers */
bool safe_mod4(int *ptr, int a, int b, int c, int d) {
    int r = 0;
    safe_mod(&r, a, b);
    safe_mod(&r, r, c);
    safe_mod(ptr, r, d);
    return true;
}

/* Define the safe modulus function for five integers */
bool safe_mod5(int *ptr, int a, int b, int c, int d, int e) {
    int r = 0;
    safe_mod(&r, a, b);
    safe_mod(&r, r, c);
    safe_mod(&r, r, d);
    safe_mod(ptr, r, e);
    return true;
}
