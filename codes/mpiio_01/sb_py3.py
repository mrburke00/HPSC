#!/usr/bin/python3

import os

import sys
import getopt
import glob
import struct

# ==
# ||
# || Main Program
# ||
# ==

def sb(argv):

    os.system('clear')
    
    print()
    print( '------------------------------------')
    print( '   sb.py                            ')
    print( '------------------------------------')
    print()
    # -
    # |
    # | User arguments
    # |
    # -

    readFile = 'NULL'
    nPtsx    = 0
    
    try:
        opts, args = getopt.getopt(argv,"h f: x:")
      
    except:
       print( 'Error in command-line arguments.')
       exit(0)

    for opt, arg in opts:
        
        if opt == "-h":
            print( './sb.py -f <binary file to be read>')
            print( 'For in-class demo, try: ./sb_py3.py -f output.bin -x 10')
            exit(0)
        
        if opt == "-x":
            nPtsx = int(arg)
        
        elif opt == "-f":
            readFile = arg


    print()
    print('Read user specified file: ' )
    print('-------------------------------------')
    print()
    print('File name = ' + readFile)
    print()

    file = open(readFile, "rb")
    
    Num  = file.read(4)

    count = 0
    while Num:
        floatValue = struct.unpack('f',Num)[0]
        print ("{:5.1f}".format(floatValue),end='')

        count += 1
        if count == nPtsx:
            print ()
            count = 0
        Num  = file.read(4)

    file.close()


    
    
    print
    print

    

if __name__ == "__main__":
    sb(sys.argv[1:])



