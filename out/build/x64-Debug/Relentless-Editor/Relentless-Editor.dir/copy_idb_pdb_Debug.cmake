# CMake generated file
# The compiler generated pdb file needs to be written to disk
# by mspdbsrv. The foreach retry loop is needed to make sure
# the pdb file is ready to be copied.

foreach(retry RANGE 1 30)
  if (EXISTS "C:/Users/emilf/Desktop/Relentless/out/build/x64-Debug/Relentless/Relentless.dir/${PDB_PREFIX}Relentless.pdb" AND (NOT EXISTS "C:/Users/emilf/Desktop/Relentless/out/build/x64-Debug/Relentless-Editor/Relentless-Editor.dir/${PDB_PREFIX}Relentless.pdb" OR NOT "C:/Users/emilf/Desktop/Relentless/out/build/x64-Debug/Relentless-Editor/Relentless-Editor.dir/${PDB_PREFIX}Relentless.pdb  " IS_NEWER_THAN "C:/Users/emilf/Desktop/Relentless/out/build/x64-Debug/Relentless/Relentless.dir/${PDB_PREFIX}Relentless.pdb"))
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy "C:/Users/emilf/Desktop/Relentless/out/build/x64-Debug/Relentless/Relentless.dir/${PDB_PREFIX}Relentless.pdb" "C:/Users/emilf/Desktop/Relentless/out/build/x64-Debug/Relentless-Editor/Relentless-Editor.dir/${PDB_PREFIX}" RESULT_VARIABLE result  ERROR_QUIET)
    if (NOT result EQUAL 0)
      execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 1)
    else()
      break()
    endif()
  elseif(NOT EXISTS "C:/Users/emilf/Desktop/Relentless/out/build/x64-Debug/Relentless/Relentless.dir/${PDB_PREFIX}Relentless.pdb")
    execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 1)
  endif()
endforeach()
