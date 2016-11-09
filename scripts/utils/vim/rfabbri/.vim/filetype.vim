" my filetype file
if exists("did_load_filetypes")
  finish
endif
augroup filetypedetect
  au! BufRead,BufNewFile *.\(c_incl\)		      setfiletype c
  au! BufRead,BufNewFile *.txx		            set filetype=cpp
  au! BufRead,BufNewFile *.\(sce\|scilab\|dem\)		setfiletype scilab
  au! BufRead,BufNewFile *.am.input		            setfiletype automake
  au BufNewFile,BufRead *.wiki    setf Wikipedia
augroup END
