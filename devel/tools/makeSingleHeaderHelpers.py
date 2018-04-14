import re
import subprocess
import os.path


def extractFileFromIncludes( include ):
    return re.match('#include \"(.+)\"(.*)\n', include).group(1)

def extractFilesFromIncludes( fileContent ):
    res = []

    for line in fileContent:
        if line.startswith('#include \"'):
             res.append(extractFileFromIncludes(line))

    return res

def getIncludeFiles( proxyHeader ):
    dir_ = os.path.dirname(proxyHeader)
    with open(proxyHeader,'r') as f:
        headers = extractFilesFromIncludes(f.readlines())
        return [dir_ + '/' + header for header in headers]


def headerContent( fileContent, ns ):

    includes = []
    dst = []
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
        if line.startswith('namespace ' + ns):
            assert bracketLevel==1
            continue
        if line.startswith('}') and bracketLevel == 0: # end of ns scope
            continue
        if line =='\n' and bracketLevel <= 1:
            continue

        dst.append(line)

    dst.append('\n')
    assert bracketLevel==0

    return includes,dst


def headerContents( headers, ns ): 

    inc = []
    dst_ = []

    for headerFile in headers:
        with open(headerFile) as f:
            i, d = headerContent(f.readlines(), ns)
            inc.extend(i)
            dst_.extend(d)

    return inc, dst_


def collapsNamespace( ns, lines ):
    return re.sub('\n}[\n| ].*\n*namespace ' + ns + ' {.*\n', '', ''.join(lines))

def assembleHeader(includes, body, ns) :
    r = ''
    r = r + '#pragma once\n'
    r = r + '\n'
    r = r + includes
    r = r + '\n'
    r = r + 'namespace ' + ns + ' {\n'
    r = r + '\n'
    r = r + body
    r = r + '\n'
    r = r + '} // namespace ' + ns + '\n'
    r = r + '\n'

    return r



def writeHeader(singleHeaderTarget, includes, codeJoined, ns):
    with open(singleHeaderTarget,'w') as f:
        f.write(assembleHeader(''.join(includes),codeJoined,ns))



LLVM = 'LLVM'
Google = 'Google'
Chromium = 'Chromium'
Mozilla = 'Mozilla'
WebKit = 'WebKit'


def clang_format_inplace(file, style):
    subprocess.check_output( ['clang-format-5.0', '-i', '-style=' + style, file] )