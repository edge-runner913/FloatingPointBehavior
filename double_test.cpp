//
// Created by User on 2026/10/3.
//

#include <cstdint>
#include <iostream>
#include <cmath>
#include <stdfloat>
#include <limits>
#include <vector>
#include <climits>
#include "ScopeTimer.hpp"
#include <assert.h>

#include <functional>

void test_func(const std::function<void()> &callback, const char *prompt = "test") {
	ScopeTimer timer(prompt);
	return callback();
}

template <typename T>
void run_denormal_core() {
	const size_t N = 10'000'000;

	// 1. 準備常規數與次常規數資料
	std::vector<T> normal_data(N, T(1.0001));
	T subnormal_val = std::numeric_limits<T>::min() / 2;
	std::vector<T> subnormal_data(N, subnormal_val);

	// 2. 常規數運算
	{
		ScopeTimer timer("Normal Loop");
		T sum_normal = 0;
		for (size_t i = 0; i < N; ++i) {
			sum_normal += normal_data[i] * T(0.99);
		}

		// 阻斷編譯器優化，強制寫回記憶體
#if defined(__GNUC__) || defined(__clang__)
		asm volatile("" : : "g"(sum_normal) : "memory");
#else
		volatile T dummy1 = sum_normal;
#endif
	}
	{
		ScopeTimer timer("Abnormal Loop");
		// 3. 次常規數運算（觸發微碼異常）
		T sum_subnormal = 0;
		for (size_t i = 0; i < N; ++i) {
			sum_subnormal += subnormal_data[i] * T(0.99);
		}

#if defined(__GNUC__) || defined(__clang__)
		asm volatile("" : : "g"(sum_subnormal) : "memory");
#else
		volatile T dummy2 = sum_subnormal;
#endif
	}
}

template <typename T>
void run_memory_bound_core(size_t elements) {
	ScopeTimer timer("run_memory_bound_core");

	std::vector<T> src(elements, T(1.1));
	std::vector<T> dst(elements, T(0.0));

	// 純粹的記憶體搬移與流水線寫入
	for (size_t i = 0; i < elements; ++i) {
		dst[i] = src[i] * T(2.0);
	}

	// 阻斷編譯器優化，確保 dst 陣列的修改不會被跳過
#if defined(__GNUC__) || defined(__clang__)
	asm volatile("" : : "g"(dst.data()) : "memory");
#else
	volatile T dummy = dst[0];
#endif
}

#define repeat 100'000'000

template <typename T>
auto sine(T num) {
	ScopeTimer timer("sine");
	auto res = static_cast<T>(num);
	for (int i = 0; i < repeat; i++)
		res = std::sin(res);
	return res;
}

template <typename T>
auto square(T num) {
	ScopeTimer timer("square");
	auto res = static_cast<T>(num);
	for (int i = 0; i < repeat; i++)
		res = std::sqrt(res);
	return res;
}

template <typename T>
auto power(T num1, T num2) {
	ScopeTimer timer("power");
	auto res = static_cast<T>(num1);
	for (int i = 0; i < repeat; i++)
		res += std::pow(num1, num2);
	return res;
}

template <typename T>
auto sqrt(T num1) {
	ScopeTimer timer("sqrt");
	auto res = static_cast<T>(num1);
	for (int i = 0; i < repeat; i++)
		res = std::sqrt(res);
	return res;
}

template <typename T>
auto expon(T num) {
	ScopeTimer timer("expon");
	auto res = static_cast<T>(num);
	for (int i = 0; i < 10; i++)
		res = std::exp(res);
	return res;
}


int main() {
	std::cout << sizeof(long double) * CHAR_BIT << std::endl;
	static_assert(sizeof(_Float64) * CHAR_BIT == 64, "Error: _Float64 is not 64-bit on this architecture");

	const volatile _Float64 num1 = 1, num2 = 2;

	volatile _Float64 store = num1;

	const auto add = [&]() {
		store = 0;
		for (int i = 0; i < repeat; i++)
			store = num1 + num2;
		std::printf("%.17g\n", static_cast<double>(store));
	};
	const auto sub = [&]() {
		store = 0;
		for (int i = 0; i < repeat; i++)
			store = num1 - num2;
		std::printf("%.17g\n", static_cast<double>(store));
	};
	const auto mul = [&]() {
		store = 0;
		for (int i = 0; i < repeat; i++)
			store = num1 * num2;
		std::printf("%.17g\n", static_cast<double>(store));
	};
	const auto div = [&]() {
		store = 0;
		for (int i = 0; i < repeat; i++)
			store = num1 / num2;
		std::printf("%.17g\n", static_cast<double>(store));
	};

	const auto sine = [&]() {
		for (int i = 0; i < repeat; i++)
			store = std::sin(num1);
		std::printf("%.17g\n", static_cast<double>(store));
	};

	const auto square = [&]() {
		for (int i = 0; i < repeat; i++)
			store = std::sqrt(num1);
		std::printf("%.17g\n", static_cast<double>(store));
	};
	const auto power = [&]() {
		for (int i = 0; i < repeat; i++)
			store = std::pow(num1, num2);
		std::printf("%.17g\n", static_cast<double>(store));
	};

	const auto sqrt = [&]() {
		for (int i = 0; i < repeat; i++)
			store = std::sqrt(num1);
		std::printf("%.17g\n", static_cast<double>(store));
	};

	const auto expon = [&]() {
		for (int i = 0; i < repeat; i++)
			store = std::exp(num1);
		std::printf("%.17g\n", static_cast<double>(store));
	};

	test_func(add, "add");
	test_func(sub, "sub");
	test_func(mul, "mul");
	test_func(div, "div");
	test_func(sine, "sine");
	test_func(square, "square");
	test_func(power, "power");
	test_func(sqrt, "sqrt");
	test_func(expon, "expon");

	// run_denormal_core<_Float64>();
	run_denormal_core<_Float64>();
	//
	// // L1 Cache 級別（資料量小，完全在極速快取內）
	// run_memory_bound_core<_Float64>(4000); // ~16 KB
	run_memory_bound_core<_Float64>(2000); // ~16 KB
	//
	// // RAM 級別（資料量大，榨乾記憶體頻寬，觸發大量 Cache Miss）
	// run_memory_bound_core<_Float64>(64'000'000); // ~256 MB
	run_memory_bound_core<_Float64>(32'000'000); // ~256 MB
}