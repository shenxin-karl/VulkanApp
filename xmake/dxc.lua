function link_dxc_compiler(dxcDir) 
    local libDir = nil
    if is_arch("x86") then
        libDir = path.join(dxcDir, "lib", "x86")
    elseif is_arch("x64", "x86_64") then
        libDir = path.join(dxcDir, "lib", "x64")
    elseif is_arch("arm64") then
        libDir = path.join(dxcDir, "lib", "arm64")
    end
    add_linkdirs(libDir)
    add_sysincludedirs(path.join(dxcDir, "inc"), {public=true})
    add_links("dxcompiler")
end