#include <stdio.h>
#include <conio.h>
#include "adaptive_filter_rls.h"
#include "../vector_math/special_math.h"
#include "../vector_math/s_malloc.h"

struct rls_st rls_st_str;

int main(void) {
	int p = 25;
	float lambda = 1.85;
	float delta = 0.001;
	int sucesess = rls_init(p, lambda, delta, &rls_st_str);




	return 0;
}