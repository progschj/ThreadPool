#pragma once
#include <functional>


class ScopeGuard
{
public:
	explicit ScopeGuard(std::function<void()> const& call_on_exit);
	~ScopeGuard();

	void Disable();

private:
    // no default constructor
    ScopeGuard() = delete;

	// noncopyable
    ScopeGuard(const ScopeGuard&) = delete;
    void operator=(const ScopeGuard&) = delete;

    std::function<void()> const function_;
    bool disabled_;
};

ScopeGuard::ScopeGuard(std::function<void()> const& call_on_exit)
	: function_(call_on_exit)
	, disabled_(false)
{

}

ScopeGuard::~ScopeGuard()
{
	if (!disabled_ && function_)
		function_();
}

void ScopeGuard::Disable()
{
	disabled_ = true;
}


#define SCOPEGUARD_CAT4(s, a, b, c) a##s##b##s##c
#define SCOPEGUARD_MAKENAME(prefix, infix, suffix) SCOPEGUARD_CAT4(_, prefix, infix, suffix)

#define ON_SCOPE_EXIT(func_or_lambda) \
ScopeGuard SCOPEGUARD_MAKENAME(__scope_exit, __LINE__, __COUNTER__)(func_or_lambda)
