#pragma once
#include <stdexcept>

struct DivideByZeroError : public std::logic_error
{
	using std::logic_error::logic_error;
};
