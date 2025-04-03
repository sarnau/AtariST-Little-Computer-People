#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import struct
import binascii
import math
from PIL import Image, ImageDraw

COLORS = []
for color in '0000 0442 0265 0754 0310 0040 0754 0760 0247 0631 0700 0333 0555 0007 0777 0410'.split(' '):
	COLORS.append((int(color[1],16) * 32,int(color[2],16) * 32,int(color[3],16) * 32))

def textFile(filename):
	return open(filename, mode='r').read()

def decompressFile(filename):
	fileContent = open(filename, mode='rb').read()
	fileSize, = struct.unpack('>H', fileContent[0:2])
	header = fileContent[2:17]
	#print('%x [%s]' % (fileSize,header))
	data = fileContent[17:]
	offset = 0
	flag = True
	count = 0
	output = ''
	while offset < fileSize - 17:
		if flag:
			nibble = (data[offset] >> 4) & 0xF
			flag = False
		else:
			nibble = data[offset] & 0xF
			offset += 1
			flag = True
		if nibble != 0xF:
			output += chr(header[nibble])
		else:
			c = 0
			for i in range(0,2):
				c <<= 4
				if flag:
					c |= (data[offset] >> 4) & 0xF
					flag = False
				else:
					c |= data[offset] & 0xF
					offset += 1
					flag = True
			output += chr(c)
		count += 1
	return output

def decompressImageFile(filename):
	fileContent = open(filename, mode='rb').read()
	fileSize, = struct.unpack('>H', fileContent[0:2])
	header = struct.unpack('>15H', fileContent[2:2+15*2])
	#print('%x [%s]' % (fileSize,header))
	data = fileContent[32:]
	offset = 0
	flag = True
	count = 0
	output = ''
	while offset < fileSize - 32:
		if flag:
			nibble = (data[offset] >> 4) & 0xF
			flag = False
		else:
			nibble = data[offset] & 0xF
			offset += 1
			flag = True
		if nibble != 0xF:
			output += '%04.4x' % header[nibble]
		else:
			c = 0
			for i in range(0,4):
				c <<= 4
				if flag:
					c |= (data[offset] >> 4) & 0xF
					flag = False
				else:
					c |= data[offset] & 0xF
					offset += 1
					flag = True
			output += '%04.4x' % c
		count += 2
	return output

def saveImage(width,height,transp,fileContent,offset,filename):
	img = Image.new('RGBA', (width,height), color = 'white')
	draw = ImageDraw.Draw(img)
	words = math.ceil(width/16)
	for line in range(0,height):
		for word in range(0,words):
			planes = struct.unpack('>4H', fileContent[offset:offset+8])
			offset += 8
			for pixel in range(16):
				c0 = (planes[0] >> (15-pixel)) & 1
				c1 = (planes[1] >> (15-pixel)) & 1
				c2 = (planes[2] >> (15-pixel)) & 1
				c3 = (planes[3] >> (15-pixel)) & 1
				col = (c0 << 0) + (c1 << 1) + (c2 << 2) + (c3 << 3)
				if transp:
					if col:
						transparent = 255
					else:
						transparent = 0
				else:
					transparent = 255
				draw.point([pixel + word * 16, line], (COLORS[col][0],COLORS[col][1],COLORS[col][2],transparent))
	img.save(filename)
	return offset

def loadScreens():
	ret = decompressImageFile('./DATA/HOUSE.SCN')
	saveImage(320,200,False,binascii.unhexlify(ret),0,'HOUSE.PNG')

	ret = decompressImageFile('./DATA/TITLE.SCN')
	saveImage(320,200,False,binascii.unhexlify(ret),0,'TITLE.PNG')

def loadSpritesOrObjects(filename,destpath,transp):
	name = filename.split('/')[-1]
	fileContent = open(filename, mode='rb').read()
	offset = 0
	index = 0
	while offset < len(fileContent):
		height,width = struct.unpack('>HH', fileContent[offset:offset+4])
		offset += 4
		offset = saveImage(width,height,transp,fileContent,offset,destpath + '%s_%d.png' % (name,index))
		index += 1

def loadCards():
	fileContent = open('./DATA/CARDS', mode='rb').read()
	offset = 0
	index = 0
	while offset < len(fileContent):
		offset = saveImage(16,24,False,fileContent,offset,'./CARDS/CARDS_%d.png' % (index))
		index += 1

def loadLCP(filename):
	name = filename.split('/')[-1].split('.')[0]
	fileContent = open(filename, mode='rb').read()
	count,size = struct.unpack('>HH', fileContent[:4])
	offset = 4
	index = 0
	while offset < len(fileContent):
		offset = saveImage(16,21,True,fileContent,offset,'./BODY/%s_%d.png' % (name,index))
		index += 1

if False:
	print('=' * 40)
	print(decompressFile('./DATA/LETTER.TXT'))
	print('=' * 40)
	print(decompressFile('./DATA/WORDPZ.TXT'))
	print('=' * 40)
	print(decompressFile('./DATA/WORDS'))
	print('=' * 40)
	print(textFile('./DATA/NAMES'))

#loadScreens()
#loadSpritesOrObjects('./DATA/OBJECTS','./OBJECTS/', False)
#loadSpritesOrObjects('./DATA/SPRITES','./SPRITES/', True)
#loadCards()
loadLCP('./DATA/BODY.LCP')
#loadLCP('./DATA/PE2.LCP')
