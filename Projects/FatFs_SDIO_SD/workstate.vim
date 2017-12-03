let SessionLoad = 1
if &cp | set nocp | endif
let s:cpo_save=&cpo
set cpo&vim
inoremap <silent> <Plug>NERDCommenterInInsert  <BS>:call NERDComment(0, "insert")
nnoremap <silent>  :Rgrep
nnoremap <silent>  :RgrepAdd
map  h
map <NL> j
map  k
map  l
map  :cnext
map  :cprev
nmap  :tjump =expand("<cword>")
vnoremap <silent> # :call VisualSearch('b')
vnoremap $e `>a"`<i"
vnoremap $q `>a'`<i'
vnoremap $$ `>a"`<i"
vnoremap $3 `>a}`<i{
vnoremap $2 `>a]`<i[
vnoremap $1 `>a)`<i(
vnoremap <silent> * :call VisualSearch('f')
noremap ,ff :%s/$//g:%s// /g
nmap ,fd :cs find d =expand("<cword>"):copen
nmap ,fi :cs find i ^=expand("<cfile>")$:copen
nmap ,fe :cs find e =expand("<cword>"):copen
nmap ,ft :cs find t =expand("<cword>"):copen
nmap ,fc :cs find c =expand("<cword>"):copen
nmap ,fg :cs find g =expand("<cword>")
nmap ,fs :cs find s =expand("<cword>"):copen
nmap ,ca <Plug>NERDCommenterAltDelims
vmap ,cA <Plug>NERDCommenterAppend
nmap ,cA <Plug>NERDCommenterAppend
vmap ,c$ <Plug>NERDCommenterToEOL
nmap ,c$ <Plug>NERDCommenterToEOL
vmap ,cu <Plug>NERDCommenterUncomment
nmap ,cu <Plug>NERDCommenterUncomment
vmap ,cn <Plug>NERDCommenterNest
nmap ,cn <Plug>NERDCommenterNest
vmap ,cb <Plug>NERDCommenterAlignBoth
nmap ,cb <Plug>NERDCommenterAlignBoth
vmap ,cl <Plug>NERDCommenterAlignLeft
nmap ,cl <Plug>NERDCommenterAlignLeft
vmap ,cy <Plug>NERDCommenterYank
nmap ,cy <Plug>NERDCommenterYank
vmap ,ci <Plug>NERDCommenterInvert
nmap ,ci <Plug>NERDCommenterInvert
vmap ,cs <Plug>NERDCommenterSexy
nmap ,cs <Plug>NERDCommenterSexy
vmap ,cm <Plug>NERDCommenterMinimal
nmap ,cm <Plug>NERDCommenterMinimal
vmap ,c  <Plug>NERDCommenterToggle
nmap ,c  <Plug>NERDCommenterToggle
map ,cc :botright cope
map ,h :call ToHexModle()
noremap ,m mmHmt:%s///ge'tzt'm
map ,t :FufCWD **/
map ,s? z=
map ,sa zg
map ,sp [s
map ,sn ]s
map ,ss :setlocal spell!
map ,cd :cd %:p:h
map ,tm :tabmove
map ,tc :tabclose
map ,te :tabedit
map ,tn :tabnew %
map ,ba :1,300 bd!
map ,bd :Bclose
map <silent> , :noh
map ,g :vimgrep // **/*.<Left><Left><Left><Left><Left><Left><Left>
map ,t8 :setlocal shiftwidth=4
map ,t4 :setlocal shiftwidth=4
map ,t2 :setlocal shiftwidth=2
map ,e :e! ~/.vimrc
nmap ,qq :qa!
nmap ,w :w!
nmap 1 :bp
nmap 2 :bn
nnoremap ; :
nmap C :stj =expand("<cword>")
nmap \fd :cs find d =expand("<cword>"):copen
nmap \fi :cs find i ^=expand("<cfile>")$:copen
nmap \ff :cs find f =expand("<cfile>"):copen
nmap \fe :cs find e =expand("<cword>"):copen
nmap \ft :cs find t =expand("<cword>"):copen
nmap \fc :cs find c =expand("<cword>"):copen
nmap \fg :cs find g =expand("<cword>")
nmap \fs :cs find s =expand("<cword>"):copen
nmap \rs :source workstate.vim :rviminfo workstate.viminfo
nmap \ms :mksession! workstate.vim :wviminfo! workstate.viminfo
map fg : Dox
nmap gx <Plug>NetrwBrowseX
vnoremap <silent> gv :call VisualSearch('gv')
nnoremap j gj
nnoremap k gk
nmap ms :mksession! workstate.vim :wviminfo! workstate.viminfo
nmap mbt :MBEToggle
nmap rs :source workstate.vim :rviminfo workstate.viminfo
nmap sy :call Do_CsTag()
map tl :silent! Tlist
map <S-F9> :!gcc -O3 -o %< % 
map <F9> :!gcc -g -Wall -lm -o %< % 
nnoremap <silent> <Plug>NetrwBrowseX :call netrw#NetrwBrowseX(expand("<cWORD>"),0)
nmap <silent> <Plug>NERDCommenterAppend :call NERDComment(0, "append")
nnoremap <silent> <Plug>NERDCommenterToEOL :call NERDComment(0, "toEOL")
vnoremap <silent> <Plug>NERDCommenterUncomment :call NERDComment(1, "uncomment")
nnoremap <silent> <Plug>NERDCommenterUncomment :call NERDComment(0, "uncomment")
vnoremap <silent> <Plug>NERDCommenterNest :call NERDComment(1, "nested")
nnoremap <silent> <Plug>NERDCommenterNest :call NERDComment(0, "nested")
vnoremap <silent> <Plug>NERDCommenterAlignBoth :call NERDComment(1, "alignBoth")
nnoremap <silent> <Plug>NERDCommenterAlignBoth :call NERDComment(0, "alignBoth")
vnoremap <silent> <Plug>NERDCommenterAlignLeft :call NERDComment(1, "alignLeft")
nnoremap <silent> <Plug>NERDCommenterAlignLeft :call NERDComment(0, "alignLeft")
vmap <silent> <Plug>NERDCommenterYank :call NERDComment(1, "yank")
nmap <silent> <Plug>NERDCommenterYank :call NERDComment(0, "yank")
vnoremap <silent> <Plug>NERDCommenterInvert :call NERDComment(1, "invert")
nnoremap <silent> <Plug>NERDCommenterInvert :call NERDComment(0, "invert")
vnoremap <silent> <Plug>NERDCommenterSexy :call NERDComment(1, "sexy")
nnoremap <silent> <Plug>NERDCommenterSexy :call NERDComment(0, "sexy")
vnoremap <silent> <Plug>NERDCommenterMinimal :call NERDComment(1, "minimal")
nnoremap <silent> <Plug>NERDCommenterMinimal :call NERDComment(0, "minimal")
vnoremap <silent> <Plug>NERDCommenterToggle :call NERDComment(1, "toggle")
nnoremap <silent> <Plug>NERDCommenterToggle :call NERDComment(0, "toggle")
vnoremap <silent> <Plug>NERDCommenterComment :call NERDComment(1, "norm")
nnoremap <silent> <Plug>NERDCommenterComment :call NERDComment(0, "norm")
map <C-F5> :make
map <C-F8> :!gdb ./%<
map <C-F9> :!./%<
map <F2> :NERDTreeToggle
map <C-F7> :FufTaggedFile
map <F7> :FufTag
cnoremap  <Home>
cnoremap  <End>
cnoremap  
cnoremap  <Down>
cnoremap  <Up>
inoremap <expr>  omni#cpp#maycomplete#Complete()
cnoremap $q eDeleteTillSlash()
cnoremap $c e eCurrentFileDir("e")
cnoremap $j e ./
cnoremap $d e ~/Desktop/
cnoremap $h e ~/
inoremap <expr> . omni#cpp#maycomplete#Dot()
inoremap <expr> : omni#cpp#maycomplete#Scope()
inoremap <expr> > omni#cpp#maycomplete#Arrow()
iabbr xdate =strftime("%d/%m/%y %H:%M:%S")
let &cpo=s:cpo_save
unlet s:cpo_save
set autoindent
set autoread
set background=dark
set backspace=indent,eol,start
set completeopt=longest,menu
set cscopequickfix=c-,d-,e-,g-,i-,s-,t-
set display=lastline
set noequalalways
set expandtab
set fileencodings=usc-bom,utf-8,euc-jp,gb18030,gbk,gb2312,cp936,iso-8859-1
set fileformats=unix,dos,mac
set grepprg=/bin/grep\ -nH
set helplang=cn
set hidden
set history=300
set hlsearch
set ignorecase
set incsearch
set iskeyword=@,48-57,_,192-255,_,$,@,%,#,-
set laststatus=2
set lazyredraw
set matchtime=2
set mouse=a
set omnifunc=omni#cpp#complete#Main
set printoptions=paper:a4
set ruler
set runtimepath=~/.vim,~/.vim/bundle/vundle,/var/lib/vim/addons,/usr/share/vim/vimfiles,/usr/share/vim/vim73,/usr/share/vim/vimfiles/after,/var/lib/vim/addons/after,~/.vim/after,~/.vim/bundle/vundle/,/usr/local/lib/python2.7/dist-packages/powerline/bindings/vim/,~/.vim/bundle/vundle/after
set shiftwidth=2
set shortmess=atI
set showcmd
set showmatch
set showtabline=2
set smartindent
set smarttab
set softtabstop=2
set splitbelow
set statusline=\ %r%{Tlist_Get_Tagname_By_Line()}%h\ %w\ %F%m%r%h\ %w\ [FORMAT=%{&ff}]\ [TYPE=%Y]\ %w\ CWD:\ %r%{CurDir()}%h\ \ \ Line:\ %l/%L:%c
set suffixes=.bak,~,.swp,.o,.info,.aux,.log,.dvi,.bbl,.blg,.brf,.cb,.ind,.idx,.ilg,.inx,.out,.toc
set noswapfile
set switchbuf=usetab
set tabline=%!PowerlinePyeval('powerline.tabline()')
set tabstop=2
set textwidth=500
set updatetime=200
set whichwrap=b,s,,,[,],<,>,h,l
set wildmenu
set nowritebackup
let s:so_save = &so | let s:siso_save = &siso | set so=0 siso=0
let v:this_session=expand("<sfile>:p")
silent only
cd /mnt/Projects/STM32/BOARD_V1.0/FatFs_SDIO_SD
if expand('%') == '' && !&modified && line('$') <= 1 && getline(1) == ''
  let s:wipebuf = bufnr('%')
endif
set shortmess=aoO
badd +90 ~/.vimrc
badd +293 USER/main.c
badd +3202 FATFS/src/ff.c
silent! argdel *
edit USER/main.c
set splitbelow splitright
set nosplitright
wincmd t
set winheight=1 winwidth=1
argglobal
inoremap <buffer> 	 =CodeComplete()=SwitchRegion()
setlocal keymap=
setlocal noarabic
setlocal autoindent
setlocal nobinary
setlocal bufhidden=
setlocal buflisted
setlocal buftype=
setlocal cindent
setlocal cinkeys=0{,0},0),:,0#,!^F,o,O,e
setlocal cinoptions=
setlocal cinwords=if,else,while,do,for,switch
setlocal colorcolumn=
setlocal comments=sO:*\ -,mO:*\ \ ,exO:*/,s1:/*,mb:*,ex:*/,://
setlocal commentstring=/*%s*/
setlocal complete=.,w,b,u,t,i
setlocal concealcursor=
setlocal conceallevel=0
setlocal completefunc=
setlocal nocopyindent
setlocal cryptmethod=
setlocal nocursorbind
setlocal nocursorcolumn
setlocal nocursorline
setlocal define=
setlocal dictionary=
setlocal nodiff
setlocal equalprg=
setlocal errorformat=
setlocal expandtab
if &filetype != 'c'
setlocal filetype=c
endif
setlocal foldcolumn=0
setlocal foldenable
setlocal foldexpr=0
setlocal foldignore=#
setlocal foldlevel=0
setlocal foldmarker={{{,}}}
setlocal foldmethod=manual
setlocal foldminlines=1
setlocal foldnestmax=20
setlocal foldtext=foldtext()
setlocal formatexpr=
setlocal formatoptions=croql
setlocal formatlistpat=^\\s*\\d\\+[\\]:.)}\\t\ ]\\s*
setlocal grepprg=
setlocal iminsert=0
setlocal imsearch=0
setlocal include=
setlocal includeexpr=
setlocal indentexpr=
setlocal indentkeys=0{,0},:,0#,!^F,o,O,e
setlocal noinfercase
setlocal iskeyword=@,48-57,_,192-255,_,$,@,%,#,-
setlocal keywordprg=
set linebreak
setlocal linebreak
setlocal nolisp
setlocal nolist
setlocal makeprg=
setlocal matchpairs=(:),{:},[:]
setlocal modeline
setlocal modifiable
setlocal nrformats=octal,hex
set number
setlocal number
setlocal numberwidth=4
setlocal omnifunc=omni#cpp#complete#Main
setlocal path=
setlocal nopreserveindent
setlocal nopreviewwindow
setlocal quoteescape=\\
setlocal noreadonly
setlocal norelativenumber
setlocal norightleft
setlocal rightleftcmd=search
setlocal noscrollbind
setlocal shiftwidth=2
setlocal noshortname
setlocal smartindent
setlocal softtabstop=2
setlocal nospell
setlocal spellcapcheck=[.?!]\\_[\\])'\"\	\ ]\\+
setlocal spellfile=
setlocal spelllang=en
setlocal statusline=%!PowerlinePyeval('powerline.statusline(1)')
setlocal suffixesadd=
setlocal noswapfile
setlocal synmaxcol=3000
if &syntax != 'c'
setlocal syntax=c
endif
setlocal tabstop=2
setlocal tags=
setlocal textwidth=500
setlocal thesaurus=
setlocal noundofile
setlocal nowinfixheight
setlocal nowinfixwidth
setlocal wrap
setlocal wrapmargin=0
silent! normal! zE
let s:l = 295 - ((21 * winheight(0) + 19) / 39)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
295
normal! 0
tabnext 1
if exists('s:wipebuf')
  silent exe 'bwipe ' . s:wipebuf
endif
unlet! s:wipebuf
set winheight=1 winwidth=20 shortmess=atI
let s:sx = expand("<sfile>:p:r")."x.vim"
if file_readable(s:sx)
  exe "source " . fnameescape(s:sx)
endif
let &so = s:so_save | let &siso = s:siso_save
doautoall SessionLoadPost
unlet SessionLoad
" vim: set ft=vim :
