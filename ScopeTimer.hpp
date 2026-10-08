//
// Created by User on 2026/10/3.
//

#pragma once
#include <deque>
#include <memory>
#include <string>
#include <utility>

namespace ScopeTimerPolicy {
class GlobalStack {
	std::deque<std::string> stk;

	GlobalStack() = default;

	~GlobalStack() = default;

	GlobalStack(const GlobalStack &) = delete;

	GlobalStack &operator=(const GlobalStack &) = delete;

	GlobalStack(GlobalStack &&) = delete;

	GlobalStack &operator=(GlobalStack &&) = delete;

public:
	static GlobalStack &instance();

	void push(std::string &&s) {
		stk.emplace_back(std::move(s));
	}

	void pop() {
		stk.pop_back();
	}

	const auto &get_stack() const {
		return stk;
	}

	std::string to_string() const;
};
} // namespace ScopeTimerPolicy
class ScopeTimer {
	struct Internal;
	std::unique_ptr<Internal> internal;

public:
	explicit ScopeTimer(std::string &&name);

	~ScopeTimer();

	ScopeTimer(const ScopeTimer &) = delete;

	ScopeTimer &operator=(const ScopeTimer &) = delete;

	ScopeTimer(ScopeTimer &&) = delete;

	ScopeTimer &operator=(ScopeTimer &&) = delete;
};