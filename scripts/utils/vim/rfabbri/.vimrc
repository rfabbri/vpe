set showmatch
set shiftwidth=2
set ts=2
set et
set ruler
set ic
set ai 
set tw=80
"set columns=90
" set lines=40
"set lines=32
set vb
set cmdheight=2

set printoptions=paper:letter

" memorization between sessions
set viminfo='10,\"30,:20,%,n~/.viminfo
au BufReadPost * if line("'\"")|execute("normal `\"")|endif

"completion
set wildmenu

set exrc			  " enable per-directory .vimrc files
set secure			" disable unsafe commands in local .vimrc files

"highlight all matches in a search
"set hls

"deactivate matches of current search
"map <F8> :nohlsearch

"treats long lines 
"nnoremap j gj
"nnoremap k gk
"set showbreak=>\ \ \ \ \ \ \  

"""******* Customizar isto para cd. tipo de arquivo
"map <F2> ^Mi^I^[
"F2 save (update)
map <silent> <F2> :up<CR>

map <F4> 15+
map <F6> :make
map <F7> :!ci -l %:e %
map <F8> :!rlog %
map <F9> :!./a.out
map <F10> 0i	j

map <S-Right> <C-PageDown>
map <S-Left> <C-PageUp>

"inoremap ^] ^X^]
"inoremap ^F ^X^F
"inoremap ^D ^X^D
"inoremap ^L ^X^L

"---------------------------------------------------
" SYNTAX CONFIGURATION
"if &term =~ "xterm"
" if has("terminfo")
"	set t_Co=8
"   set t_Sf=[3%p1%dm
"   set t_Sb=[4%p1%dm
" else
"   set t_Co=8
"   set t_Sf=[3%dm
"   set t_Sb=[4%dm
" endif
"endif

syn on
colorscheme torte
"colorscheme delek
"---------------------------------------------------
"Seleciona opcoes baseado no tipo de arquivo

filetype plugin on


"---------------------------------------------------
"SPELLING 

map <F12> :source $HOME/.vim/engspchk.vim

so $HOME/.vim/acento.vim


"disabling automatic spelling of certain documents
"this is annoying for me because I write both in english and
"portuguese
let spell_auto_type = ""


"---------------------------------------------------
" SPELLING 

"---------------------------------------------------
" C SUPPORT
let g:C_AuthorName      = 'Ricardo Fabbri'     
let g:C_AuthorRef       = 'rfabbri'                         
let g:C_Email           = 'rfabbri@lems.brown.edu'            
let g:C_Company         = 'Brown University'    
let g:C_LoadMenus      = "no"

let g:EnhCommentifyUseAltKeys = 'no'


"-----------

"cd %:h

"let g:tex_flavor = "latex"
noh
