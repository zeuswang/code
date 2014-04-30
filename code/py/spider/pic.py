import urllib2
import Image
import cStringIO
pic_url = "http://image.suning.cn/content/catentries/00000000010315/000000000103157611/fullimage/000000000103157611_1.jpg"
file = urllib2.urlopen(pic_url)
tmpIm = cStringIO.StringIO(file.read())
img = Image.open(tmpIm)
img=img.resize((104,139))
img.save("test.jpg")
