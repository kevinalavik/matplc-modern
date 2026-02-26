import matplc_internal as C
import types

inited=0

class point:
	"""A class to give access to MatPLC points"""
	def __init__(self, a=None, b=None, c=None):
	    # 0 arguments: a null point
	    # 1 argument (name): by name
	    # 1 argument (point): copy point (for typecasts)
	    #    ie point_f(p) and point(p) work as expected
	    # 3 arguments (parent, bit, len): subpoint
	    if not inited:
	    	raise "must call matplc.init() first"
	    
	    if a==None and b==None and c==None:
	        self.pt = C.pt_null()
	    elif type(a)==types.StringType and b==None and c==None:
	        (self.pt, valid) = C.pt_by_name(a)
	        if not valid:
	            self.pt = None
	            raise KeyError, "matplc point \""+a+"\" not found"
	    elif type(a)==types.InstanceType and b==None and c==None:
	      self.pt = a.pt
	    elif type(a)==types.InstanceType and b!=None and c!=None:
	        (self.pt, valid) = C.subpt(a.pt, b, c)
	        if not valid:
	            self.pt = None
	            raise "could not make a subpoint"
	    else:
	        raise TypeError, "point() or point(name) or point(parent,bit,len)"

	def get(self):
		return C.plc_get(self.pt)
	def set(self, v=1):
		C.plc_set(self.pt, v)
	def reset(self, v=0):
		C.plc_set(self.pt, v)
	def __nonzero__(self):
		C.plc_get(self.pt)!=0
	def __len__(self):
		len=C.pt_len(self.pt)
		if len<=-1: raise "Something bad happened in plc_pt_len()"
		return len
	def __getitem__(self, i):
		if type(i)==types.SliceType:
			if i.step!=None and i.step!=1:
				raise "must not specify step for subpoints"
			return point(self, i.start, i.stop-i.start+1)
		else:
			return point(self, i, 1)

class point_f(point):
	"""A class to give access to MatPLC floating-point points"""
	def get(self):
		return C.plc_get_f(self.pt)
	def set(self, v):
		C.plc_set_f(self.pt, v)
	def reset(self, v=0.0):
		C.plc_set_f(self.pt, v)
	def __nonzero__(self):
		raise "can't implicitly test a floating-point point for truth"

def init(modname="python"):
	global inited
	if inited:
		if modname != inited[0]:
			raise inited[1]
	else:
		C.init(modname)
		inited=(modname, "already inited with a different name")

def pt_by_name(name): return point(name)
def scan_beg(): C.plc_scan_beg()
def scan_end(): C.plc_scan_end()
def update(): C.plc_update()
def get(p): return C.plc_get(p.pt)
def set(p,v): C.plc_set(p.pt, v)

#Only used for initial functionality tests:
# def test(name): return C.plc_test(name)

def done():
	global inited
	C.plc_done()
	# inited = 0
	inited = (None, "reiniting after done() not yet debugged")
