//
// Created by User on 2026/10/5.
//


#include <cstdio>
#include <cmath>
#include <iostream>
#include <stdfloat>

int main() {
	volatile double e16 = 1e16, neg = -1.0, neg0 = -0.0;
	std::printf("[associative]: %.17g\n", (1 + e16) - e16);
	std::cout << "[finite]" << std::isnan(sqrt(neg)) << '\n';
	std::printf("[no-signed]: %.17g\n", 1 / (neg0 + 0.0));

	std::printf("[reciprocal]: %.17g\n", 5.0 / 3.0);
}