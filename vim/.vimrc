filetype plugin indent on
set foldcolumn=3
" 在窗口里开始编辑一个缓冲区时设置 'foldlevel'。用于开始编辑时总是关闭所有的折叠 (值为零)，关闭某些折叠 (1) 或者没有折叠 (99)。
set foldlevelstart=99
" 设置折叠级别: 高于此级别的折叠会被关闭
set foldlevel=99




set tabstop=4
" (自动) 缩进每一步使用的空白数目
set noexpandtab


" 如果发现文件在 Vim 之外修改过而在 Vim 里面没有的话，自动重新读入。
set autoread

 
" 语法高亮
syntax on
syntax enable
set background=dark
"colorscheme solarized
colorscheme lucius
