#pragma once
#include <functional>


class ScopeGuard
{
public:
    explicit ScopeGuard(std::function<void()> const& callOnExit)
	: function_(callOnExit)
	, disabled_(false)
	{
	}

	~ScopeGuard()
	{
		if(!disabled_ && function_)
            function_();
	}

	void Disable()
	{
        disabled_ = true;
	}

private:
    ScopeGuard() = delete;
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

    std::function<void()> const function_;
    bool disabled_;
};

#define SCOPEGUARD_CAT(a, b) a##b
#define SCOPEGUARD_MAKENAME(prefix, suffix) SCOPEGUARD_CAT(prefix, suffix)

#define ON_SCOPE_EXIT(func) \
ScopeGuard SCOPEGUARD_MAKENAME(scopeExit, __LINE__)(func)

// Anonymous declaration. TODO: need test on more compilers.
#define SCOPE_EXIT(lambda) (ScopeGuard)(lambda)
