
### Creating a new Internal project based on VPE 

```bash
# clone VPE from github
git clone vpe internalvpe
# do all your edits in InternalVPE
# change the name and the remote to your new internal repo URL
```

### Migrating a new Internal project to a workflow based on VPE
```bash
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
```

### First-timer cloning and seting up LEMSVPE
```bash
git clone git@bitbucket.org:rfabbri2/lemsvpe.git lemsvpe
git checkout -b new-master origin/new-master
git remote add vpe http://github.com/rfabbri/vpe
git fetch vpe
git branch --track vpe-master vpe/master
vpe-bootstrap
# check user's git is greater than 2.
```

### Pulling in changes from VPE from within Internal project
```bash
# VPE was merged directly into the project root. That's where it lives.
# So we use regular merges. If there are conflicts (eg, for the README), we
# should always favour our version.

git fetch vpe
git merge -s ours vpe/master  # this didn't work, one commit had conflict,
                              # the rest got ignored. I just carefully
                              # cherry-picked the VPE changes I needed
```

