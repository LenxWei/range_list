#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <boost/pool/singleton_pool.hpp>
#include <map>
#include <string>
#include <sstream>
#include <stdexcept>
#include <iostream>

using namespace std;
using namespace boost::python;

typedef unsigned long addr_t;

struct range_item{
	const addr_t address;
	addr_t size;
	
	range_item():address(0),size(0)
	{}

	range_item(addr_t a, addr_t s):address(a),size(s)
	{}
	
	bool has(addr_t a)const
	{
		return address <= a && a < address+size;
	}

	bool operator==(const range_item& o)const
	{
		return address==o.address && size==o.size;
	}

};

ostream& operator<<(ostream& o, range_item r)
{
	ostringstream s;
	s << hex << r.address <<":"<< r.size ;
	o << s.str();
	return o;
}

#if 1
typedef map<addr_t, range_item, std::less<addr_t>,
	    boost::fast_pool_allocator<pair<const addr_t, range_item>
				       //,boost::default_user_allocator_new_delete,
				       //,boost::details::pool::null_mutex
				      > 
	> range_map_t;
#else
typedef map<addr_t, range_item
		> range_map_t;
#endif

typedef range_map_t::iterator range_iter;

typedef pair<const addr_t, range_item> range_iter_deref;

bool operator==(const range_item& s, const object& o)
{
	if(o.is_none())
		return s.address==0 && s.size==0;
	{
		extract<range_item&> x(o);
		if(x.check()){
			range_item& y=x();
			return s.address==y.address && s.size==y.size;
		}
	}
	{
		extract<range_iter_deref&> x(o);
		if(x.check()){
			range_iter_deref& y=x();
			return s.address==y.second.address && s.size==y.second.size;
		}
	}

	return false;
}
	
bool operator!=(const range_item& s, const object& o)
{
	return !(s==o);
}

bool operator==(const range_iter_deref& x, const range_item&  y)
{
	return x.second.address==y.address && x.second.size==y.size;
}

bool operator==(const range_iter_deref& x, const range_iter_deref&  y)
{
	return x.second.address==y.second.address && x.second.size==y.second.size;
}

ostream& operator<<(ostream& o, const range_iter_deref& r)
{
	o << r.second;
	return o;
}

struct range_list{
	range_map_t _data;
	typedef range_iter iter_t;
	
	size_t size()const
	{
		return _data.size();
	}

	void clear()
	{
		return _data.clear();
	}
	
    iter_t _simple_search(addr_t address)
	{
		iter_t it=_data.upper_bound(address);
		if(it!=_data.begin())
			return --it;
		return _data.end();
	}
	
    bool insert(const range_item& obj)
	{
		iter_t it_l=_simple_search(obj.address), end=_data.end();
		iter_t it_h=_simple_search(obj.address+obj.size-1);
		if(it_l!=it_h)
			return false;
		if(it_l!=end){
			if(it_l->second.has(obj.address))
				return false; // intersection
		}
		_data.insert(make_pair(obj.address, obj));
		return true;
	}
	
    void remove(const range_item& obj)
	{
		iter_t it=_simple_search(obj.address);
		if(it->second==obj){
			_data.erase(it);
			return;
		}
		throw std::invalid_argument("not in list");
	}
	
    range_iter index(addr_t address)
	{
		iter_t it=_simple_search(address), end=_data.end();
		if(it==end || !it->second.has(address))
			throw std::invalid_argument("not in list");
		return it;
	}

    range_item search(addr_t address)
	{
        iter_t it = _simple_search(address), end=_data.end();
        if(it==end || !it->second.has(address))
            return range_item();
	return it->second;
	}

	range_item finger(addr_t address)
	{
		return search(address);
	}
	 
	iter_t begin()
	{
		return _data.begin();
	}
	
	iter_t end()
	{
		return _data.end();
	}
};

range_iter_deref deref(const range_iter& it)
{
	return *it;
}


void translator(const invalid_argument& x) {
    PyErr_SetString(PyExc_UserWarning, x.what());
}

BOOST_PYTHON_MODULE(range_list)
{
	// register_exception_translator<
          // invalid_argument>(translator);
	class_<range_item>("range_item", init<addr_t, addr_t>())
		.def_readonly("address", &range_item::address)
		.def_readwrite("size", &range_item::size)
		.def("has", &range_item::has)
		.def(self_ns::str(self_ns::self))
		//.def(self_ns::self == self_ns::self)
		//.def(self_ns::self == range_iter_deref())
		.def(self_ns::operator==(self_ns::self, object()))
		;
	class_<range_list>("range_list")
		.def("__iter__", range(&range_list::begin, &range_list::end))
		.def("__len__", &range_list::size)
		.def("clear", &range_list::clear)
		.def("insert", &range_list::insert)
		.def("remove", &range_list::remove)
		.def("search", &range_list::search)
		.def("index", &range_list::index)
		.def("finger", &range_list::finger)
		;
	class_<range_iter>("range_iter")
		.def(self_ns::self == self_ns::self)
		.def(self_ns::self != self_ns::self)
		;
	class_<range_iter_deref>("range_iter_deref")
		.def_readonly("item", &pair<const addr_t,range_item>::second)
		.def_readonly("address",&pair<const addr_t,range_item>::first)
		.def(self_ns::str(self_ns::self))
		.def(self_ns::self == self_ns::self)
//		.def(self_ns::self == range_item())
		;
}

