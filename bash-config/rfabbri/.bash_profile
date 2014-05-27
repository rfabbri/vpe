# DO THE FOLLOWING PERIODICALLY
# Start keychain, passing as args all private keys to be cached
#keychain -q $HOME/.ssh/id_rsa
PATH=$PATH:$HOME/bin/mw-scripts:$HOME/bin/mw-cmd:/usr/local/bin/i686-pc-linux-gnu:/mnt/soft/bin:/sw/bin:/usr/local/moredata/scilab-5.2.2/bin:/home/rfabbri/src/apache-maven-3.0.4-bin/bin:.
PATH=$HOME/bin:/usr/local/bin:/opt/local/bin:/opt/local/sbin:$PATH
export PATH


#, other_key1, other_key2, ...
# That creates a file which must be sourced to set env vars
#source $HOME/.keychain/cortex.lems.brown.edu-sh
       
# This file is sourced by bash for login shells.  The following line
# runs your .bashrc and is recommended by the bash info pages.
[[ -f ~/.bashrc ]] && . ~/.bashrc

[[ -f ~/.git-completion.bash ]] && . ~/.git-completion.bash

#xmodmap $HOME/.xmodmap
echo U+24B6


bind "set completion-ignore-case on"
bind "set show-all-if-ambiguous on"
