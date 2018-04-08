import re
import subprocess

headerDir = '../src'
headerFiles = [
   'throwImpl.h',
   'DeferredOperations.h',
   'NullptrAccessHandler.h',
   'staticValue.h',
   'OptionalValue.h',
   'InstancePointer.h',
   'instance.h',
   'InstanceRegistration.h']

singleHeader = '../../include/globalInstances.h'

LLVM = 'LLVM'
Google = 'Google'
Chromium = 'Chromium'
Mozilla = 'Mozilla'
WebKit = 'WebKit'

style = LLVM

                         
includes = []
dst = []


def parse( fileContent ):
    bracketLevel = 0

    for line in fileContent:

        bracketLevel += line.count('{') - line.count('}')

        if line.startswith('#pragma once'):
            assert bracketLevel==0
            continue
        if line.startswith('#include \"'):
            assert bracketLevel==0
            continue
        if line.startswith('#include <'):
            assert bracketLevel==0
            includes.append(line)
            continue
        if line.startswith('namespace global'):
            assert bracketLevel==1
            continue
        if line.startswith('}') and bracketLevel == 0: # end of global scope
            continue
        if line =='\n' and bracketLevel <= 1:
            continue

        dst.append(line)

    dst.append('\n')
    

   

for headerFile in headerFiles:
    with open(headerDir + '/' +headerFile) as f:
        parse(f.readlines())

namespaceDetailMerged = re.sub('\n}[\n| ].*\n*namespace detail {.*\n', '', ''.join(dst))

with open(singleHeader,'w') as f:
    f.write('#pragma once\n')
    f.write('\n')
    f.writelines(includes)
    f.write('\n')
    f.write('namespace global {\n')
    f.write('\n')
    f.write(namespaceDetailMerged)
    f.write('\n')
    f.write('} // namespace global\n')
    f.write('\n')



subprocess.check_output( ['clang-format-5.0', '-i', '-style=' + style, singleHeader] )


print ('done.')