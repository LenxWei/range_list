from range_list0 import *
from random import randrange, seed
import time

seed(time.time())
s=100000
l=range_list()
for i in xrange(1,s):
	a=range_item(randrange(0x7fffffff),randrange(0xff))
	l.insert(a)
	