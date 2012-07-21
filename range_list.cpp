#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <map>
#include <string>
#include <sstream>
#include <stdexcept>

using namespace std;
using namespace boost::python;

typedef long addr_t;

struct range_item{
	const addr_t address;
	addr_t size;
	
	range_item():address(-1),size(-1)
	{}

	range_item(addr_t a, addr_t s):address(a),size(s)
	{}
	
	bool has(addr_t a)const
	{
		return address <= a && a < address+size;
	}
	
	bool operator==(object o)const
	{
		if(o.ptr()==NULL){
			return address==-1;
		}
		extract<range_item&> x(o);
		if(x.check()){
			range_item& y=x();
			return address==y.address && size==y.size;
		}
		return false;
	}
};

ostream& operator<<(ostream& o, range_item r)
{
	if(r.is_none())
		o << "None";
	else{
		ostringstream s;
		s << hex << r.address <<":"<< r.size ;
		o << s.str();
	}
	return o;
}

typedef map<addr_t, range_item>::iterator range_iter;

typedef pair<const addr_t, range_item> range_iter_deref;

ostream& operator<<(ostream& o, const range_iter_deref& r)
{
	o << r.second;
	return o;
}

struct range_list{
	map<addr_t, range_item> _data;
	typedef map<addr_t, range_item>::iterator iter_t;
	
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

void translator(const invalid_argument& x) {
    PyErr_SetString(PyExc_UserWarning, x.what());
}

BOOST_PYTHON_MODULE(range_list)
{
	// register_exception_translator<
          // invalid_argument>(translator);
	class_<range_item>("range_item", init<unsigned long, unsigned long>())
		.def(init<>())
		.def_readonly("address", &range_item::address)
		.def_readwrite("size", &range_item::size)
		.def("has", &range_item::has)
		.def(self_ns::str(self_ns::self))
		.def(self_ns::self == object)
		;
	class_<range_list>("range_list")
		.def("__iter__", range(&range_list::begin, &range_list::end))
		.def("__len__", &range_list::size)
		.def("clear", &range_list::clear)
		.def("insert", &range_list::insert)
		.def("remove", &range_list::remove)
		.def("search", &range_list::search)
		.def("finger", &range_list::finger)
		;
	class_<range_iter>("range_iter")
		.def(self_ns::self == self_ns::self)
		;
	class_<range_iter_deref>("range_iter_deref")
		.def_readonly("item", &pair<const addr_t,range_item>::second)
		.def_readonly("address",&pair<const addr_t,range_item>::first)
		.def(self_ns::str(self_ns::self))
		;
}

