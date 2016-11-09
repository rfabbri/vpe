" this is mostly a matter of taste. but LaTeX looks good with just a bit
" of indentation.
set sw=2
set ts=2
" TIP: if you write your \label's as \label{fig:something}, then if you
" type in \ref{fig: and press <C-n> you will automatically cycle through
" all the figure labels. Very useful!
set iskeyword+=:
map \lc <C-W>j:q<CR><C-W>j:q<CR>
set keywordprg=myspell
let mapleader = ";"
ab parital partial
ab depht depth
"so $HOME/.vim/acento.vim
