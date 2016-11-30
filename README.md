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

```bash
doc/                        # Other possible workflows for VPE, maintenance, etc

scripts/devsetup/           # Scripts used by ./setup-for-development

scripts/utils/              # Utilities for VXL development, templates, shell/editor config

vxl/                        # VXL folder tracked within VPE
vxl-bin -> vxl-bin-dbg      # Default VXL build folder pointing to possible debug version
vxl-bin-dbg/                # VXL build folder with debug flags
vxl-bin-rel/                # VXL build folder without debug flags.
vxl-orig/                   # Original VXL as a separate repository (mostly for history)

vxd/                        # VXD folder tracked within VPE
vxd-bin -> vxd-bin-dbg      # Default VXL build folder pointing to possible debug version
vxd-bin-dbg/                # VXD build folder with debug flags
vxd-bin-rel/                # VXD build folder without debug flags.
vxl-orig/                   # Original VXD as a separate repository (mostly for history)
```

You will also have remotes and branches setup for you

```bash
git remote
```

```
    origin	https://github.com/rfabbri/vpe.git
    vxl	https://github.com/vxl/vxl.git
    vxd	https://github.com/rfabbri/vxd.git
    utils	git://git.code.sf.net/p/labmacambira/utils
```

```bash
git branch
```

```
  * master
    utils-master
    vxd-master
    vxl-master
```

Each of these branches point to the last commit from the remote that VPE merged
into master. For instance, to see what vxl master actually has, `git fetch` and
then inspect the `vxl/master` branch instead of `vxl-master`.

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
```bash
pwd           # we are in vpe/vxl/core/vgl/algo
sw            # switches path between vxl source and vxl-bin
pwd           # we are in vpe/vxl-bin/core/vgl/algo
sw
pwd           # we are in vpe/vxl/core/vgl/algo
```

This requires a `.bash_profile` line described in `scripts/devsetup/tips`.

### Make anywhere with mymake

```bash
pwd           # we are in vpe/vxl/core/vgl/algo
mymake        # makes in vpe/vxl-bin
sw            # switches path between vxl source and vxl-bin
mymake        # works the same if you are already in vxl-bin
```

### CD Path

If you setup your CDPATH per `scripts/devsetup/tips`,
you can get to any folder from anywhere, for instance:
```bash
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

```bash
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

### Inspecting history

- Currently, `git log --follow` doesn't work for sub folders to trace down to
  the original repository. This means that, to see all commits from VXL that are
  outside of VPE, you will need to do something like this:

```bash
cd vxl-orig
gitk --all
```

### Code searching with tags

See `scripts/devsetup/tips`.

# Use a recent Git
Always use Git version >= 2 when working with monorepos.


## Workflow Usage Patterns

### VPE
#### Basic usage (99% of the time)
Edit pattern

  - heavy edits to VXD
  - small edits or tweaks to VXL

Share pattern
  
  - VXD edits and pushes done in agreed upon feature branches (eg, `curve-cues`),
    merged often into `master`
  - VXL small edits shared on VXL branches inside VPE 

New collab working with the team needs to know

  - Most of the time: nothing. The master branch already has proper VXD and VXL merged in.
  - If new collab wants to work on a feature, he needs to know the feature
    branch to work on VPE

#### About once a week
  - Integrate to VXD upstream done by more experienced/more active peer

#### About once a month
  - Pull changes from VXL and integrate into VPE

#### Very rare
  - Integrate to VXL upstream done by more experienced/more active peer
  
## Subtree maintenance for VPE
This is an improvement to the alternative process proposed in a [stackoverflow answer](http://stackoverflow.com/questions/10918244/git-subtree-without-squash-view-log/40349121#40349121).

For the curious, the procedure we used to create the VPE monorepo initially is
in the file [additional-maintenance](./doc/additional-maintenance.md). 

### Pulling in changes from VXL/VXD
```bash
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
# Update VPE upstream
git push origin master
git push origin vxl-master

# you could also just merge directly! (be careful)
# git fetch vxl
# git merge -s recursive -Xsubtree=vxl vxl/master
```

### Pushing to VXL/VXD upstream from VPE
#### 1. Make edits to VXL/VXD in an organized way
```bash
# Edit vxl/ normally (best to organize your commits in a separate branch)
#
# eg:  
echo '// test' >> vxl/CMakeLists.txt   # an existing VXL file is edited
#
# if we want that change to be backported to VXL, we prepend TO VXL to the
# commit and (preferably) keep it in a separate branch:

git ci -am "VPE->VXL: cmakelists"  # this message shows up on upstream

# Or, if we forgot, we can tag the commit "TO-VXD"
```
Keep doing other commits anywhere in the tree.  When backporting, we have to
cherry-pick when the original team has made free commits anywhere in the tree.
If you yourself are working on the tree, and separate your commits to vxl/ and
vxd/ folders in separate branches merged to your master, this becomes a rebase
instead of cherrypicking.

#### 2. Cherrypick/Rebase edits
```bash
git fetch vxl
git checkout -b vxl-integration vxl-master
git checkout vxl-integration
# merge changes from master using subtree (_adapt_ to pick the commits you want)

git cherry-pick -x --strategy=subtree -Xsubtree=vxl/ master

# check from git log or gitk if that generates a commit with the wrong prefix,
# if so, undo the commit by resetting HEAD and give up, start again.
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

# Or, if you organized your VXL commits directly into eg vxl-integration, just rebase:

git branch master-reb master # master or any other branch tip to rebase
git rebase -s subtree -Xsubtree=vxl --onto vxl-integration feature-in-progress master-reb
git checkout vxl-integration
git merge master-reb
```

#### 3. Push edits to VXL/VXD

```bash
# double-check your future vxl-master commits will look good and linear

git co vxl-master
git merge vxl-integration
git push origin vxl-master # to push the vxl-master branch to toplevel VPE
git push vxl HEAD:master
git branch -D master-reb

# you may want to tag master that you've done all integration up to here
git co master
git tag -d integrated-VXL 
git push origin :refs/tags/integrated-VXL
git tag -a integrated-VXL -m "Integrated all commits touching VXD up to this point."
```

Rebase is nice, since rebasing interactively means that you have a chance to
edit the commits which are rebased (inserting move-related info such as the
origin sha1 etc). You can reorder the commits, and you can
remove them (weeding out bad or otherwise unwanted patches).

## Internal projects based on VPE 

For creating a new internal project based on VPE (possibly private),
or for migrating an existing VXL/VXD internal project to a workflow based on
VPE, see [Internal Projects and VPE](./doc/internal-projects.md).

