# ReplayManipulator
**ReplayManipulator is a bakkesmod plugin that manages and manipulates "everything" with a replay**

## Features
- Modify the loadout of any car in a replay
- Apply custom decals to cars (using the AlphaConsole folder and format for this)
- Add custom paint colors to items 
- Rename any player in a replay
- Change player camera settings
- Fixes issues with camera transitions when using slowmotion
- View and open all your saved replays from the settings menu
- Change maps (Thumbnail feature had to be dropped in the rewrite)
- Change stadium colors
- Hide the ball

## Building 
- This project uses vcpkg for it's 3rd-part dependencies. You will have to install this and integrate this with visual studio. [See here for a quick start on vcpkg](https://github.com/microsoft/vcpkg#quick-start-windows)

This should be the required commands for installing vcpkg, but may be out of date at the time of you reading this
```
> git clone https://github.com/microsoft/vcpkg
> .\vcpkg\bootstrap-vcpkg.bat
> .\vcpkg\vcpkg integrate install
```

Assuming you've installed vcpkg and bakkemod on your computer, visual studio should be able to build the project with no additional setup required.



## Contributing
Please follow the same codestyle as the rest of the project (there's a clang-format file in the solution to simplify this)


1. Create a issue for the bug\feature you intend to fix or add
2. Start a new feature branch based on the develop branch
3. Start a pullrequest when you're ready to merge your changes.
4. Wait for your pullrequest to be accepted


### Support
If you want to support future development and maintenance of this plugin. You can sponsor me on github or donate to my PayPal. Links for both these options should be visible on this repository
