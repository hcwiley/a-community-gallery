import os
import Image
import time
import urllib
#os.system("curl 'http://gallery.art72.org/xml/gallery' > gallery.txt")
file = urllib.urlopen('http://gallery.art72.org/xml/gallery')
textfile = open("gallery.txt", 'w')
for i in range(10):
  os.system('RD /S /Q gallery%s' % i)
directory = 1
os.system('mkdir gallery1')
fileNum = 1
c = -1
de = ''
cc = 0
lines = file.readlines()
#print lines
string = ''
for line in lines:
    textfile.write(line)
    line = '%s' % line
    #print line
    line = line.replace('\n','')
    if c == -1:
        c += 1
        continue
    if c == 0:
        path = 'gallery%s' % directory
        name = line.split('/')
        name = name[len(name)-1]
        #print name
        #image = urllib.urlopen('%s' % (line))
        urllib.urlretrieve('%s' % line, "%s/%s" % (path, name))
        #os.system("curl '%s' > %s/%s" % (line, path, name))
        img = Image.open("%s/%s" % (path, name))
        img.save('%s/%s.jpg' % (path, fileNum))
        #print '%s/%s' % (path, name)
        os.system('RD "%s/%s"' % (path, name))
        #image.close()
        c += 1
    elif c == 1:
        c += 1
        continue
    elif c == 2:
        os.system("echo %s > %s/%s-title.txt" % (line, path, fileNum))
        c += 1
    elif c == 3:
        os.system("echo %s > %s/%s-artist.txt" % (line, path, fileNum))
        c += 1
    elif c == 4:
        os.system("echo %s > %s/%s-year.txt" % (line, path, fileNum))
        c += 1
    elif c == 5:
        os.system("echo %s > %s/%s-materials.txt" % (line, path, fileNum))
        c += 1
    elif c == 6:
        if de == '':
            de = open("%s/%s-description.txt" % (path, fileNum), "w")
        if line != '<<<EOF>>>':
            for char in line:
                if char != ' ':
                    if ord(char) == 226:
                      char = '"'
                    if ord(char) < 127 and ord(char) > 32:
                        string += char
                        cc += 1
                elif char == '\n':
                    print "%s " % (string)
                    de.write("%s " % (string))
                    string = '' 
                    cc += 1
                else:
                    de.write("%s " % (string))
                    string = '' 
                if cc > 50:
                    de.write('\n')
                    cc = 0
        else:
            de.write("%s " % (string))
            de.write('\n')
            de.flush()
            de.close()
            de = ''
            string = ''
            c = -1
            cc = 0
            fileNum += 1
            if fileNum > 4:
                fileNum = 1
                directory += 1 
                os.system('mkdir gallery%s' % directory)
file.close()
textfile.close()
#file = open("gallery.txt", 'w')
#file.write('%s' % directory - 1)
#file.close()
