# Given an editable .nib, we only want the keyedobjects.nib file to be shipped, and
# CMake doesn't really have facilities for renaming files while copying into a bundle.
# Solution: shuffle the file around as needed by making a temp copy and renaming it
# instead of the original. That's what CopyNIB() does.
function(CopyNIB nib_source nib_dest)
  set(nib_source_file ${nib_source}/keyedobjects.nib)
  set(nib_temp_dir ${nib_dest}.tmp)
  set(nib_temp_file ${nib_temp_dir}/keyedobjects.nib)

  if (${nib_source_file} IS_NEWER_THAN ${nib_dest})
    file(COPY ${nib_source_file} DESTINATION ${nib_temp_dir})
    file(RENAME ${nib_temp_file} ${nib_dest})
    file(REMOVE ${nib_temp_dir})
  endif (${nib_source_file} IS_NEWER_THAN ${nib_dest})
endfunction(CopyNIB)
