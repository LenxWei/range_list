s=1000000

class range_item:
    # "range" is reserved
    def __init__(self,address,size):
        self.address = address
        self.size = size
    def __str__(self):
        return "%x:%x"%(self.address,self.size)
    def __eq__(self,o):
        return self.address==o.address and self.size==o.size

