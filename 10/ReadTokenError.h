#pragma once
#include <stdexcept>

struct ReadTokenError : public std::runtime_error
{
	using std::runtime_error::runtime_error;
};
