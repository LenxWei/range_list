# This is mostly a stripped down duplicate of dumb_buffer_stores
#   w/o hierarchy or types
class range_item:
    # "range" is reserved
    def __init__(self,address,size):
        self.address = address
        self.size = size
    def __str__(self):
        return "%x:%x"%(self.address,self.size)
class range_list(list):
    def _simple_search(self,address):
        # bsearch
        bot = 0; top = len(self)
        while bot < top:
            place = (top+bot)/2
            if address < self[place].address:
                top = place
            elif address >= self[place].address +\
                    self[place].size:
                bot = place+1
            else:
                return place

    def index(self,obj):
        idx = self._simple_search(obj.address)
        if idx != None and self[idx] == obj:
            return idx
        raise ValueError("%s not in list"%str(obj))

    def search(self,address):
        idx = self._simple_search(address)
        if idx == None:
            return None
        return self[idx]
    finger = search
        
    def _search(self,address):
        """Return:
        a direct hit: (index,index)
        a miss: (lower_index,upper_index)
        on a miss, if either of the indeces are invalid, it includes None instead"""
        bot = 0; top = len(self)
        if top == 0: return (None,None)
        while bot < top:
            place = (top+bot)/2
            if address < self[place].address:
                top = place
            elif address >= self[place].address +\
                    self[place].size:
                bot = place+1
            else:
                return (place,place)

        # No direct hits
        if self[place].address > address:
            # (???, place)
            if place == 0:
                return (None,place)
            return (place-1,place)
        if place == len(self)-1:
            return (place,None)
        return (place,place+1)

    detailed_search = _search

    def insert(self,obj,position=None):
        if position != None:
            list.insert(self,position,obj)
            return True

        start_l,start_h = self.detailed_search(obj.address)
        end_l,end_h = self.detailed_search(obj.address+obj.size-1)

        if start_l != end_l or start_h != end_h:
            return False

        if start_h != None:
            list.insert(self,start_h,obj)
        else:
            list.append(self,obj)
        return True

    def remove(self,obj):
        # Lists usually have .remove( element )
        #    so don't collapse remove( element )
        #    with remove( address )
        del self[ self.index(obj) ]

    def remove_address(self,address):
        del self[self.index(address)]
