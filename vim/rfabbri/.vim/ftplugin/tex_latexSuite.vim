" LaTeX filetype
"	  Language: LaTeX (ft=tex)
"	Maintainer: Srinath Avadhanula
"		 Email: srinath@fastmail.fm

"let g:Tex_DefaultTargetFormat='pdf'
"let g:tex_flavor = "pdflatex"
"let g:Tex_MultipleCompileFormats='pdf, aux'

"let g:Tex_ViewRule_dvi='okular'
"let g:Tex_ViewRule_pdf='okular'
"
let g:Tex_ViewRule_dvi='xdvi'
"let g:Tex_ViewRule_pdf='okular'
" mac
"let g:Tex_ViewRule_pdf='open'

if !exists('s:initLatexSuite')
	let s:initLatexSuite = 1
	exec 'so '.expand('<sfile>:p:h').'/latex-suite/main.vim'

	silent! do LatexSuite User LatexSuiteInitPost
endif

silent! do LatexSuite User LatexSuiteFileType
