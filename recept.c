#include "recept.h"
#include "bar.h"
#include "tau.h"

double complex delta_dc(double complex cval, double complex prior_cval) {
	if (prior_cval != 0.0) {
		return cval / prior_cval;
	} else {
		return 0.0;
	}
}


int main() {
	return 0;
}
