
table = [
    'Feature | This Lib | Classical Singleton | [herpe] | [ugrif] | [xytis] | [aworx] | [fwolt] | [zyf38] | [cheno] | [cppma]',
    'supports instance replacement for testing | X | - | - | - | - | - | - | - | X | X',
    '2-phase initialization avoidable| X | - | - | - | - | - | - | - | - | -',
    'control over construction seqence | full | limited | limited<sup>2</sup> | limited<sup>2</sup> | limited<sup>2</sup> | limited<sup>2</sup> | limited | limited<sup>2</sup> | limited<sup>2</sup> | full<sup>2</sup>',
    'control over destruction seqence | full | none | none | full | full | full | none | none | full | full',
    'control over destruction point in time |  full | none | none | full | full | full | none | none | full | full',
    'automatic destruction | X | X | X | - | -<sup>3</sup> | - | X | X | -<sup>4</sup> | X',
    'constructor arguments | X | - | X<sup>1</sup> | - | - | - | X | - | - | up to 4',
    'threadsave construction | - | X | X | - | - | X<sup>5</sup> | X | X | X | optional',
    'implementation pattern | indep. class | function | CRTP | macro |  indep. class  | CRTP | CRTP | indep. class | indep. class | indep. class',
    'forces virtual destructor | - | - | X | - | - | X | - | - | - | -',
    'thread local instances | - | - | - | - | - | - | - | - | X | -']

for x in range(11):

    line = ''
    for oldLine in table:
        line = line + ' | ' + oldLine.split('|')[x]

    line = line + ' |'

    print(line)

