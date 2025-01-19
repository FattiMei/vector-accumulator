export uniform float minimumf(const uniform float x[], const uniform size_t n) {
	varying float minimum = x[0];

	foreach (i = 0 ... n) {
		minimum = min(minimum, x[i]);
	}

	return reduce_min(minimum);
}


export uniform double minimumd(const uniform double x[], const uniform size_t n) {
	varying double minimum = x[0];

	foreach (i = 0 ... n) {
		minimum = min(minimum, x[i]);
	}

	return reduce_min(minimum);
}
