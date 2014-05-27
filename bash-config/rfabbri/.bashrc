# This file is sourced by all *interactive* bash shells on startup,
# including some apparently interactive shells such as scp and rcp
# that can't tolerate any output.  So make sure this doesn't display
# anything or bad things will happen !

# Test for an interactive shell.  There is no need to set anything
# past this point for scp and rcp, and it's important to refrain from
# outputting anything in those cases.
if [[ $- != *i* ]] ; then
	# Shell is non-interactive.  Be done now!
	return
fi

# XXX Bug for my old KDE
#xmodmap $HOME/.Xmodmap

# append to the history file, don't overwrite it
shopt -s histappend

# check the window size after each command and, if necessary,
# update the values of LINES and COLUMNS.
shopt -s checkwinsize

# make less more friendly for non-text input files, see lesspipe(1)
[ -x /usr/bin/lesspipe ] && eval "$(SHELL=/bin/sh lesspipe)"


# set variable identifying the chroot you work in (used in the prompt below)
if [ -z "$debian_chroot" ] && [ -r /etc/debian_chroot ]; then
    debian_chroot=$(cat /etc/debian_chroot)
fi

# set a fancy prompt (non-color, unless we know we "want" color)
case "$TERM" in
    xterm-*color*) color_prompt=yes;;
esac

# uncomment for a colored prompt, if the terminal has the capability; turned
# off by default to not distract the user: the focus in a terminal window
# should be on the output of commands, not on the prompt
#force_color_prompt=yes

if [ -n "$force_color_prompt" ]; then
    if tput setaf 1 >&/dev/null; then
	# We have color support; assume it's compliant with Ecma-48
	# (ISO/IEC-6429). (Lack of such support is extremely rare, and such
	# a case would tend to support setf rather than setaf.)
	color_prompt=yes
    else
	color_prompt=
    fi
fi

if [ "$color_prompt" = yes ]; then
    PS1='${debian_chroot:+($debian_chroot)}\[\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\$ '
else
    PS1='${debian_chroot:+($debian_chroot)}\u@\h:\w\$ '
fi
unset color_prompt force_color_prompt


# Change the window title of X terminals 
case ${TERM} in
	xterm*|rxvt*|Eterm|aterm|kterm|gnome)
		PROMPT_COMMAND='echo -ne "\033]0;${USER}@${HOSTNAME%%.*}:${PWD/$HOME/~}\007"'
		;;
	screen)
		PROMPT_COMMAND='echo -ne "\033_${USER}@${HOSTNAME%%.*}:${PWD/$HOME/~}\033\\"'
		;;
esac

# enable color support of ls and also add handy aliases
# xxx if [ -x /usr/bin/dircolors ]; then
if [ -x /sw/bin/gdircolors ]; then
    test -r ~/.dircolors && eval "$(gdircolors -b ~/.dircolors)" || eval "$(gdircolors -b)"
    alias ls='ls --color=auto'
    #alias dir='dir --color=auto'
    #alias vdir='vdir --color=auto'

    alias grep='grep --color=auto'
    alias fgrep='fgrep --color=auto'
    alias egrep='egrep --color=auto'
fi

# VARIABLES

#CVSROOT=/vision/projects/cvsroot
CVS_RSH=ssh
LEMSVXL=$HOME/cprg/vxlprg/lemsvxl
VXL_SRC=$HOME/cprg/vxlprg/vxl
EDITOR=vim
PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig
CDPATH=.:..:~:$LEMSVXL:$VXL_SRC/core:$VXL_SRC/contrib:$VXL_SRC/contrib/brl:$VXL_SRC/contrib/brl/bbas:$VXL_SRC/contrib/gel/mrc:$LEMSVXL/brcv:$LEMSVXL/basic:$LEMSVXL/brcv/rec:$LEMSVXL/brcv/seg:$LEMSVXL/contrib/rfabbri:$LEMSVXL/brcv/mvg:/vision:/mnt/
#TEXMF="/usr/share/texmf"
TEXINPUTS=".:$HOME/lib/texmf/tex//:$TEXINPUTS"
BIBINPUTS=".:/home/rfabbri/lib/doc/bib:/home/rfabbri/lib/doc/old-reference/bib/kimia/vision/:/home/rfabbri/lib/doc/old-reference/bib/kimia/vision/shape/:/home/rfabbri/lib/doc/old-reference/bib/kimia/bib-admin/:/home/rfabbri/lib/doc/old-reference/bib/kimia/math/:/home/rfabbri/lib/doc/old-reference/bib/kimia/:/home/rfabbri/lib/doc/old-reference/bib/kimia/vision/shape-from/:/home/rfabbri/lib/doc/old-reference/bib/kimia/psychology/:/home/rfabbri/lib/doc/old-reference/bib/kimia/vision/segmentation/:/home/rfabbri/lib/doc/old-reference/bib/kimia/medical/:/home/rfabbri/lib/doc/old-reference/bib/kimia/vision/texture:/vision/docs/kimia/reference/:/vision/docs/kimia/reference/old-reference/bib/vision/:/vision/docs/kimia/reference/old-reference/bib/vision/shape/:/vision/docs/kimia/reference/old-reference/bib/bib-admin/:/vision/docs/kimia/reference/old-reference/bib/math/:/vision/docs/kimia/reference/old-reference/bib/:/vision/docs/kimia/reference/old-reference/bib/vision/shape-from/:/vision/docs/kimia/reference/old-reference/bib/psychology/:/vision/docs/kimia/reference/old-reference/bib/vision/segmentation/:/vision/docs/kimia/reference/old-reference/bib/medical/:/vision/docs/kimia/reference/old-reference/bib/vision/texture"
XDVIINPUTS=/usr/share/texmf/xdvi:/usr/share/texmf/dvips/config
NNTPSERVER='news.brown.edu'
#MFINPUTS=".:$HOME/lib/texmf/fonts//"
VISUAL=vim
PAGER=/usr/bin/less
#LANG="en_US.utf8"
export LANG=en_US.UTF-8
export LC_ALL=en_US.UTF-8
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/rfabbri/src/bundler/bin
CLASSPATH=$CLASSPATH:$HOME/lib/java
BROWSER=/usr/bin/firefox
PYTHONPATH=$PYTHONPATH:$HOME/lib/python
GREP_OPTIONS='--color=auto'

export PATH CDPATH VXL_DIR EDITOR PKG_CONFIG_PATH CVS_RSH TEXINPUTS XDVIINPUTS BIBINPUTS NNTPSERVER VISUAL LANG LD_LIBRARY_PATH PAGER CLASSPATH BROWSER PYTHONPATH GREP_OPTIONS

#eval `dircolors -b`


# ALIASES
# You may want to put all your additions into a separate file like
# ~/.bash_aliases, instead of adding them here directly.
# See /usr/share/doc/bash-doc/examples in the bash-doc package.

if [ -f ~/.bash_aliases ]; then
    . ~/.bash_aliases
fi


ulimit -Sc hard

# PROMPT

function proml {
local BLUE="\[\033[0;34m\]"
local RED="\[\033[0;31m\]"
local LIGHT_RED="\[\033[1;31m\]"
local WHITE="\[\033[1;37m\]"
local NO_COLOUR="\[\033[0m\]"
case $TERM in
    xterm*)
        TITLEBAR='\[\033]0;\u@\h:\w\007\]'
        ;;
    *)
        TITLEBAR=""
        ;;
esac

PS1="${TITLEBAR}\
$BLUE[$RED\$(date +%H:%M)$BLUE]\
$BLUE[$LIGHT_RED\u@\h:\w$BLUE]\r
$WHITE\$$NO_COLOUR "
PS2='> '
PS4='+ '
}

function shortpwd {
   pwd_length=20

   DIR=`pwd`

   echo $DIR | grep "^$HOME" >> /dev/null

   if [ $? -eq 0 ]
   then
     CURRDIR=`echo $DIR | awk -F$HOME '{print $2}'`
     newPWD="~$CURRDIR"

     if [ $(echo -n $newPWD | wc -c | tr -d " ") -gt $pwd_length ]
     then
       newPWD="~/..$(echo -n $PWD | sed -e "s/.*\(.\{$pwd_length\}\)/\1/")"
     fi
   elif [ "$DIR" = "$HOME" ]
   then
     newPWD="~"
   elif [ $(echo -n $PWD | wc -c | tr -d " ") -gt $pwd_length ]
   then
     newPWD="..$(echo -n $PWD | sed -e "s/.*\(.\{$pwd_length\}\)/\1/")"
   else
     newPWD="$(echo -n $PWD)"
   fi

}

PROMPT_COMMAND=shortpwd
PS1='\u@\h:$newPWD$ '

source "$HOME/bin/funcoeszz"
export ZZPATH="$HOME/funcoeszz"

#setterm -blength 0

# enable programmable completion features (you don't need to enable
# this, if it's already enabled in /etc/bash.bashrc and /etc/profile
# sources /etc/bash.bashrc).
if [ -f /etc/bash_completion ] && ! shopt -oq posix; then
    . /etc/bash_completion
fi

#xmodmap $HOME/.xmodmap
