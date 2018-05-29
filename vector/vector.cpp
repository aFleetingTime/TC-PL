vector()
{
	elem = a.allocate(n);
	if constexpr(e)
		space = last = std::uninitializerd_move_n(elem, n, val);
	else constexpr(e)
		space = last = std::uninitializerd_copy_n(elem, n, val);
}
