//
// Created by User on 2026/10/5.
//

#include <cstdio>
#include <iomanip>
#include <iostream>

int main() {
	printf("%.9f\n", 0.2000000005);
	printf("%.20f\n", 0.2000000005);

	printf("%.4f\n", 0.10005);
	printf("%.20f\n", 0.10005);
	std::cout << std::fixed << std::setprecision(5) << 0.100005 << '\n';
	std::cout << std::fixed << std::setprecision(20) << 0.100005 << '\n';
}