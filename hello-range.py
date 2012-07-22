from range_list import *
from random import randrange

class range_item:
    # "range" is reserved
    def __init__(self,address,size):
        self.address = address
        self.size = size
    def __str__(self):
        return "%x:%x"%(self.address,self.size)
    def __eq__(self,o):
        return self.address==o.address and self.size==o.size

a=range_item(2,3)
assert a!=None
l=range_list()
l.insert(a)
assert len(l)==1
b1=l.search(2)
assert a==b1
b2=l.search(10)
assert b2==None

for i in xrange(1,10):
	a=range_item(i*10,5)
	l.insert(a)

print "all:"
for i in l:
	print i

x,y=l.detailed_search(100)
print "\ndetailed search(100)",l.at(x),y
assert y==None
assert l.at(x)==range_item(90,5)

x,y=l.detailed_search(50)
assert y==x
assert l.at(x)==range_item(50,5)

x,y=l.detailed_search(49)
assert l.at(y)==range_item(50,5)
assert l.at(x)==range_item(40,5)
assert x+1==y

x,y=l.detailed_search(1)
assert x==None
assert l.at(y)==range_item(2,3)

print "\nslice to end:"
i1=l.index(50)
for x in l.slice(i1):
	print x

print "\nslice:"
i2=l.index(80)
for x in l.slice(i1,i2):
	print x

l.remove_address(80)
print "after remove 80, now  len is", len(l)
l.delete(l.begin())
print "after remove the first item, now len is", len(l)
l.remove(l.at(l.end()-1))
print "after remove the last item, now len is", len(l)

i3=i1+1
print "i1+1:", l.at(i3)

l1=range_list()
l1.insert(range_item(100,5))
l1.insert(range_item(110,5))

l.merge(l1)
print "\nmerge:"
for x in l:
  print x

class range_item_s(range_item):
    # "range" is reserved
    def __init__(self,address,size,s):
        self.address = address
        self.size = size
        self.s=s
    def __str__(self):
        return "%x:%x - %s"%(self.address,self.size,self.s)

s="str"
a=range_item_s(256,5,s)
l.insert(a)

print "\ninsert a range_item with extra attr:"
b=l.search(257)
print "the item  is",b

