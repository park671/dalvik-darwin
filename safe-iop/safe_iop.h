//
// Created by Park Yu on 2024/6/21.
//

#ifndef SAFE_IOP_H
#define SAFE_IOP_H

#ifndef SAFE_MATH_H
#define SAFE_MATH_H

#include <stdbool.h>

/* Prototypes for safe arithmetic functions */
bool safe_add(int *ptr, int a, int b);
bool safe_add3(int *ptr, int a, int b, int c);
bool safe_add4(int *ptr, int a, int b, int c, int d);
bool safe_add5(int *ptr, int a, int b, int c, int d, int e);

bool safe_sub(int *ptr, int a, int b);
bool safe_sub3(int *ptr, int a, int b, int c);
bool safe_sub4(int *ptr, int a, int b, int c, int d);
bool safe_sub5(int *ptr, int a, int b, int c, int d, int e);

bool safe_mul(int *ptr, int a, int b);
bool safe_mul3(int *ptr, int a, int b, int c);
bool safe_mul4(int *ptr, int a, int b, int c, int d);
bool safe_mul5(int *ptr, int a, int b, int c, int d, int e);

bool safe_div(int *ptr, int a, int b);
bool safe_div3(int *ptr, int a, int b, int c);
bool safe_div4(int *ptr, int a, int b, int c, int d);
bool safe_div5(int *ptr, int a, int b, int c, int d, int e);

bool safe_mod(int *ptr, int a, int b);
bool safe_mod3(int *ptr, int a, int b, int c);
bool safe_mod4(int *ptr, int a, int b, int c, int d);
bool safe_mod5(int *ptr, int a, int b, int c, int d, int e);

#endif /* SAFE_MATH_H */

#endif //SAFE_IOP_H
