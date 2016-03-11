import sys

def handle_file(fileName):
   inputFile = open(fileName, 'r')
   outputFile = open(fileName + '.c', 'w')
   line = inputFile.readline().rstrip()
   while True:
       if not line.startswith('---'):
           break
       variableName = line[3:]
       print >> outputFile, "const char* const " + variableName + " ="
       while True:
          line = inputFile.readline()
          if not line or line.startswith('---'):
              break
          print >>outputFile, '"%s\\n"' % line.rstrip()
       print >>outputFile, ";"

def main():
    handle_file(sys.argv[1])

if __name__ == '__main__':
    main()
