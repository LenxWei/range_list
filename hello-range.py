from range_list import *

a=range_item(2,3)
assert a!=None
l=range_list()
l.insert(a)
assert len(l)==1
b1=l.search(2)
assert a==b1
b2=l.search(10)
assert b2==None

for i in l:
	print i
	
