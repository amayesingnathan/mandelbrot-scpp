import sys
import os
import platform
import subprocess
import shutil
from unicodedata import name
import requests
from pathlib import Path

def ReplaceNamespace(namespace):
    with open('premake5.lua', 'r') as premakeRoot:
        filedata = premakeRoot.read()
    filedata = filedata.replace('scpp-template', namespace)
    with open('premake5.lua', 'w') as premakeRoot:
        premakeRoot.write(filedata)

def ReplaceProjectName(projectName):
    with open('premake5.lua', 'r') as premakeRoot:
        filedata = premakeRoot.read()
    filedata = filedata.replace('TemplateProject', projectName)
    with open('premake5.lua', 'w') as premakeRoot:
        premakeRoot.write(filedata)
        
    with open('{name}/premake5.lua'.format(name=projectName), 'r') as premakeProj:
        filedata = premakeProj.read()
    filedata = filedata.replace('TemplateProject', projectName)
    with open('{name}/premake5.lua'.format(name=projectName), 'w') as premakeProj:
        premakeProj.write(filedata)
    
class ProjectConfiguration:

    @classmethod
    def CheckProjectConfig(cls):
        return not os.path.isdir('TemplateProject')

    @classmethod
    def SetupNamespace(cls, namespace):
        ReplaceNamespace(namespace)
        
    @classmethod
    def SetupProject(cls, projectName):        
        os.rename('TemplateProject', projectName)
        ReplaceProjectName(projectName)
        subprocess.call(['git', 'submodule', 'add', 'https://github.com/amayesingnathan/streamline-cpp.git', '{name}/dependencies/streamline-cpp'.format(name=projectName)])
        subprocess.call(['git', 'submodule', 'update', '--init', '--recursive'])