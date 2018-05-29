#pragma once
#include <stdexcept>

struct BadEntry : public std::runtime_error
{
	using std::runtime_error::runtime_error;
};
