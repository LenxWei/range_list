#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/python/object/iterator_core.hpp>
#include <boost/pool/pool_alloc.hpp>
//#include <boost/pool/singleton_pool.hpp>
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
    object obj;

	range_item():address(0),size(0)
	{}

	range_item(addr_t a, addr_t s):address(a),size(s)
	{}

	range_item(addr_t a, addr_t s, object x):address(a),size(s),obj(x)
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

range_item r_None;

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
				//,boost::default_user_allocator_new_delete
				//,boost::details::pool::null_mutex
			>
		> range_map_t;
#else
typedef map<addr_t, range_item
> range_map_t;
#endif

typedef range_map_t::iterator range_iter;

range_iter operator+(range_iter x, unsigned long c)
{
    for (unsigned long i=0; i < c; ++i)
        ++x;
    return x;
}

range_iter operator-(range_iter x, unsigned long c)
{
    for (unsigned long i=0; i < c; ++i)
        --x;
    return x;
}

bool operator==(const range_item& s, const object& o)
{
	if(o.is_none())
		return s.address==0;
	{
		extract<range_item&> x(o);
		if(x.check()){
			range_item& y=x();
			return s.address==y.address && s.size==y.size;
		}
	}

	return false;
}

bool operator!=(const range_item& s, const object& o)
{
	return !(s==o);
}

struct range_slice{
    range_iter _begin;
    range_iter _end;

    range_slice(range_iter begin1, range_iter end1):_begin(begin1),_end(end1)
    {}

    range_slice& own()
    {
        return *this;
    }

    range_item& next()
    {
        if(_begin==_end)
            boost::python::objects::stop_iteration_error();
        else{
            range_item& k=_begin->second;
            ++_begin;
            return k;
        }
    }
};

struct range_list{
	range_map_t _data;

	size_t size()const
	{
		return _data.size();
	}

	void clear()
	{
		return _data.clear();
	}

	range_iter _simple_search(addr_t address)
	{
		range_iter it=_data.upper_bound(address);
		if(it!=_data.begin())
			return --it;
		return _data.end();
	}

	bool insert(const range_item& obj)
	{
		range_iter it_l=_simple_search(obj.address), end=_data.end();
		range_iter it_h=_simple_search(obj.address+obj.size-1);
		if(it_l!=it_h)
			return false;
		if(it_l!=end){
			if(it_l->second.has(obj.address))
				return false; // intersection
		}
		_data.insert(make_pair(obj.address, obj));
		return true;
	}

    void del(range_iter it)
    {
        if(it!=end()){
            _data.erase(it);
            return;
        }
		throw std::invalid_argument("not in list");
    }

	void remove(const range_item& obj)
	{
		range_iter it=_simple_search(obj.address);
		if(it!=end() && it->second==obj){
			_data.erase(it);
			return;
		}
		throw std::invalid_argument("not in list");
	}

	void remove_address(addr_t address)
	{
		range_iter it=_simple_search(address);
		if(it!=end() && it->second.has(address)){
			_data.erase(it);
			return;
		}
		throw std::invalid_argument("not in list");
	}


	range_iter index(addr_t address)
	{
		range_iter it=_simple_search(address), end=_data.end();
		if(it==end || !it->second.has(address))
			throw std::invalid_argument("not in list");
		return it;
	}

	range_item& search(addr_t address)
	{
		range_iter it = _simple_search(address), end=_data.end();
		if(it==end || !it->second.has(address))
			return r_None;
		return it->second;
	}

	object detailed_search(addr_t address)
	{
		range_iter it=_simple_search(address), end=_data.end();
		if(it==end)
			return make_tuple(object(), begin());
		if(it->second.has(address))
			return make_tuple(it,it);
		range_iter next=it;
                ++next;
		if(next==end)
			return make_tuple(it,object());
		return make_tuple(it, next);
	}

	range_item& finger(addr_t address)
	{
		return search(address);
	}

	range_iter begin()
	{
		return _data.begin();
	}

	range_iter end()
	{
		return _data.end();
	}

	range_slice slice(const range_iter& it)
	{
		return range_slice(it, end());
	}

	range_slice slice2(const range_iter& it, const range_iter& it2)
	{
		return range_slice(it, it2);
	}

    range_slice all()
    {
        return range_slice(begin(),end());
    }

	range_item& at(const range_iter& it)
	{
		if(it==_data.end())
			throw invalid_argument("null iter");
		return it->second;
	}

    void merge(range_list& o)
    {
        for(range_iter it=o.begin(),end=o.end();it!=end;++it){
            insert(it->second);
        }
    }
};

void translator(const invalid_argument& x) {
	PyErr_SetString(PyExc_UserWarning, x.what());
}

BOOST_PYTHON_MODULE(range_list)
{
	// register_exception_translator<
	// invalid_argument>(translator);
	class_<range_item>("range_item", init<addr_t, addr_t>())
        .def(init<addr_t, addr_t, object>())
	.def_readonly("address", &range_item::address)
	.def_readwrite("size", &range_item::size)
	.def_readwrite("obj", &range_item::obj)
	.def("has", &range_item::has)
	.def(self_ns::str(self_ns::self))
	.def(self_ns::operator==(self_ns::self, object()))
	;
	class_<range_list>("range_list")
	.def("__iter__", &range_list::all)
	.def("__len__", &range_list::size)
	.def("clear", &range_list::clear)
	.def("insert", &range_list::insert)
	.def("remove", &range_list::remove)
	.def("remove_address", &range_list::remove_address)
	.def("delete", &range_list::del)
	.def("merge", &range_list::merge)
	.def("search", &range_list::search,return_internal_reference<>())
	.def("index",&range_list::index)
	.def("begin",&range_list::begin)
	.def("end",&range_list::end)
	.def("at", &range_list::at,return_internal_reference<>())
	.def("slice",&range_list::slice)
	.def("slice",&range_list::slice2)
	.def("finger", &range_list::finger,return_internal_reference<>())
	.def("detailed_search",&range_list::detailed_search)
	;
	class_<range_iter>("range_iter")
	.def(self_ns::self == self_ns::self)
	.def(self_ns::self != self_ns::self)
	.def(self_ns::self + int())
	.def(self_ns::self - int())
	;
	class_<range_slice>("range_slice", init<range_iter, range_iter>())
	.def("__iter__", &range_slice::own, return_internal_reference<>())
	.def("next", &range_slice::next, return_internal_reference<>())
	;
}

