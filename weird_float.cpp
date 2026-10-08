//
// Created by User on 2026/10/3.
//

#include <cstdio>
#include <cmath>

int main() {
	volatile const double a = 0.1, b = 10.0, c = -1.0;

	std::printf("[sin]: %.17g\n", sin(b));
	std::printf("[cos]: %.17g\n", cos(b));
	std::printf("[tan]: %.17g\n", tan(b));

	std::printf("[exp]: %.17g\n", exp(b));
	std::printf("[pow]: %.17g\n", pow(b, a));

	std::printf("[sqrt]: %.17g\n", sqrt(b));
	std::printf("[illegal]: %.17g\n", sqrt(c));
	std::printf("%.17g\n", a * b + c);
}