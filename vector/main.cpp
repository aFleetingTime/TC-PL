#include "Vector.h"
#include <vector>
#include <list>
#include <iostream>
#include <cmath>

int main()
{
	std::list<int> lis;
	std::vector<int> svec(lis.begin(), lis.end());
	Vector vec(svec.begin(), svec.end());
	Vector<int> vecc(svec.cbegin(), svec.cend());
	Vector<int> veccc(vec.begin(), vec.end());
	Vector<int> ve(lis.begin(), lis.end());
	Vector<int> vesk(lis.cbegin(), lis.cend());
}
