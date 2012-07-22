from range_list import *
from random import randrange

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

for i in l:
	print i
	
x,y=l.detailed_search(100)
assert y==None
assert l.at(x)==range_item(90,5)

x,y=l.detailed_search(50)
assert y==x
assert l.at(x)==range_item(50,5)

x,y=l.detailed_search(49)
assert l.at(y)==range_item(50,5)
assert l.at(x)==range_item(40,5)

x,y=l.detailed_search(1)
assert x==None
assert l.at(y)==range_item(2,3)
