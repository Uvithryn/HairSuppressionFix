set_xmakever('3.0.1')
includes('lib/CommonLibSSE-NG')

set_project('HairSuppressionFix')
set_version('1.3.0')
set_license('GPL-3.0')

set_languages('c++23')
set_warnings('allextra')
set_policy('package.requires_lock', true)
set_toolset('msvc', 'ninja')

add_rules('mode.debug', 'mode.releasedbg', 'mode.release')

-- These options are kept so the ClibDT GUI's --skyrim_se=y / --skyrim_ae=y /
-- --skyrim_vr=y command-line flags remain valid. Unlike the stock template,
-- all three now default to TRUE, matching CommonLibSSE-NG's own defaults.
-- That produces a single DLL covering SE 1.5.97, AE 1.6.x/1.7.x, and VR.
--
-- CommonLibSSE-NG defines ENABLE_SKYRIM_SE / _AE / _VR itself from its own
-- identically-named options, so this project does not add those defines.
--
-- Because all three defines are active at once, runtime selection happens at
-- RUNTIME via REL::Module::IsVR() and version checks. See Hooks/Biped.cpp.

option('skyrim_se')
    set_default(true)
    set_showmenu(true)
    set_description('Enable runtime support for Skyrim Special Edition')
option_end()

option('skyrim_ae')
    set_default(true)
    set_showmenu(true)
    set_description('Enable runtime support for Skyrim Anniversary Edition')
option_end()

option('skyrim_vr')
    set_default(true)
    set_showmenu(true)
    set_description('Enable runtime support for Skyrim VR')
option_end()

target('HairSuppressionFix')
    add_deps('commonlibsse-ng')

    add_rules('commonlibsse-ng.plugin', {
        name        = 'HairSuppressionFix',
        author      = 'Uvithryn',
        description = 'Suppresses character hair properly. This version includes Beard Mask Fix.'
    })

    add_files('src/**.cpp')
    add_headerfiles('src/**.h')

    add_includedirs(
        'src',
        '$(projectdir)',
        '$(projectdir)/ClibUtil',
        '$(projectdir)/ClibUtil/detail',
        '$(projectdir)/xbyak',
        '$(projectdir)/simpleini'
    )

    set_pcxxheader('src/pch.h')
