#include <cmath>
#include <memory>

int main()
{
	std::allocator<int> alloc;
	constexpr std::size_t max = std::pow(10, 8);
	int *p = alloc.allocate(max);
	for(std::size_t i = max + 1; --i;)
		alloc.construct(p + i - 1, 1);
	for(std::size_t i = max + 1; --i;)
		alloc.destroy(p + i - 1);
	alloc.deallocate(p, max);
}
