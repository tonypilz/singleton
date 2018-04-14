import makeSingleHeaderHelpers as hlp

#config
singleHeaderProxy = '../src/globalInstances.h'
collapseNamespaceDetail = True
ns_global = 'global'
ns_detail = 'detail'
clang_format_style = hlp.LLVM


#execute
headerFiles = hlp.getIncludeFiles( singleHeaderProxy )
singleHeaderTarget = headerFiles.pop(0) #first header is the target header

includes, code = hlp.headerContents(headerFiles, ns_global)

codeJoined = hlp.collapsNamespace(ns_detail,code) if collapseNamespaceDetail else ''.join(code)

hlp.writeHeader(singleHeaderTarget, includes, codeJoined, ns_global)

hlp.clang_format_inplace(singleHeaderTarget,clang_format_style)

print ('done.')