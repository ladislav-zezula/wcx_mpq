WCX_MPQ
=======

### MPQ Plugin for Total Commander
This plugin can be installed into Total Commander. It allows to view and modify MPQ archives as if they were a ZIP file.

### Build Requirements
To build the MPQ plugin, you need to have one of these build environments
* Visual Studio 202x
* Visual Studio 2008
* WDK 6001
Also, the following tool is needed to be in PATH:
* zip.exe (TODO: Get the ZIP download URL)

1) Make a new directory, e.g. C:\Projects
```
md C:\Projects
cd C:\Projects
```

2) Clone the common library
```
git clone https://github.com/ladislav-zezula/Aaa.git
```

3) Clone and build the StormLib library
```
git clone https://github.com/ladislav-zezula/StormLib.git
cd StormLib
make-msvc.bat
cd ..
```

4) Clone and build the WCX_MPQ plugin
```
git clone https://github.com/ladislav-zezula/wcx_mpq.git
cd wcx_mpq
make-msvc.bat /web
```
The installation package will be in the current directory.

5) Locate the wcx_mpq.zip file and step into it in Total Commander (Ctrl+PgDown).
Total Commander will ask whether you want to install the plugin. After the plugin
is installed, you may configure the plugin (especially listfile folder).
