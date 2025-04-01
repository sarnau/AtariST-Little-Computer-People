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
	with open(filename, mode='r') as file:
		return file.read()

def decryptTextFile(filename):
	with open(filename, mode='rb') as file:
		fileContent = file.read()
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

def decryptImageFile(filename):
	with open(filename, mode='rb') as file:
		fileContent = file.read()
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

def loadScreens():
	resolution = '0000'
	palette = ''.join('0000 0442 0265 0754 0310 0040 0754 0760 0247 0631 0700 0333 0555 0007 0777 0410'.split())

	ret = decryptImageFile('./DATA/HOUSE.SCN')
	with open('HOUSE.PI1', mode='wb') as file:
		file.write(binascii.unhexlify(resolution+palette+ret))

	ret = decryptImageFile('./DATA/TITLE.SCN')
	with open('TITLE.PI1', mode='wb') as file:
		file.write(binascii.unhexlify(resolution+palette+ret))

def loadSpritesOrObjects(filename,destpath):
	name = filename.split('/')[-1]
	with open(filename, mode='rb') as file:
		fileContent = file.read()
		offset = 0
		index = 0
		while offset < len(fileContent):
			height,width = struct.unpack('>HH', fileContent[offset:offset+4])
			img = Image.new('RGB', (width,height), color = 'white')
			draw = ImageDraw.Draw(img)
			words = math.ceil(width/16)
			#print('%4x : %dx%d (%d)' % (offset,width,height,words))
			offset += 4
			for line in range(0,height):
				for word in range(0,words):
					p0 = (fileContent[offset+0] << 8) + fileContent[offset+1]
					p1 = (fileContent[offset+2] << 8) + fileContent[offset+3]
					p2 = (fileContent[offset+4] << 8) + fileContent[offset+5]
					p3 = (fileContent[offset+6] << 8) + fileContent[offset+7]
					offset += 8
					for pixel in range(16):
						c0 = (p0 >> (15-pixel)) & 1
						c1 = (p1 >> (15-pixel)) & 1
						c2 = (p2 >> (15-pixel)) & 1
						c3 = (p3 >> (15-pixel)) & 1
						col = (c0 << 3) + (c1 << 2) + (c2 << 1) + (c3 << 0)
						draw.point([pixel + word * 16, line], COLORS[col])
			img.save(destpath + '%s_%d.png' % (name,index))
			index += 1

def loadCards():
	with open('./DATA/CARDS', mode='rb') as file:
		fileContent = file.read()
		offset = 0
		index = 0
		height = 24
		words = 1
		while offset < len(fileContent):
			img = Image.new('RGB', (words * 16,height), color = 'white')
			draw = ImageDraw.Draw(img)
			for line in range(0,height):
				for word in range(0,words):
					p0 = (fileContent[offset+0] << 8) + fileContent[offset+1]
					p1 = (fileContent[offset+2] << 8) + fileContent[offset+3]
					p2 = (fileContent[offset+4] << 8) + fileContent[offset+5]
					p3 = (fileContent[offset+6] << 8) + fileContent[offset+7]
					offset += 8
					for pixel in range(16):
						c0 = (p0 >> (15-pixel)) & 1
						c1 = (p1 >> (15-pixel)) & 1
						c2 = (p2 >> (15-pixel)) & 1
						c3 = (p3 >> (15-pixel)) & 1
						col = (c0 << 3) + (c1 << 2) + (c2 << 1) + (c3 << 0)
						draw.point([pixel + word * 16, line], COLORS[col])
			img.save('./CARDS/CARDS_%d.png' % (index))
			index += 1

def loadLCP(filename):
	name = filename.split('/')[-1].split('.')[0]
	with open(filename, mode='rb') as file:
		fileContent = file.read()
		count,size = struct.unpack('>HH', fileContent[:4])
		offset = 4
		index = 0
		height = 21
		words = 1
		while offset < len(fileContent):
			img = Image.new('RGB', (words * 16,height), color = 'white')
			draw = ImageDraw.Draw(img)
			for line in range(0,height):
				for word in range(0,words):
					p0 = (fileContent[offset+0] << 8) + fileContent[offset+1]
					p1 = (fileContent[offset+2] << 8) + fileContent[offset+3]
					p2 = (fileContent[offset+4] << 8) + fileContent[offset+5]
					p3 = (fileContent[offset+6] << 8) + fileContent[offset+7]
					offset += 8
					for pixel in range(16):
						c0 = (p0 >> (15-pixel)) & 1
						c1 = (p1 >> (15-pixel)) & 1
						c2 = (p2 >> (15-pixel)) & 1
						c3 = (p3 >> (15-pixel)) & 1
						col = (c0 << 3) + (c1 << 2) + (c2 << 1) + (c3 << 0)
						draw.point([pixel + word * 16, line], COLORS[col])
			img.save('./BODY/%s_%d.png' % (name,index))
			index += 1

#print(decryptTextFile('./DATA/LETTER.TXT'))
#print(decryptTextFile('./DATA/WORDPZ.TXT'))
#print(decryptTextFile('./DATA/WORDS'))
#print(textFile('./DATA/NAMES'))
#loadScreens()
#loadSpritesOrObjects('./DATA/OBJECTS','./OBJECTS/')
#loadSpritesOrObjects('./DATA/SPRITES','./SPRITES/')
#loadCards()
loadLCP('./DATA/BODY.LCP')
