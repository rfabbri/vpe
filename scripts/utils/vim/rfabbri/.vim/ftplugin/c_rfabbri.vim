"
" My local C settings (rfabbri)
" This is not the plugin 
"
so $HOME/.vim/sem_acento.vim
"so $HOME/.vim/acento.vim
set makeprg=mymake
"set tw=100
set path +=~/cprg/vxlprg/**
"set path +=~/cprg/vxlprg/lemsvxlsrc/**
"set path +=~/cprg/vxlprg/vxl/contrib/gel/vsol/**
"set path +=~/cprg/vxlprg/vxl/core/vgl/**
"set path +=~/cprg/vxlprg/vxl/contrib/gel/mrc/vpgl/**
"set path +=~/cprg/vxlprg/vxl/core/vnl/**
"set path +=~/cprg/vxlprg/vxl/core/vgui/**
"set path +=~/cprg/vxlprg/vxl/core/vul/**
"set path +=~/cprg/vxlprg/vxl/core/vil/**
"set path +=~/cprg/vxlprg/vxl/contrib/bbas/**
"set path +=~/cprg/vxlprg/vxl/contrib/bbas/bgui/**
"set path +=~/cprg/vxlprg/vxl/core/vsl/**
set suffixesadd=.h
set noai
set cindent
set formatoptions+=c
ab ae AnimalExport
"ab endl vcl_endl
"ab cout vcl_cout
"ab cerr vcl_cerr
ab uns unsigned
ab uint unsigned
ab dou double
"ab cos vcl_cos
"ab sin vcl_sin
ab stat static
"ab string vcl_string
"ab vector vcl_vector
"ab vetor vcl_vector
"ab pi vnl_math::pi
ab xxx XXX
"map <F5> :!make
"map <F5> :!gcc -o %< %
map <F8> :!ctags *.c *.h
"map <F9> :!%<
"map <F10> 0i	j
map <F11> 0i//j
map <S-F11> 0xxj
map <F12> :!ddd %< &
map ,c *#i/*<Esc>ea*/<Esc>:noh<CR>
" For the 'gf' command: the following changes names from e.g. vbl/bla.h to bla.h
set includeexpr=substitute(v:fname,'.*\/','','g')

" Set the tag file search order
set tags=./tags,tags,~/tags,~/cprg/autotools/animal/tags,~/sciprg/toolbx/siptoolbox/tags,~/cprg/vision-nonvxl/opencv-git/tags,~/cprg/vxlprg/tags,../tags,~/vision-nonvxl/opencv-git/tags,~/pd-repos/pd-svn-w-extended-git-plus-gem/gem-git/tags,~/cprg/vision-nonvxl/leptonica-1.68/tags,~/src/blender/tags,~/aulas/programacao-paralela/books/perfbook/CodeSamples/tags,~/src/scilab/tags

se keywordprg=myman

"Temporary:
map ,i ip_iv_[<Esc>lla]<Esc> 

"-------------------------------------------------------------------------------
" use of dictionaries
" 
" dictionary : List of file names that are used to lookup words
"              for keyword completion commands
"   complete : search the files defined by the 'dictionary' option
"-------------------------------------------------------------------------------
"
set dictionary+=$HOME/.vim/wordlists/c
set complete+=k
"
"-------------------------------------------------------------------------------
"  some additional hot keys
"-------------------------------------------------------------------------------
"
"     F4  -  show tag under curser in the preview window
"     F6  -  list all errors           
"     F7  -  display previous error
"     F8  -  display next error   

nmap  <silent> <F4>    :exe ":ptag ".expand("<cword>")<CR>
map   <silent> <F6>    :copen<CR>
map   <silent> <F7>    :cp<CR>
map   <silent> <F8>    :cn<CR>
"
imap  <silent> <F4>    <Esc>:exe ":ptag ".expand("<cword>")<CR>
imap  <silent> <F6>    <Esc>:copen<CR>
imap  <silent> <F7>    <Esc>:cp<CR>
imap  <silent> <F8>    <Esc>:cn<CR>

" Rfabbri's construct maps
" C comment
map   ;co              O/*  */<Esc>2hi
imap  ;co              /*  */<Esc>2hi


" This one inserts a {} block after a function definition
map    ;bl               o<Esc>0i{<CR>}<Esc>O

map ;a :A<CR>

syn region MySkip contained start="^\s*#\s*\(if\>\|ifdef\>\|ifndef\>\)" skip="\\$" end="^\s*#\s*endif\>" contains=MySkip 
 
let g:CommentDefines = "" 
 
hi link MyCommentOut2 MyCommentOut 
hi link MySkip MyCommentOut 
hi link MyCommentOut Comment 
 
map <silent> ,x :call ClearCommentDefine()<CR> 
 
function! AddCommentDefine() 
   let g:CommentDefines = "\\(" . expand("NDEBUG") . "\\)" 
   syn clear MyCommentOut 
   syn clear MyCommentOut2 
   exe 'syn region MyCommentOut start="^\s*#\s*ifndef\s\+' . g:CommentDefines . '\>" end=".\|$" contains=MyCommentOut2' 
   exe 'syn region MyCommentOut2 contained start="' . g:CommentDefines . '" end="^\s*#\s*\(endif\>\|else\>\|elif\>\)" contains=MySkip' 
endfunction 
 
function! ClearCommentDefine() 
   let g:ClearCommentDefine = "" 
   syn clear MyCommentOut 
   syn clear MyCommentOut2 
endfunction 

call AddCommentDefine()
