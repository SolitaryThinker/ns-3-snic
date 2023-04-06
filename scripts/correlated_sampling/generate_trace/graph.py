#!/usr/bin/python3

import os
import sys

def ns_to_gbps(lat):
    pass

def main():
    print('graphing')
    print(sys.argv)

    if len(sys.argv) < 3:
        print('not enough args')
        sys.exit(1)
    filePrefix = sys.argv[1]
    numFiles = int(sys.argv[2])

    print('parsing', filePrefix, ' #', numFiles);

    values = []
    fileNames = []

    for i in range(numFiles):
        fileName = filePrefix+'_'+str(i)+'.txt'
        print(fileName)
        fileNames.append(fileName)

        values.append([])

        with open(os.path.join('../../../', fileName)) as fi:
            for line in fi:
                line = line.
                print(line,end='')
                values[i].append(line)




if __name__ == '__main__':
    main()
