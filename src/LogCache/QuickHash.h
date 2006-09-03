#pragma once

///////////////////////////////////////////////////////////////
//
// quick_hash<>
//
//		A quick linear (array) hash index class. It requires HF
//		to provide the following interface
//
//		value_type	data type to hash
//		index_type	type of the index to store in the hash
//		NO_INDEX_VALUE an index_type to mark empty buckets
//
//		operator()	value_type -> size_t hash function
//		value()		index_type -> value_type
//		equal()		(value_type, index_type) -> bool 
//		
//		The capacity approximately doubles by each rehash().
//		Only insertion and lookup are povided. Collisions are
//		resolved using linear probing.
//
//		Use statistics() to monitor the cache performance.
//
///////////////////////////////////////////////////////////////
	 
template<class HF>
class quick_hash
{
public:

	typedef typename HF::value_type value_type;
	typedef typename HF::index_type index_type;
	
	struct statistics_t
	{
		size_t capacity;
		size_t used;
		size_t collisions;
		size_t max_path;
		size_t collision_path_sum;

		statistics_t()
			: capacity (1)
			, used (0)
			, collisions (0)
			, max_path (1)
			, collision_path_sum (0)
		{
		}
	};

private:

	class prime_grower
	{
	private:

		static const size_t primes[31];
		statistics_t statistics;
		size_t index;

	public:

		prime_grower()
			: index (0)
			, statistics()
		{
			statistics.capacity = primes[index];
		}

		size_t capacity() const 
		{
			return statistics.capacity;
		}

		size_t size() const 
		{
			return statistics.used;
		}
		
		size_t collisions() const 
		{
			return statistics.collisions;
		}
		
		void inserted_cleanly() 
		{
			++statistics.used;
		}

		void inserted_collision (size_t path_size) 
		{
			++statistics.used;
			++statistics.collisions;
			statistics.collision_path_sum += path_size;
			if (statistics.max_path <= path_size)
				statistics.max_path = path_size + 1;
		}

		void grow() 
		{
			statistics.capacity = primes[++index];
			statistics.collisions = 0;
			statistics.used = 0;
			statistics.collision_path_sum = 0;
			statistics.max_path = 1;
		}
		
		size_t map (size_t hash_value) const 
		{
			return hash_value % capacity();
		}
		
		size_t next (size_t index) const 
		{
			return (index + 1381000000) % capacity();
		}

		const statistics_t& get_statistics() const
		{
			return statistics;
		}
	};

	index_type* data;
	prime_grower grower;
	HF hf;
	
private:

	// are we allowed to add new entries to the hash
	// without re-hashing?

	bool should_grow() const
	{
		// grow, if there are many collisions 
		// or capacity is almost exceeded
		// There must also be at least one empty entry

		return grower.size() + grower.collisions() + 1 >= grower.capacity();
	}

	// initialize the new array before re-hashing
	
	void create_data()
	{
		size_t new_capacity = grower.capacity();
	
		data = new index_type[new_capacity];
		std::fill_n (data, new_capacity, HF::NO_INDEX_VALUE);
	}
	
	// add a value to the hash 
	// (must not be in it already; hash must not be full)
	
	void internal_insert (const value_type& value, index_type index)
	{
		// first try: uncollisioned insertion

		size_t bucket = grower.map (hf (value));
		index_type* target = data + bucket;

		if (*target == HF::NO_INDEX_VALUE)
		{
			*target = index;
			grower.inserted_cleanly();

			return;
		}

		// collision -> look for an empty bucket

		size_t collision_path_size = 0;
		do
		{
			bucket = grower.next (bucket);
			target = data + bucket;
			++collision_path_size;
		}
		while (*target != HF::NO_INDEX_VALUE);
		
		// insert collisioned item

		*target = index;
		grower.inserted_collision (collision_path_size);
	}
		
	void rehash (index_type* old_data, size_t old_data_size)
	{
		create_data();

		for (size_t i = 0; i < old_data_size; ++i)
		{
			index_type index = old_data[i];
			if (index != HF::NO_INDEX_VALUE)
				internal_insert (hf.value (index), index);
		}

		delete[] old_data;
	}
	
public:

	// construction / destruction

	quick_hash (const HF& hash_function) 
		: data(NULL)
		, hf (hash_function)
		, grower()
	{
		create_data();
	}
	
	quick_hash (const quick_hash& rhs) 
		: data (NULL)
		, hf (rhs.hf)
		, grower()
	{
		operator= (rhs);
	}
	
	~quick_hash() 
	{
		delete[] data;
	}

	// find the bucket containing the desired value;
	// return HF::NO_INDEX_VALUE if not contained in hash
	
	index_type find (const value_type& value) const
	{
		size_t bucket = grower.map (hf (value));
		index_type index = data[bucket];

		while (index != HF::NO_INDEX_VALUE)
		{
			// found?

			if (hf.equal (value, index))
				break;

			// collision -> look at next bucket position

			bucket = grower.next (bucket);
			index = data[bucket];
		}

		// either found or not in hash
		
		return index;
	}
	
	void insert (const value_type& value, index_type index)
	{
		assert (find (value) == HF::NO_INDEX_VALUE);
		
		if (should_grow())
			reserve (grower.capacity()+1);
	
		internal_insert (value, index);
	}

	void reserve (size_t min_bucket_count)
	{
		index_type* old_data = data;
		size_t old_data_size = grower.capacity();
		
		while (grower.capacity() < min_bucket_count)
			grower.grow();
			
		if (grower.capacity() != old_data_size)
			rehash (old_data, old_data_size);
	}

	// assignment

	quick_hash& operator=(const quick_hash& rhs)
	{
		if (grower.capacity() != rhs.grower.capacity())
		{
			delete[] data;
			data = new index_type [rhs.grower.capacity()];
		}

		hf = rhs.hf;
		grower = rhs.grower;

		std::copy (rhs.data, rhs.data + rhs.grower.capacity(), data);

		return *this;
	}

	// get rid of all entries

	void clear()
	{
		if (grower.size() > 0)
		{
			delete[] data;
			grower = prime_grower();
			create_data();
		}
	}

	// read cache performance statistics

	const statistics_t& statisitics() const
	{
		return grower.get_statistics();
	}
};

template<class HF>
const size_t quick_hash<HF>::prime_grower::primes[31] = 
	{1, 3, 7, 17, 
	 31, 67, 127, 257, 
	 509, 1021, 2053, 4099,
	 8191, 16381, 32771, 65537,
	 131071, 262147, 524287, 1048573,
	 2097143, 4194301, 8388617, 16777213,
	 33554467, 67108859, 134217757, 268435459,
	 536870909, 1073741827};

