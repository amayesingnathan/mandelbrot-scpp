# scpp-template

Desktop Application C++ project template using premake5 and [streamline-cpp](https://github.com/amayesingnathan/streamline-cpp).

## Description

This is a template for a desktop application C++ project, using the streamline-cpp framework. The template comes with a basic premake template project, 
already setup to use streamline-cpp, and when the set up script is run premake will be downloaded and project files will be generated.

The first time that the setup script is run, the user is prompted to supply the name for the workspace and starting template project. 
The repo directory will still need to be renamed manually as sometimes the permissions for this can be restricted.

### Dependencies

* Python3
* premake5 - Will be downloaded and extracted as a binary when the setup script is run if it is not found.
* [streamline-cpp](https://github.com/amayesingnathan/streamline-cpp)
* Currently only supports Windows and Linux (Ubuntu tested).

### Installing

* Clone the repo and run `setup.bat` for Windows or `setup.sh` for Linux. 
* Enter the name for the solution and the starting project.
* Project files will be generated for the target platform to use.
* Extend the project as desired!

## License

This project is licensed under the MIT License - see the LICENSE.md file for details