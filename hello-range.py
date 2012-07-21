#  Copyright Joel de Guzman 2002-2007. Distributed under the Boost
#  Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
#  or copy at http://www.boost.org/LICENSE_1_0.txt)
#  Hello World Example from the tutorial

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
	