# How VPE was built using subtrees
These are the commands used to build the monorepo.
Use them to build another one, or to learn how VPE works in detail.

### Adding VXL to VPE
```bash
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
```

## Adding vxd to vpe
```bash
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
```

## Finalizing
    - .gitignore

## Pulling in changes from secondary packages (if any)

```bash
    # for now all our packages are like VXL/VXD, with full history merged
    # in. These instructions are here if you want to include some other package
    # without history merging (ie, you have squashed its commits and rebased)
    git fetch utils

    git checkout -b utils-rebase utils/master

    # TO TEST: merge with squash (so we don't keep their history)
    git merge -s recursive --squash utils-rebase -Xsubtree=vxl vxl/master    # optional branch vxl/anybranch
```

