 app
--------
Here are some notes on this app's image.


Building Docker image
================
Get paraview-Install.zip from developers and unzip into this diretory
```
docker build -t tagname  .
```

Other files of interest used in image
=========================
Paraview is an edited version of Paraview-Install/paraview.sh which uses a system-installed qt version instead of the libs they provided
`qt_install_utils/qt-installer.qs` is an input script to online qt installer (see Dockerfile for more details on how its used) 
`qt_install_utils/extract-qt-installer` can be used to see a list of modules which might be helpful when editing qt-installer.qs (`./extract-qt-installer --list qt-opensource-linux-x64-5.12.4.run`)
