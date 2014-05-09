let g:Tex_IgnoredWarnings =
	\'Underfull'."\n".
	\'Overfull'."\n".
	\'specifier changed to'."\n".
	\'You have requested'."\n".
	\'Missing number, treated as zero.'."\n".
	\'size of graphic'."\n".
	\'not found'."\n".
	\'There were undefined references'."\n".
	\'Reference '."\n".
	\'Citation %.%# undefined'

TCLevel 10

let g:Tex_CompileRule_dvi = 'latex -src-specials -interaction=nonstopmode $*'
TCTarget dvi

"SpellSetLanguage br
"highlight SpellErrors  guifg=Red
