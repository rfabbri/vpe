# VPE: Vision Programming Environment

VPE (the Vision Programming Environment) is a monolithic superproject (a.k.a.
monorepo) for tightly
coupled projects based on [VXL](http://vxl.sourceforge.net) and
[VXD](http://github.com/rfabbri/vxd).  It bundles VXL, VXD, project code and
related utilities into a unified programming environment, making the setup more
homogeneous among a team. Advantages:

- **No dependency mess:** everyone in the team use the same VXL and VXD versions when working on master.
- **Plain old Git:** most of the time, updating to/from VXL or VXD within VPE is tracked in VPE.
  Only the VPE team sees these changes, uniformly.
- **Separate repositories for upstream sharing:** subrepos are simultaneously kept in their own upstream repositories.
- **Harder to break things:** Propagating changes to/from upstream subrepos
  requires a specific git procedure; this creates a buffer between VPE and
  upstream VXL/VXD.  While it is seamless to share VXL or VXD changes with the
  team through VPE, one has to be disciplined to share with upstream, which
  should only occur when needed, with proper branches and code quality.
- **Promoting across VXL/VXD is tracked:** If you promote code across the
  different VXL-based projects, these operations will get documented and tracked
  as commits within VPE.

The basic idea is to replicate a repo hierarchy most VXD developers would have:
```
    vpe/
      vxl
      vxd
      scripts
      vxl-bin
      vxd-bin
```
Within `scripts/` one can find general VXL development scripts, such as scripts to aid
switching between build '-bin' and source folders, and general developer
scripts for searching code, configuring your Ubuntu or Mac OS system for
programming, etc.
  
The technique for building VPE is a variant of *git subtree*, mainly for the
above advantages. If we had many submodules, git submodules would be used.

## First steps: bootstrapping the environment

```
git clone http://github.com/rfabbri/vpe
cd vpe/
```

### If you just want to build and use VPE

Run the following script within vpe:
```
./setup-for-use
```
This will create symlinks for vxl-bin and vxd-bin

### If you want to develop VPE-based project heavily
Run the following script within vpe:
```
./setup-for-development
```
This will create useful `vxl-bin` and `vxd-bin` symlinks,
establish useful remotes and branches.

### A tour of VPE
Once bootstrapped for development, you will get the following files

```
other-workflows.md             Other possible workflows for VPE beyond subtrees

scripts/devsetup/              Scripts used by ./setup-for-development

scripts/utils/                 Utilities for VXL development, templates, shell/editor config

vxl/                           VXL folder tracked within VPE
vxl-bin -> vxl-bin-dbg         Default VXL build folder pointing to possible debug version
vxl-bin-dbg/                   VXL build folder with debug flags
vxl-bin-rel/                   VXL build folder without debug flags.
vxl-orig/                      Original VXL as a separate repository (mostly for history)

vxd/                           VXD folder tracked within VPE
vxd-bin -> vxd-bin-dbg         Default VXL build folder pointing to possible debug version
vxd-bin-dbg/                   VXD build folder with debug flags
vxd-bin-rel/                   VXD build folder without debug flags.
vxl-orig/                      Original VXD as a separate repository (mostly for history)
```

## Building VPE

### Compile VXL
```

  cd ./vxl-bin
  ccmake ../vxl
  make   # use mymake from scripts/utils/vxl to run it from from both vxl-bin and vxl
```

### Compile VXD
```
  cd ../vxd-bin
  ccmake ../vxd
  make'
```
For further information on building each of these libraries and the best CMake flags to
use, see http://wiki.nosdigitais.teia.org.br/VXL.

## Scripts to help with sourcecode

Follow the tips in `scripts/devsetup/tips` closely. 

### Put `scripts/utils` in your PATH

I recommend having a ~/bin folder in your PATH for your personal scripts,
then issuing:

```
cd scripts/utils/vxl
ln -s $PWD/* ~/bin
```

### Switching between builds

For example, you are working in `vgl/algo` but would like to switch
to a bin folder in order to debug an executable.
```
pwd           # we are in vpe/vxl/core/vgl/algo
sw            # switches path between vxl source and vxl-bin
pwd           # we are in vpe/vxl-bin/core/vgl/algo
sw
pwd           # we are in vpe/vxl/core/vgl/algo
```

This requires a `.bash_profile` line described in `scripts/devsetup/tips`.

### Make anywhere with mymake

```
pwd           # we are in vpe/vxl/core/vgl/algo
mymake        # makes in vpe/vxl-bin
sw            # switches path between vxl source and vxl-bin
mymake        # works the same if you are already in vxl-bin
```

### CD Path

If you setup your CDPATH per `scripts/devsetup/tips`,
you can get to any folder from anywhere, for instance:
```
cd vgl/algo
cd vpgl
cd vxl
cd vxd
```

### Switching builds between Debug/Release

```
ls -ld vxl-bin vxd-bin

    vxd-bin -> vxd-bin-dbg
    vxl-bin -> vxl-bin-dbg
```
```
switchbuild rel
```

Will relink all build folders to release:

```
ls -ld vxl-bin vxd-bin

    vxd-bin -> vxd-bin-rel
    vxl-bin -> vxl-bin-rel
```

It is up to the programmer to configure CMAKE to be consistent with DBG or REL naming.
The programmer is by no means tied to these folder names, but the scripts assume
this convention.

You will want relink by hand if you have different builds with different compile
flags. The following command is equivalent to `switchbuild rel`

```
rm vxl-bin vxd-bin
ln -s vxl-bin-rel vxl-bin
ln -s vxd-bin-rel vxd-bin
```

I myself have something like this:
```
vxl-bin -> vxl-bin-clang-libstdcxx-c11-dbg-new
```

As long as you use the `vxl-bin/vxd-bin` symlinks, you are good to go with the
`sw` and `mymake` scripts.


### Code searching with tags

See `scripts/devsetup/tips`.


# Requirements

## VPE
### Basic usage (99% of the time)
  Edit pattern

  - heavy edits to VXD
  - small edits or tweaks to VXL

  Share pattern
  
  - VXD edits and pushes done in agreed upon feature branches (curve-cues),
    merged often into master
  - less often: VXL small edits shared on VXL feature branches on a peer remote
    which is not the official one

  New collab working with the team needs to know
  - remote and feature branch to work in VXD
  - corresponding remote and patch branch to work in VXL

### Fairly common
  - Integrate to VXD master done by more experienced/more active peer

### Very rare
  - Integrate to VXL master done by more experienced/more active peer
  
# Internal/ LEMS

### Basic usage (99% of the time)
  Edit pattern

  - heavy edits to Internal
  - moderate edits to VXD
  - small edits or tweaks to VXL

  Share pattern
  
  - Internal edits and pushes done in agreed upon feature branches (new-sfm-system),
    merged often into master
  - VXD edits and pushes done in agreed upon feature branches (edge-improvement)
  - less often: VXL small edits shared on VXL feature branches on a peer remote
    which is not the official one (github/rfabbri branch compile-fix)
  - when some good feature is pushed, a pull request/email is sent
    - this fails when: 

    - Say vxl and vxd got an update of a few commits. You pulled in the updates,
      but no other person in the team did. 
    - At this point, everyone has an old vxl/ and vxd/ 
    - You make an edit to Internal, commit, tested and push.
        - case 1: you did send a pull request (rarely rappens in rapid Internal dev)
        - case 2: you did not tell all your peers about this push. After all, it
          was only a few commits.
    - If someone else updates, they may get a compile or unforeseen errors,
      because your Internal commits were tested against a slightly older vxl.
    - They may see the compile error was due to VXL, and go in and update VXL.
      But VXD is slightly off, too..
    
    Everyone run these risks in day to day programming.
    - VXD will be moderately high developed. So it will be out of sync
      constantly, because people mostly use git pull from Internal.

  New collab working with the team needs to know

  To bootstrap
  - remote and feature branch to work in Internal
  - remote and feature branch to work in VXD
  - corresponding remote and patch branch to work in VXL
  - extras (for new person):
      - establishing src/bin layouts
      - what other libs and system packages to install
      - what project-specific setup and utilities to install (aliases, mymake, switchpath, etc)

  To keep up (assuming he stopped for a while and didn't receive all pull requests)
  - git fetch each Internal
  - figure out from peers what are the branches they're working with
  - git update the branch



### Fairly common
  - Integrate to Internal master done quite often (people tend integrate and push to
    master in my experience, when they take ownership of the repo)
  - Integrate to VXD master done by more experienced/more active peer

### Very rare
  - Integrate to VXL master done by more experienced/more active peer

# Subtree variant for VPE
This is an improvement to link [5]'s alternative in the response therein.

## Adding vxl to vpe
    > initial-dummy-file
    git add .
    git commit -m 'initial dummy commit'

    git remote add vxl ../vxl
    git fetch vxl

    # at this point, if you may have conflicting files,
    # checkout a branch and do the move as a separate commit,
    # prior to the merge.
    #
    # git checkout -b vxl-move-tmp vxl/master  # keeps vxl history across git clones
    # mkdir vxl
    # git mv `ls|grep -v vxl` vxl
    # git commit -m 'moved vxl to vxl/ folder'
    # git checkout master
    # an _then_ merge. This would work for adding other subtrees later on
    # lgit merge --allow-unrelated-histories vxl/master
    git merge --allow-unrelated-histories vxl-move-tmp

    # if you did move with a branch, now delete the branch
    # git branch -D vxl-move-tmp
    # --- SQUASHING HISTORY ------------------------------------------------
    # if you are importing a secondary subpackage, you might not want full history.
    # Then squash it, instead of the above merge:
    # To squash all commits since you branched away from master, do
    # git checkout vxl-move-tmp
    # git rebase -i master
    # Note that rebasing to the master does not work if you merged the master into your feature branch while you were working on the new feature. If you did this you will need to find the original branch point and call git rebase with a SHA1 revision.
    # 
    # Your editor will open with a file like
    # 
    # 
    # pick fda59df commit 1
    # pick x536897 commit 2
    # pick c01a668 commit 3
    # Each line represents a commit (in chronological order, the latest commit will be at the bottom).
    # 
    # To transform all these commits into a single one, change the file to this:
    # 
    # 
    # pick fda59df commit 1
    # squash x536897 commit 2
    # squash c01a668 commit 3
    # This means, you take the first commit, and squash the following onto it. If you remove a line, the corresponding commit is actually really lost. Don't bother changing the commit messages because they are ignored. After saving the squash settings, your editor will open once more to ask for a commit message for the squashed commit.
    # 
    # You can now merge your feature as a single commit into the master:
    # 
    # 
    # git checkout master
    # git merge squashed_feature
    # ----------------------------------------------------------------------

    git rm initial-dummy-file
    git commit -m 'cleaning up initial file'
    mkdir vxl
    git mv `ls|grep -v vxl` vxl
    git commit -m 'moved vxl to vxl/ folder'

    git checkout -b vxl-master vxl/master  # keeps vxl history across git clones
    # don't do the above command if you don't want to track this package's
    # history

    git remote add origin git@github.com:rfabbri/vpe.git
    git push -u origin master
    git push origin vxl-master
    git co master

## Adding vxd to vpe

    git remote add vxd ../vxd
    git fetch vxd
    git merge --allow-unrelated-histories vxd/master
    mkdir vxd
    git mv `ls|grep -v '^vxd$' | grep -v '^vxl$'` vxd
    git commit -m 'moved vxd to vxd/ folder'

    git checkout -b vxd-master vxd/master  # keeps vxl history across git clones
    git push origin master
    git push origin vxd-master
    git co master

    # check histories look perfect

## Finalizing
    - .gitignore

## Pulling in changes from VXL/VXD

    # do it in steps to make sure whats going on
    git fetch vxl
    # git branch vxl-master vxl/master # create optional branch if you're fetching 
                                       # non-master branch, else use vxl-master normally

    git checkout master
    # merges vxl/master into vxl-master
    git merge -s recursive vxl-master -Xsubtree=vxl vxl/master    # optional branch vxl/anybranch
    # git merge -s recursive utils-master -Xsubtree=scripts/utils utils/master   # for utils

    # Check that vxl-master has the commits as you want them
    # If necessary, checkout vxl-master and see if the files are in the right subfolder
    git merge vxl-master  # optional branch
    git push origin master
    git push origin vxl-master

    # you could also just merge directly! (be careful)
    # git fetch vxl
    # git merge -s recursive -Xsubtree=vxl vxl/master

## Pulling in changes from secondary packages (if any)

    # for now all our packages are like VXL/VXD, with full history merged
    # in. These instructions are here if you want to include some other package
    # without history merging (ie, you have squashed its commits and rebased)
    git fetch utils

    git checkout -b utils-rebase utils/master

    # TO TEST: merge with squash (so we don't keep their history)
    git merge -s recursive --squash utils-rebase -Xsubtree=vxl vxl/master    # optional branch vxl/anybranch

## Updating the remote
### 1. Make edits
    # Edit vxl/ normally

    # eg:  echo '// test' >> vxl/CMakeLists.txt   # an existing VXL file is edited
    # if we want that change to be backported to VXL, we prepend TO VXL:
    git ci -am "VPE->VXL: cmakelists"  # this message shows up on upstream
    # Or, if we forgot, we can tag the commit "TO-VXD"

Keep doing other commits to anywhere in the tree.  When backporting, we have to
cherry-pick when the original team has made free commits anywhere in the tree.
If you yourself are working on the tree, and separate your commits to vxl/ and
vxd/ folders in separate branches merged to your master, this becomes a rebase
instead of cherrypicking (see similar approach 2 below).

### 2. Cherrypick/Rebase edits
    git fetch vxl
    git checkout -b vxl-integration vxl-master
    git checkout vxl-integration
    # merge changes from master using subtree
    git cherry-pick -x --strategy=subtree -Xsubtree=vxl/ master
    # check if that generates a commit with the wrong prefix, if so,
    # undo the commit by resetting HEAD and give up, start again.
    #
    # --strategy=subtree (-s means something else in cherry-pick) also helps to make sure
    # changes outside of the subtree (elsewhere in container code) will get quietly
    # ignored. 

    # use '-e' flag to cherry-pick to edit the commit message before passing upstream
    # you may want to say something about it if it was a move from another package
    #
    # the -x in these commands annotate the commit message with the SHA1 of VPE
    # Use the following to make sure files outside of the subtree (elsewhere in container code) 
    # will get quietly ignored. This may be useful when cherypicking a rename, since move is rm+add

    # Or, if you organized your VXL commits directly into eg vxl-integration, just rebase

    git branch master-reb master # master or any other branch tip to rebase
    git rebase -s subtree -Xsubtree=vxl --onto vxl-integration feature-in-progress master-reb
    git checkout vxl-integration
    git merge master-reb

## 3. Push edits

    # double-check your future vxl-master commits will look good and linear

    git co vxl-master
    git merge vxl-integration
    git push origin vxl-master # to push the vxl-master branch to toplevel VPE
    git push vxl HEAD:master
    git branch -D master-reb

    # tag master that you've done all integration up to here
    git co master
    git tag -d integrated-VXL 
    git push origin :refs/tags/integrated-VXL
    git tag -a integrated-VXL -m "Integrated all commits touching VXD up to this point."

    Rebase is nice, since rebasing interactively means that you have a chance to
    edit the commits which are rebased (inserting move-related info such as the
    origin sha1 etc). You can reorder the commits, and you can
    remove them (weeding out bad or otherwise unwanted patches).

### Creating a new Internal project based on VPE 

    # clone VPE from github
    git clone vpe internalvpe
    # do all your edits in InternalVPE
    # change the name and the remote to your new internal repo URL

### Migrating a new Internal project to a workflow based on VPE
    # lets call this new project LEMSVXL

    # lemsvxl is a checked-out copy of Internal project
    # we first create a VPE monorepo for that project
    cp -R lemsvxl lemsvpe
    cd lemsvpe

    # new-master is the branch we work on LEMSVXL stuff
    # it is the modernized version
    git checkout new-master

    # move everything to lemsvxl subdir, if you want
    mkdir lemsvxl
    git mv `ls |grep -v '^lemsvxl$'` lemsvxl

    # do a ls -a to check for any hidden files


    # check that VPE itself doesn't have conflicting file names with the
    # existing project, as we merge it right in the root folder:
    # if it does have conflicting files, checkout a copy of VPE,
    # checkout a new branch, then remove the undesired files prior to merging
    # it to your repo.

    git remote add vpe ../vpe
    git fetch vpe
    # double-check no files match! Usually .gitignore is conflicted, or README
    git merge --allow-unrelated-histories vpe/master

    # Now replace the origin remote by a new one for LEMSVPE
    git remote add origin git@bitbucket.org:rfabbri2/lemsvpe.git

    # massive push
    git push -u origin --all # pushes up the repo and its refs for the first time
    git push origin --tags # pushes up any tags

    # substitute remotes initially done locally to a proper remote

    git remote remove vpe

    # run all this in a bootstrap/setup script
    git remote add vpe http://github.com/rfabbri/vpe
    git fetch vpe
    git branch --track vpe-master vpe/master

    # run VPE's bootstrap first
    git remote add vxl git@visionserver.lems.brown.edu:kimia_group/lemsvxl.git
    git remote add vxd git@github.com:rfabbri/vxd.git
    git fetch vxl
    git fetch vxd
    git branch --track vxl-master vxl/master
    git branch --track vxd-master vxd/master

    # check user's git is greater than 2.

### First-timer cloning and seting up LEMSVPE
    git clone git@bitbucket.org:rfabbri2/lemsvpe.git lemsvpe
    git checkout -b new-master origin/new-master
    git remote add vpe http://github.com/rfabbri/vpe
    git fetch vpe
    git branch --track vpe-master vpe/master
    vpe-bootstrap
    # check user's git is greater than 2.

### Pulling in changes from VPE from within Internal project
    # VPE was merged directly into the project root. That's where it lives.
    # So we use regular merges. If there are conflicts (eg, for the README), we
    # should always favour our version.

    git fetch vpe
    git merge -s ours vpe/master  # this didn't work, one commit had conflict,
                                  # the rest got ignored. I just carefully
                                  # cherry-picked the VPE changes I needed
### Conclusion
- same as before

### Caveates
- `git log --follow` doesn't work for sub folders to trace down to the orig repo

- mere users only push to main repo, but will never ever push directly to the
  subrepos, unless they are advanced users. this may be a good thing, as
  day-to-day workflow gets centralized and synced a lot easier (one repo) than
  before.


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

