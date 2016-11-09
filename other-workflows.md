# Further Ideas to make it easier to work on VXL + VXD + internal Project


# Subtrees for VPE

We're in VPE, and want to add VXD and VXL

## Adding plugin example from [2], for reference

    git remote add plugin ../remotes/plugin
    git fetch plugin
    git read-tree --prefix=vendor/plugins/demo -u plugin/master
    git commit -m "Added demo plugin subtree in vendor/plugins/demo"

## Adding VXL to VPE

    git remote add vxl ../remotes/vxl
    git fetch vxl
    git read-tree --prefix=vxl -u vxl/master # an alternative would be -u vxl-master
    git commit -m "Added vxl subtree in vxl/"
    git checkout -b vxl-master vxl/master  # keeps vxl history across git clones
    git push origin vxl-master

## Adding VXD to VPE
    git remote add vxd ../remotes/vxd
    git fetch vxd
    git read-tree --prefix=vxd -u vxd/master
    git commit -m "Added vxd subtree in vxd/"
    git checkout -b vxd-master vxd/master  # keeps vxd history across git clones
    git push origin vxl-master

## Pulling in changes from VXL

    git fetch vxl
    git merge -s subtree vxl/master  # this will join histories
    # git merge -X subtree=vxl/ vxl/master  # if the above goes crazy
    # git merge -s subtree --squash vxl/master # this will not join, untraceable
    # git commit -m "FROM VXL: update"
    git checkout vxl-master  # just to make it point to the right place
    git merge vxl/master
    git checkout master
    git push origin vxl-master

## Pulling in changes from VXD

    git fetch vxd
    git merge -s subtree vxd/master  # this will join histories
    # git merge -X subtree=vxd/ vxd/master  # if the above goes crazy
    # git merge -s subtree --squash vxd/master # this will not join, untraceable
    # git commit -m "FROM VXD: update"
    git checkout vxd-master  # just to make it point to the right place
    git merge vxd/master
    git checkout master
    git push origin vxd-master

## Making changes to VXL/VXD within VPE

### Checked out VPE fresh, made changes to VXL, Push upstream to VXL
When VPE is checked out, VXL's remote is not there. Add it if you want to
push/pull directly without passing through VPE

    git remote add ...

    In VPE we'll have booststrap/setup scripts for this.

### VXL/VXD change flow
Edit vxl/ normally

    # eg:  echo '// test' >> vxl/CMakeLists.txt   # an existing VXL file is edited
    # if we want that change to be backported to VXL, we prepend TO VXL:
    git ci -am "VPE->VXL: cmakelists"  # this message shows up on upstream
    # Or, if we forgot, we can tag the commit "TO-VXD"

Keep doing other commits to anywhere in the tree.  When backporting, we have to
cherry-pick when the original team has made free commits anywhere in the tree.
If you yourself are working on the tree, and separate your commits to vxl/ and
vxd/ folders in separate branches merged to your master, this becomes a rebase
instead of cherrypicking (see similar approach 2 below).

    git checkout vxl-master
    # usually works: 
    # git cherry-pick -x --strategy=subtree master
    # safer and more consistent:
    git cherry-pick -x --strategy=subtree -Xsubtree=vxl/ master

    # check if that generates a commit with the wrong prefix, if so,
    # undo the commit by resetting HEAD and give up.

    # use '-e' flag to cherry-pick to edit the commit message before passing upstream

    # --strategy=subtree (-s means something else in cherry-pick) also helps to make sure
    # changes outside of the subtree (elsewhere in container code) will get quietly
    # ignored. 

    # if merging, use the same strategies.
    #
    # the -x in these commands annotate the commit message with the SHA1 of VPE
    # Use the following to make sure files outside of the subtree (elsewhere in container code) 
    # will get quietly ignored. This may be useful when cherypicking a rename, since move is rm+add

    git push  # to push the vxl-master branch
    git push vxl HEAD:master

### Original example on which the above is based, showing the kinds of edits
    echo '// Now super fast' >> vendor/plugins/demo/lib/index.js
    git ci -am "[To backport] Faster plugin"
    date >> main-file-1
    git ci -am "Container-only work"
    date >> vendor/plugins/demo/fake-work
    date >> main-file-2
    git ci -am "[To backport] Timestamping (requires container tweaks)"
    echo '// Container-specific' >> vendor/plugins/demo/lib/index.js
    git ci -am "Container-specific plugin update"

## VXL/VXD -> VPE file move within VPE

    Works, just integrate the commit to VXL/VXD as the above directions for making
    any other type of change.

    History tracing:

## VPE -> VXL/VXD file move within VPE
    Works, just integrate the commit to VXD as the above directions for making
    any other type of change.

    History tracing:

## VXD -> VXL file move within VPE
    Works, just integrate the commit to VXD and VXL as the above directions for making
    any other type of change.

    History tracing:
  

## Remove VXD or any subtree

It’s just a directory in your repo. A good ol’ git rm will do.

    git rm -r vxl
    git commit -m "Removing vxl subtree"
    git remote rm vxl

    # perhaps optimzie git / todo: test tree size
    git gc  # repo still huge
    git gc --prune=now --aggressive  # till, 103M!


## History lookup

    git log --follow filename

This will show all logs, including renames and moves. It doesn't work
across subtrees, but is being fixed by the Git team [5].
If you use the subtree variant below, it works a bit better.

    git log -- '*filename'          # from the toplevel

This views each commit that touched a file whose name ends with filename. It
won't follow actual renames, and may show false positive if you have several
files with the same basename [1].

## Caveats
- problem 1: history of subtree not visible with git log. This is under
  development by the Git team for `git log --follow`.

## Benefits
The toplevel VPE team can edit any submodule and propagate changes to VPE peers,
using regular git workflow. The vxl-master and vxd-master branches track where
we are at these subprojects. Once in a blue-moon, we push to the subprojects.

Worst case scenario:
If we need to heavily sync to the subproject-specific remotes, two cases:

1. Experienced users (1 or 2 people in team)
Pushing often to the subproject is infeasible
- We'll make wild commits in VPE on a branch, push to VPE regularly.
- Once in a good while, we carefully push to the subproject.

2. Collaborators which only care to know regular Git (most of team)
- Work normally on VPE and push changes done everywhere in the repo to VPE
- Never update submodule directly; rely on experienced users to do so.
- Pulling in an updated VXL necessarily relies on VPE remote having done so.
  They will never do this independently. They rarely do anyways. If they really
  want to do so, then they learn how to do it. Which means we make it difficult
  (but far from impossible) for them to diverge from the VPE team on which VXL
  or VXD is being used.

# VPE repository as scripts over regular Git
A script that builds the VPE environment for a beginner or collaborator.
Something very much along the lines of https://github.com/github/scripts-to-rule-them-all

- creates everything in a toplevel vpe/ folder
- pulls in all repos, including recommended utility ones (utils-macambira and boost sources if need be)
- mkdir all bin 
- does an apt-get or port install for common packages
- install common sh aliases and git aliases
- moves between repos are history-tracked only by being disciplined and including the
  origin SHA1 in every cherrypick or merge
- have a bin/ or scripts/ subfolder for scripts
- to share your dev environment:
  - integrate and push to master in each, or:
  - push each to a separate branch in person's github for each
  - write in the vpe-build script or config file the corresponding branches
  - send the script to the person.
- one could have a "share" script which would go inside a separate or father
  repo, look up the SHA1 and currently checked out branch of the checked-in
  repos, and upload that. This little repo or file would track people's history
  of branches, and it could be user-specific, just so other people know where
  you were when you comitted.

Advantages
- current people keep working as is, just adding VXD to the loop
  
Locking commits and branches for sub-projects for sharing
  - separate repositories can achieve this by:
    - setting up a post-commit hook to write the commit SHA of the submodules to
      the parent repo. (basically, the .gitmodules is the actual good thing, to
      let users know what the maintainer is using).
    - the maintainer could have a script: mark-as-build, to populate a file with
      the id of the commit of the dependant repos plus the remote and branches being used (ouch).

# Subtree variant for VPE
This is currently used in VPE, see README.

# Submodules for VPE
- well supported
- bad: all users need to learn a lot of submodule lingo
- good: if the repo maintainer updates the sub-repos and pushes, users are
  locked to that sub-repo version.
  - If your peers do a mere git pull, it will pull-in the work VXL.
  - If your peers update vxl, everyone gets vxl
  - In that way, vxl is updated equally for everyone in the team,
  instead of each pulling a different vxl master. You sync to the team repo,
  rather than to the vxl team repo.
  (? can we track multiple versions of vxl?)
- in the simple use case of only editing and pushing to the container repo,
  naive users may not be aware vxl/ folders are not separate git repos
- basically the workflow has no base case for simple usage. It is always going
  to be complex
- advantage: much more well-defined and well-maintaned than subtrees if you
  know the lingo
- advantage: for advanced users it may provide a good way to share code in a
  predictable way, locking the subprojects to the commits that people are
  working at the moment.

Submodules differ from subtrees:
- submodules are separate git repositories inside a a bigger one, yet are not
  managed as usual git repositories within the bigger one. 
  - git log works nicely, with separate histories for each
- subtrees look more like a subfolder uniformly managed by git, but with tiny annoying quirks

- for submodules, collaborators won't automatically see updates to submodules—if
  you update a submodule, you may need to remind your colleagues to run git
  submodule update or they will likely see odd behavior.


For VPE, submodules are not attractive since:
- people's usual git workflow is broken
- commits made to sub projects may be lost.


##  Pulling in changes from VPE

    git spull   # alias or script. reminds me of the scripted superrepo approach

    git pull
    git submodule update     # always.. massive danger if you don’t, as 
                             # your next container commit will regress the submodule

#### If the submodule url changed, everyone has to:
    git pull
    git submodule sync
    git submodule update     # always.. massive danger if you don’t, as 
                             # your next container commit will regress the submodule

Spull alias cover all cases:

    git config --global alias.spull '!git pull && git submodule sync --recursive && git submodule update --init --recursive'

##  Pulling in changes from VXL/VXD

    cd vxl
    git fetch
    git merge origin/master
    cd .. 
    git diff --submodule 
    # you can see that the submodule was updated and get a list of commits that were added to it. 

    git commit -am
    git push
   

## Updating VXL/VXD within VPE

    # first, make sure update submodule master to remote master,
    # or, more generally, that you have a branch tracking the submodule

    cd vxl

    git status      # it will show detached state
    git checkout vxl-master     # if it doesn't exist, create it
    git merge origin/master

    # if you don't want to use that branch, create any other, as long as you are
    # inside the submodule, branches will be local.

    # Now make your changes and commit normally, within vxl/
    git push origin vxl-master

    cd ..

    # Part 2:  update VPE's repo to see if we have other branches or our own
    # vxl-master updated in VPE's repo
    git submodule update --remote --merge

    # If you forget the --rebase or --merge, Git will just update the submodule
    # to whatever is on the server and reset your project to a detached HEAD state.
    # Then you have to checkout vxl-master again and redo the work

    # on the toplevel, you won't use git push, but first
    git push --recurse-submodules=check

    
# Android Repo tool

- barely any reliable doc on the web for the VPE case



# Links

[1] https://git-scm.com/book/en/v2/Git-Tools-Advanced-Merging
[2] https://medium.com/@porteneuve/mastering-git-subtrees-943d29a798ec#.sjbirxm4y
[3] https://saintgimp.org/author/saintgimp/
[4] http://paste.ubuntu.com/11732805/ (tested, didn't work)
[5] http://stackoverflow.com/questions/10918244/git-subtree-without-squash-view-log/40349121#40349121

# Guidelines
- always upgrade git. use > 2

# Ideas
Some things to think about

- How do people actually manage changes across multiple Git repositories? Do we really have to do it all by hand?
- There could be a tool that tracks code that was moved across Git repositories.
  It could have some minimal functionality:
  - If you tag a move/git tree surgery by including in the commit of the
    destination repo the original repo SHA1, it could automatically detect code
    branching intra repository, or at least mark on the original repository code
    that has gone to another repository and where. A tool such as this must already exist!
  - There should be something like `git mv` across repositories, which
    automatically does the surgery of the history, even though this might not
    always be desired when moving code from Internal to a public one.

- In the future, VXD could be a sub module of VXL, and VXL would have a CMAKE
  flag BULD_VXD which is off by default.
  - VXD bundling VXL as a submodule

- [Google and Facebook are working on scaling up Mercurial and making it available.](https://www.wired.com/2015/09/google-2-billion-lines-codeand-one-place/) Perhaps we are outgrowing Git, and just have to wait to do things right.

