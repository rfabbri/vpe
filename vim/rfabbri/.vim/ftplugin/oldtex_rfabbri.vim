"acentos:
so $HOME/.vim/acento.vim


map <F3> i$Ei$
map <F4> xa$
map ,b<F3> i\textbf{Ei}
map ,b<F4> xa}

"http://www.vim.org/howto/editing.latex.html
"insert environments while you type

"alt+e
"inoremap <M-e> \emph{}<Left>    

"alt+b
"inoremap <M-b> \textbf{}<Left>    

"alt+t
"inoremap <M-t> \texttt{}<Left>    

"alt+v 
"inoremap <M-v> \verb++<Left>   

"alt+m
"inoremap <M-m> $$<Left>

map! ]i \item 
map! ]bi \begin{itemize}\item 
map! ]ei \end{itemize}
map! ]be \begin{enumerate}\item 
map! ]ee \end{enumerate}
map! ]bd \begin{description}\item[
map! ]ed \end{description}
map! ]bc \begin{center}
map! ]ec \end{center}
map! [be {\samepage\begin{eqnarray}
map! [ee \end{eqnarray}}
map! ]bc \begin{comment}
map! ]ec \end{comment}
map! ]s1 \section{
map! ]s2 \subsection{
map! ]s3 \subsubsection{
map! ]p1 \paragraph{
map! ]p2 \subparagraph{
map! ]f \frac{
map! ]o \overline{
map! ]u \underline{
map! ;em bi{\em lea}i 

map ,inf :r $HOME/lib/notas/latex/fig_simples.tex
map ,inF :r $HOME/lib/notas/latex/fig.tex

map <F6> :!latex %
map <F9> :!xdvi %<.dvi&
map <F11> 0i%j

