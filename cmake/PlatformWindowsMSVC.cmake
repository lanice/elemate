
set(WIN32_COMPILE_DEFS
    WIN32                       # Windows system
    UNICODE                     # Use unicode
    _UNICODE                    # Use unicode
    #_SCL_SECURE_NO_WARNINGS     # Calling any one of the potentially unsafe methods in the Standard C++ Library
    #_CRT_SECURE_NO_DEPRECATE    # Disable CRT deprecation warnings
)

set(DEFAULT_COMPILE_DEFS_DEBUG
    ${WIN32_COMPILE_DEFS}
    _DEBUG                      # Debug build
)

set(DEFAULT_COMPILE_DEFS_RELEASE
    ${WIN32_COMPILE_DEFS}
    NDEBUG                      # Release build
)


set(WIN32_COMPILE_FLAGS
    "/nologo /Zc:wchar_t /Zc:forScope /GF /GR /Zi /fp:precise /MP /arch:AVX /W4"
    # nologo       -> no logo
    # Zc:wchar_t   -> treat wchar_t as built-in type: yes
    # Zc:forScope  -> force conformance in for loop scope: Yes
    # GF           -> string pooling
    # GR           -> runtime type information
    # GS           -> buffer security check
    # Zi           -> debug information format: program database
    # Gm           -> enable minimal rebuild
    # fp:precise   -> floating point model: precise
    # fp:fast      -> floating point model: fast
    # Ot           -> favor fast code
    # MP           -> build with multiple processes
    # wd           -> disable warning
    #   wd4100     -> 'identifier' : unreferenced formal parameter
    #   wd4127     -> conditional expression is constant
    #   wd4267     -> 'var' : conversion from 'size_t' to 'type', possible loss of data
    # W4           -> warning level 4
    # WX           -> treat warnings as errors

    # For debug:
    # MDd          -> Runtime Library: Multithreaded Debug DLL
    # Od           -> Optimization: none
    # RTC1         -> Runtime error checks

    # For release:
    # W3           -> warn level 3
    # MD           -> runtime library: multithreaded dll
    # Ox           -> optimization: full optimization
    # Ob2          -> inline function expansion: any suitable
    # Oi           -> enable intrinsic functions: yes
    # Ot           -> favor size or speed: favor fast code
    # Oy           -> omit frame pointers: yes
    # GL           -> whole program optimization: enable link-time code generation
    # GF           -> enable string pooling
    # GS-          -> buffer security check: no
    # arch:SSE2    -> enable enhanced instruction set: streaming simd extensions 2
)

set(DEFAULT_COMPILE_FLAGS ${WIN32_COMPILE_FLAGS})

set(DEFAULT_COMPILE_FLAGS_DEBUG "/MDd /RTC1c /Od /GS" )

set(DEFAULT_COMPILE_FLAGS_RELEASE "/MD /Ot /Ob2 /Ox /GS- /GL" )



set(WIN32_LINKER_FLAGS
    "/NOLOGO /NXCOMPAT /NODEFAULTLIB:libcmt.lib"
    # NOLOGO                                            -> suppress logo
    # INCREMENTAL:NO                                    -> enable incremental linking: no
    # MANIFEST                                          -> generate manifest: yes
    # MANIFESTUAC:"level='asInvoker' uiAccess='false'"  -> uac execution level: asinvoker, uac bypass ui protection: false
    # NXCOMPAT                                          -> data execution prevention (dep): image is compatible with dep
    # DYNAMICBASE:NO                                    -> randomized base address: disable image randomization
    # /NODEFAULTLIB:library                             -> disable warning according runtime library linking, see http://msdn.microsoft.com/EN-US/library/6wtdswk0%28v=VS.110,d=hv.2%29.aspx
                                                         # this warning is caused by physx runtime library linking.
)

set(DEFAULT_LINKER_FLAGS_DEBUG
    "${WIN32_LINKER_FLAGS} /DEBUG /DYNAMICBASE:NO"
    # DEBUG        -> create debug info
)

set(DEFAULT_LINKER_FLAGS_RELEASE
    "${WIN32_LINKER_FLAGS} /OPT:REF /LTCG /OPT:ICF /DELAY:UNLOAD /INCREMENTAL:NO"
    # OPT:REF      -> references: eliminate unreferenced data
    # OPT:ICF      -> enable comdat folding: remove redundant comdats
    # LTCG         -> link time code generation: use link time code generation
    # DELAY:UNLOAD -> delay loaded dll: support unload
)


# Add platform specific libraries for linking
set(EXTRA_LIBS "")
