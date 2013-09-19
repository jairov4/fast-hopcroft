// July 2013, Jairo Andres Velasco Romero, jairov(at)javerianacali.edu.co
#pragma once

// Workaround to allow use intrinsics
#ifdef BOOST_DYNAMIC_BITSET_DYNAMIC_BITSET_HPP
#error "Include this file first to allow BOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS"
#endif
#define BOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS
#include <boost/dynamic_bitset.hpp>

#include <stdint.h>

template<typename Block>
size_t hash_seq(const std::vector<Block>& vc, size_t bits)
{
	size_t bits_per_block = sizeof(Block)*8;
	size_t n = bits / bits_per_block;
	size_t o = bits % bits_per_block;
	// el tamano es n porque no queremos tomar el ultimo item
	size_t h = _Hash_seq((const unsigned char*)vc.data(), n*sizeof(Block));
	Block mask = (Block(1) << o) - 1;
	h ^= vc[n] & mask;
	return h;
}

template<typename Block, typename Allocator = std::allocator<Block>>
class dynamic_bitset : public boost::dynamic_bitset<Block, Allocator>
{
	typedef boost::dynamic_bitset<uint64_t, Allocator> __base;
public:
	size_type find_first() const { return __base::find_first(); }
	size_type find_next(size_type n) const { return __base::find_next(n);}
	bool test(size_type n) const { return __base::test(n); }
	dynamic_bitset& set(size_type n, bool val = true) { __base::set(n, val); return *this; }
	dynamic_bitset& reset(size_type n) { __base::reset(n); return *this; }
	dynamic_bitset& set() { __base::set(); return *this; }
	dynamic_bitset& reset() { __base::reset(); return *this; }
	bool test_set(size_type n) { bool i = __base::test(n); set(n); return i; }
	bool test_reset(size_type n) { bool i = __base::test(n); reset(n); return i; }
	
	struct hash
	{
		size_t operator()(const dynamic_bitset<Block>& _Keyval) const
		{	
			return hash_seq(_Keyval.m_bits, _Keyval.m_num_bits);
		}
	};


	dynamic_bitset& operator-=(const dynamic_bitset& b)
	{
		__base::operator-=(b);
		return *this;
	}
	
	dynamic_bitset(const dynamic_bitset& c) 
		: __base(c)
	{
	}	

	explicit dynamic_bitset(size_type num_bits, unsigned long value = 0, const Allocator& alloc = Allocator()) 
		: __base(num_bits, value, alloc)
	{
	}
};

template<typename Block, typename Allocator>
dynamic_bitset<Block, Allocator>& operator-(const dynamic_bitset<Block, Allocator>& x, const dynamic_bitset<Block, Allocator>& y)
{
    dynamic_bitset<Block, Allocator> b(x);
	return b -= y;
}

#ifdef _MSC_VER  
// MSVC specific - intrinsics usage
// TODO: port to use GNU G++ intrinsics

template<typename Allocator>
class dynamic_bitset<uint64_t, Allocator> : public boost::dynamic_bitset<uint64_t, Allocator>
{
	typedef boost::dynamic_bitset<uint64_t, Allocator> __base;
public:
	dynamic_bitset(const dynamic_bitset& c) 
		: __base(c)
	{
	}

	explicit dynamic_bitset(size_type num_bits, unsigned long value = 0, const Allocator& alloc = Allocator()) 
		: __base(num_bits, value, alloc)
	{
	}

	
	struct hash
	{
		size_t operator()(const typename dynamic_bitset<uint64_t, Allocator>& _Keyval) const
		{	
			return hash_seq(_Keyval.m_bits, _Keyval.m_num_bits);
		}
	};

	dynamic_bitset& operator-=(const dynamic_bitset& b)
	{
		__base::operator-=(b);
		return *this;
	}

	dynamic_bitset& set() { __base::set(); return *this; }
	dynamic_bitset& reset() { __base::reset(); return *this; }
	
	bool test(size_type n) const
	{
		auto r = (const int64_t*)&m_bits[n/bits_per_block];
		auto idx = n%bits_per_block;
		return _bittest64(r, idx) != 0;
	}
		
	dynamic_bitset& set(size_type n, bool val = true)
	{
		auto r = (int64_t*)&m_bits[n/bits_per_block];
		auto idx = n%bits_per_block;
		if(val) _bittestandset64(r, idx);
		else _bittestandreset64(r, idx);
		return *this;
	}
	
	dynamic_bitset& reset(size_type n)
	{
		auto r = (int64_t*)&m_bits[n/bits_per_block];
		auto idx = n%bits_per_block;
		_bittestandreset64(r, idx);
		return *this;
	}

	bool test_set(size_type n) 
	{ 
		auto r = (int64_t*)&m_bits[n/bits_per_block];
		auto idx = n%bits_per_block;
		return _bittestandset64(r, idx) ? true : false;
	}

	bool test_reset(size_type n) 
	{ 
		auto r = (int64_t*)&m_bits[n/bits_per_block];
		auto idx = n%bits_per_block;
		return _bittestandreset64(r, idx) ? true : false;
	}

	size_type find_first() const
	{		
		int c = 0;
		for(auto i=m_bits.begin(); i!=m_bits.end(); i++, c+=bits_per_block)
		{
			unsigned long l;
			if(_BitScanForward64(&l, *i) != 0) return l+c;
		}
		return npos;
	}

	size_type find_next(size_type t) const
	{
		const size_t offset_mask = bits_per_block-1;

		size_t s = t / bits_per_block;
		auto i = m_bits.begin()+s;

		size_t c = t & ~offset_mask;  // 111111110000
		size_t o = t & offset_mask; // 000000001111
		if(o == offset_mask) goto __other_block;				
		block_type ii = *i; 		
		block_type old_mask = ((block_type)(-1) << (o+1));
		ii = ii & old_mask;// removes previous bits
		//ii = _blsr_u64(ii); // avx2 version
		unsigned long l;
		if(_BitScanForward64(&l, ii) != 0) 
		{ 
			assert(l+c > t);
			return l+c; 
		}
__other_block:
		i++; c+=bits_per_block;
		for(; i!=m_bits.end(); i++, c+=bits_per_block)
		{		
			ii = *i;		
			if(_BitScanForward64(&l, ii) != 0)
			{
				assert(l+c > t);
				return l+c;		
			}
		}
		return npos;
	}

};

#endif//_MSC_VER