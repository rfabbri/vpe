so $HOME/.vim/sem_acento.vim
set makeprg=mvn\ compile\ -f\ ~/pet/pet/pom.xml

" error format for maven 3
set errorformat=[ERROR]\ %f:[%l\\,%v]\ %m
se shiftwidth=4 " playN
se ts=4 " playN
set noai
set cindent
set formatoptions+=c


set path +=~/src/playn/**
set path +=~/src/playn-samples/**
set path +=~/src/tripleplay/**
set path +=~/src/react/**
set path +=~/src/pet/pet/**

" Set the tag file search order

" SCILAB:
"set tags=./tags,tags,~/tags,~/src/playn/tags,~/src/tripleplay/tags,~/src/playn-samples/tags,~/src/react/tags,~/pet/pet/tags,~/src/scilab/tags

" PlayN:
set tags=./tags,tags,~/tags,~/src/playn/tags,~/src/tripleplay/tags,~/src/playn-samples/tags,~/src/react/tags,~/pet/pet/tags

ab xxx XXX
ab uns unsigned
ab uint unsigned
ab dou double
"ab cos vcl_cos
"ab sin vcl_sin
ab stat static
map <F11> 0i//j
map <S-F11> 0xxj
map ,c *#i/*<Esc>ea*/<Esc>:noh<CR>
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

" copied from plugin/c.vim by rfabbri
function! Java_Compile ()
	exe	":cclose"
	" update : write source file if necessary
	exe	":update"
    exe ":make"
	" open error window if necessary 
    
    exe	":botright cwindow"
    exe ":copen<CR>"
endfunction

map  <buffer>  <F9>       :call Java_Compile()<CR>
map <buffer> ;ll :call Java_Compile()<CR>
map <buffer> ;lp :cp<CR>
map <buffer> ;ln :cn<CR>
 
" Automatically open, but do not go to (if there are errors) the quickfix /
" location list window, or close it when is has become empty.
"
" Note: Must allow nesting of autocmds to enable any customizations for quickfix
" buffers.
" Note: Normally, :cwindow jumps to the quickfix window if the command opens it
" (but not if it's already open). However, as part of the autocmd, this doesn't
" seem to happen.
autocmd QuickFixCmdPost [^l]* nested cwindow
autocmd QuickFixCmdPost    l* nested lwindow

" remove trailing white space upon write
autocmd BufWritePre <buffer> :%s/\s\+$//e
