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

typedef map<addr_t, object, std::less<addr_t>,
            boost::fast_pool_allocator<pair<const addr_t, object>
                >
        > range_map_t;

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

struct range_slice
{
    range_iter _begin;
    range_iter _end;

    range_slice(range_iter begin1, range_iter end1):_begin(begin1),_end(end1)
    {}

    range_slice& own()
    {
        return *this;
    }

    object next()
    {
        if(_begin==_end)
            boost::python::objects::stop_iteration_error();
        else
        {
            object& k=_begin->second;
            ++_begin;
            return k;
        }
    }
};

struct range_list
{
    range_map_t _data;

    bool __has(const range_iter& it, addr_t a) // it!=end(), a >= it->first
    {
        size_t s=extract<size_t>(it->second.attr("size"));
        return it->first+s > a;
    }

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

    bool insert(const object& obj)
    {
        addr_t address=extract<addr_t>(obj.attr("address"));
        size_t size=extract<size_t>(obj.attr("size"));
        range_iter it_l=_simple_search(address), end=_data.end();
        if(it_l==end){
            range_iter begin=_data.begin();
            if( begin==end || address+size <= begin->first )
                goto ok;
            return false; //intersect
        }
        if(__has(it_l, address)) // intersect
            return false;

        {
            range_iter it_h=it_l;
            ++it_h;
            if(it_h==end)
                goto ok;
            if(address+size > it_h->first)
                return false;
        }
ok:
        _data.insert(make_pair(address, obj));
        return true;
    }

    void del(range_iter it)
    {
        if(it!=end())
        {
            _data.erase(it);
            return;
        }
        throw std::invalid_argument("not in list");
    }

    void remove(const object& obj)
    {
        addr_t a=extract<addr_t>(obj.attr("address"));
        range_iter it=_simple_search(a);
        size_t s=extract<size_t>(obj.attr("size"));
        if(it!=end() && it->first==a && extract<size_t>(it->second.attr("size"))==s)
        {
            _data.erase(it);
            return;
        }
        throw std::invalid_argument("not in list");
    }

    void remove_address(addr_t address)
    {
        range_iter it=_simple_search(address);
        if(it!=end())
        {
            if(__has(it,address))
            {
                _data.erase(it);
                return;
            }
        }
        throw std::invalid_argument("not in list");
    }


    range_iter index(addr_t address)
    {
        range_iter it=_simple_search(address), end=_data.end();
        if(it==end || !__has(it,address))
            throw std::invalid_argument("not in list");
        return it;
    }

    object search(addr_t address)
    {
        range_iter it = _simple_search(address), end=_data.end();
        if(it==end || !__has(it,address))
            return object();
        return it->second;
    }

    object detailed_search(addr_t address)
    {
        range_iter it=_simple_search(address), end=_data.end();
        if(it==end)
            return make_tuple(object(), begin());
        if(__has(it,address))
            return make_tuple(it,it);
        range_iter next=it;
        ++next;
        if(next==end)
            return make_tuple(it,object());
        return make_tuple(it, next);
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

    object at(const range_iter& it)
    {
        if(it==_data.end())
            throw invalid_argument("null iter");
        return it->second;
    }

    void merge(range_list& o)
    {
        for(range_iter it=o.begin(),end=o.end(); it!=end; ++it)
        {
            insert(it->second);
        }
    }
};

BOOST_PYTHON_MODULE(range_list)
{
    class_<range_list>("range_list")
    .def("__iter__", &range_list::all)
    .def("__len__", &range_list::size)
    .def("clear", &range_list::clear)
    .def("insert", &range_list::insert)
    .def("remove", &range_list::remove)
    .def("remove_address", &range_list::remove_address)
    .def("delete", &range_list::del)
    .def("merge", &range_list::merge)
    .def("search", &range_list::search)
    .def("index",&range_list::index)
    .def("begin",&range_list::begin)
    .def("end",&range_list::end)
    .def("at", &range_list::at)
    .def("slice",&range_list::slice)
    .def("slice",&range_list::slice2)
    .def("finger", &range_list::search)
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
    .def("next", &range_slice::next)
    ;
}

