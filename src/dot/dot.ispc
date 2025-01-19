export uniform float dotf(const uniform float x[], const uniform float y[], const uniform size_t n) {
	varying float acc = 0.0;

	foreach (i = 0 ... n) {
		acc += x[i] * y[i];
	}

	return reduce_add(acc);
}


export uniform double dotd(const uniform double x[], const uniform double y[], const uniform size_t n) {
	varying double acc = 0.0;

	foreach (i = 0 ... n) {
		acc += x[i] * y[i];
	}

	return reduce_add(acc);
}
