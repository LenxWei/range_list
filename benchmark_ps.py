from range_list0 import *
from random import randrange, seed
import time
from config import *

seed(time.time())

l=range_list()
for i in xrange(1,s):
	a=range_item(randrange(0x7fffffff),randrange(0xff))
	l.insert(a)
	
for i in xrange(1,s):
	l.search(randrange(0x7fffffff))
