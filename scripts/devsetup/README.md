# Scripts for setting up a Vision Programming Environment project

## Usage
For instructions on running this, you should read the projects README.md, which
should have more specific instructions. If not, you should try running
`scripts/bootstrap`, then `scripts/setup` after a fresh git clone.

### scripts/bootstrap

[`scripts/bootstrap`][bootstrap] is used solely for fulfilling dependencies of the project.

For VPE this installs RPM/macports/homebrew packages and checks Git is the right version.

### scripts/setup

[`scripts/setup`][setup] is used to set up a project in an initial state.
This is typically run after an initial clone.

This will add useful branches and remotes to your projects.
After this, you can run CMake inside every `*-bin` folder, starting with `vxl/`,
moving up to `vxd/` and then any other specific project folders.

### scripts/update

[`scripts/update`][update] is used to update the project after a fresh pull.

Just a helper that fetches all remotes for you. It won't merge anything; you are
responsible for inspecting and merging, or requesting the package maintainer to
merge things for you, should you find any new interesting features.

### scripts/test

[`scripts/test`][test] is used to run the test suite of the application.

Goes inside each `*-bin` folder and types make test, or do something more
project-specific in that sense (by default it could test only current modules of
interest, and --all could test everything).

[bootstrap]: scripts/bootstrap
[setup]: scripts/setup
[update]: scripts/update
[test]: scripts/test

# Credits

This is very loosely insipred in "Scripts To Rule Them All",
a set of boilerplate scripts describing the [normalized script pattern
that GitHub uses in its projects](http://githubengineering.com/scripts-to-rule-them-all/). 
It also has some inspiration from Kitware's GitSetup, a copy of which you can
find under `vxl/scripts/`. These scripts, however, don't have any config file,
but instead you have to customize one by one for a new project, which we found
to be simple enough and more powerful.
