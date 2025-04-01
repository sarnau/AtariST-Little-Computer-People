import struct
import binascii
import math

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

def loadSpritesOrObjects(filename):
	name = filename.split('/')[-1]
	with open(filename, mode='rb') as file:
		fileContent = file.read()
		offset = 0
		index = 0
		resolution = '0000'
		palette = ''.join('0000 0442 0265 0754 0310 0040 0754 0760 0247 0631 0700 0333 0555 0007 0777 0410'.split())
		while offset < len(fileContent):
			height,width = struct.unpack('>HH', fileContent[offset:offset+4])
			words = math.ceil(width/16)
			#print('%4x : %dx%d (%d)' % (offset,width,height,words))
			offset += 4
			image = bytearray(32000)
			for line in range(0,height):
				for word in range(0,words):
					for byte in range(0,2*4):
						image[160*line+word*8+byte] = fileContent[offset]
						offset += 1
			with open('%s_%d.PI1' % (name,index), mode='wb') as file:
				file.write(binascii.unhexlify(resolution+palette))# + fileContent[offset:offset + height * words * 8])
				file.write(image)
			index += 1

def loadCards():
	with open('./DATA/CARDS', mode='rb') as file:
		fileContent = file.read()
		offset = 0
		index = 0
		resolution = '0000'
		palette = ''.join('0000 0442 0265 0754 0310 0040 0754 0760 0247 0631 0700 0333 0555 0007 0777 0410'.split())
		height = 24
		words = 1
		while offset < len(fileContent):
			image = bytearray(32000)
			for line in range(0,height):
				for word in range(0,words):
					for byte in range(0,2*4):
						image[160*line+word*8+byte] = fileContent[offset]
						offset += 1
			with open('CARDS_%d.PI1' % (index), mode='wb') as file:
				file.write(binascii.unhexlify(resolution+palette))# + fileContent[offset:offset + height * words * 8])
				file.write(image)
			index += 1

def loadLCP(filename):
	name = filename.split('/')[-1].split('.')[0]
	with open(filename, mode='rb') as file:
		fileContent = file.read()
		count,size = struct.unpack('>HH', fileContent[:4])
		offset = 4
		index = 0
		resolution = '0000'
		palette = ''.join('0000 0442 0265 0754 0310 0040 0754 0760 0247 0631 0700 0333 0555 0007 0777 0410'.split())
		height = 21
		words = 1
		while offset < len(fileContent):
			image = bytearray(32000)
			for line in range(0,height):
				for word in range(0,words):
					for byte in range(0,2*4):
						image[160*line+word*8+byte] = fileContent[offset]
						offset += 1
			with open('%s_%d.PI1' % (name,index), mode='wb') as file:
				file.write(binascii.unhexlify(resolution+palette))# + fileContent[offset:offset + height * words * 8])
				file.write(image)
			index += 1

#print(decryptTextFile('./DATA/LETTER.TXT'))
#print(decryptTextFile('./DATA/WORDPZ.TXT'))
#print(decryptTextFile('./DATA/WORDS'))
#print(textFile('./DATA/NAMES'))
#loadScreens()
#loadSpritesOrObjects('./DATA/OBJECTS')
#loadSpritesOrObjects('./DATA/SPRITES')
#loadCards()
#loadLCP('./DATA/BODY.LCP')
