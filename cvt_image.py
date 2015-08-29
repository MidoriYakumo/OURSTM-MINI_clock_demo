#! /usr/bin/python
# -*- coding: utf-8 -*-
"""
Spyder Editor

This is a temporary script file.
"""

from PIL import Image
import numpy as np

template_h = '''\
extern const uint16_t %s[];
'''

template_c = '''\
const uint16_t %s[] = {%s};
'''


class MidImage():
	def fromFile(self, fn, resize = None): # Resize to any size you love
		self.fn = fn
		self.img = Image.open(fn)
		if resize:
			if type(resize) is tuple:
				w, h = resize
			else:
				w, h = self.img.size
				scale = max(w, h)/float(resize)
				w, h = int(round(w/scale)), int(round(h/scale))
			self.img = self.img.resize((w, h), Image.BILINEAR)
		
	def show(self):
		self.img.show()
	
	def toRgb4444(self):
		self.data = np.array(self.img.convert("RGBA"))
		h, w, d = self.data.shape
		res = np.zeros((h, w), dtype=np.uint16)
		for y in range(h):
			for x in range(w):
				if d==3:
					r, g, b = self.data[y, x]
					a = 255
				else:
					r, g, b, a = self.data[y, x]
				v = 0
				v |= (r >> 4)<<12
				v |= (g >> 4)<<8
				v |= (b >> 4)<<4
				v |= (a >> 4)
				res[y, x] = v
		self.data = res
		self.dtype = 4
		return res
		
	def toRgb565(self):
		self.data = np.array(self.img.convert("RGB"))
		h, w, d = self.data.shape
		res = np.zeros((h, w), dtype=np.uint16)
		for y in range(h):
			for x in range(w):
				r, g, b = self.data[y, x]
				v = 0
				v |= (r >> 3)<<11
				v |= (g >> 2)<<5
				v |= (b >> 3)
				res[y, x] = v
				
		self.data = res
		self.dtype = 3
		return res
		
	def dumpCode(self, fn, mode = None):
		if mode is None: mode = 'w'
		fh = open(fn+'.h', mode)
		fc = open(fn+'.c', mode)
		if mode =='w': 
			fh.write("#include <stdint.h>\n")
			fc.write("#include <stdint.h>\n")
		var = "res_img_"+self.fn
		for escape in ['/', '.']:
			var = var.replace(escape, '_')
		# Meta data before image data: Skip, Channels, Height, Width
		s = '%s, %s, %s,' % (self.dtype, self.data.shape[0], self.data.shape[1])
		skip = s.count(',') + 1
		s = ('%s, ' % skip) + s
		for i, d in enumerate(self.data.reshape(-1)):
			if i % 10 == 0: s+='\n\t' # Makes it beautiful
			s+=hex(d) + ",\t"
			
		fh.write(template_h % (var))
		fc.write(template_c % (var, s[:-2]))
		fh.close()
		fc.close()
		

img = MidImage()

#img.fromFile('res/bg.jpg', 200)
#img.toRgb565()
#img.dumpCode('res', 'a')

#img.fromFile('res/fg.png', 200)
#img.toRgb4444()
#img.dumpCode('res', 'a')

#Append this jpeg to res.c
img.fromFile('res/lp.jpg')
img.toRgb565()
img.dumpCode('res', 'a')
