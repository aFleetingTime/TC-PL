#include <memory>
#include <numeric>
#include <algorithm>

template<typename T, typename Alloc>
struct Bucket
{
	using list_type = std::list<T, Alloc>;
	using hash_type = std::list<T, Alloc>;

	Bucket() = default;
	Bucket(std::size_t h) : hash(h) { }
	Bucket(std::size_t h, const Alloc &a) : hash(h), list(a) { }
	std::size_t hash;
	std::list<T, Alloc> list;
};

template<typename T, typename Hash, typename Equal, typename Alloc>
class HashTable
{
public:
	HashTable() : table(default_bucket_size) { }

	// 集合操作

private:
	using bucket = Bucket<T, Alloc>;
	struct iterator
	{
		iterator(std::vector<bucket> &t) : bucketBeg(t.begin()), bucketCur(bucketBeg), bucketEnd(t.end()) {
			gotoNextBucket();
		}

		iterator& operator++() {
			if (listIt == --bucketCur->list.end())
				gotoNextBucket();
			else ++listBeg;
			return *this;
		}

		iterator operator++(int) {
			iterator tmp = *this;
			++*this;
			return tmp;
		}

		iterator& operator--() {
			if (listIt == bucketBeg->list.begin())
				gotoPrevBucket();
			else --listBeg;
			return *this;
		}

		iterator operator--(int) {
			iterator tmp = *this;
			--*this;
			return tmp;
		}
	private:
		std::vector<bucket>::iterator bucketBeg, bucketCur, bucketEnd;
		bucket::list_type::iterator listIt;
		void gotoNextBucket() {
			while (bucketCur != bucketEnd && bucketCur->list.empty())
				++bucketCur;
			if (bucketCur != bucketEnd)
				listIt = bucketBeg->list.begin();
		}
		void gotoNextBucket() {
			while (bucketCur != std::prev(bucketBeg) && bucketCur->list.empty());
			if (bucketCur != std::prev(bucketBeg))
				listIt = bucketBeg->list.begin();
		}
	}
	constexpr std::size_t default_bucket_size = 2;
	std::vector<bucket> table;
	Alloc alloc;
};
