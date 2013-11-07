Elemate (current Development Name) {#mainpage}
=======

German install instructions:

##1. OpenSceneGraph
Download hier: http://www.openscenegraph.org/index.php/download-section/stable-releases/146-openscenegraph-3-2-release-downloads

OSG compiling - cmake optionen
OSG_USE_QT -> no
WIN32_USE_MP -> yes ( nur Windows -> multithreaded compilieren)
CMAKE_INSTALL_PREFIX -> auf Pfad setzen, wo die kompilierten libs reinkopiert werden

###Kompilieren unter Visual Studio
    OSG unter Debug und Release kompilieren
    -> damit OSG in unserem Projekt in Debug+Release genutzt werden kann
    Dann jeweils INSTALL (unter CMakePredefinedTargets) bauen -> kopiert die libraries ins install Verzeichnis
    

* vermutlich nur unter Windows nötig: Umgebungsvariable OSG_ROOT auf das Osg-Installverzeichnis setzen
* OSG_FILE_PATH -> dort sucht OSG standardmäßig nach Dateien -> wohl keine schöne Lösung als Suchpfad für unser Projekt

http://openscenegraph.sourceforge.net/documentation/OpenSceneGraph/doc/examples.html

Aktuelle Lösung für Visual Studio Debug:
    in Projektoptionen->Linker->Eingabe->Zusätzliche Abhängigkeiten
    -> alle OpenSceneGraph-dlls in *d.dll umbenennen, damit debug-dlls genutzt werden


##2. NVIDIA PhysX Einrichtung (Win64)

1. Runterladen und Installieren des SDK (V.a. PhysX, APEX kommt später) min. Version 3.2.4 von http://www.nvidia.com/object/physx_downloads.html (Requires Registration first)
2.   a. Umgebungsvariable PHYSX_ROOT auf Installationsverzeichnis setzen ODER
b. Hoffen, dass bei CMAKE das Standardverzeichnis gefunden wird
c. Beim CMAKE-Vorgang die Verzeichnisse für Lib und Include manuell setzen
3.   a. Verzeichnis $(PHYSX_ROOT)\Bin\Win64 zum PATH hinzufügen oder
b. DLLs später manuell zum Projekt hinzufügen (PhysX3_64.dll, PhysX3Common_64.dll und PhysX3Extensions.dll)
4. CMAKE Generate und ausführen.

##3. Doxygen
1. Runterladen der 32 oder 64 Bit Win-Binaries oder der Sourcen zum Selbstcompilen unter dieser Adresse: http://www.stack.nl/~dimitri/doxygen/download.html
2.   Linux: Kompilieren (Anleitung ebenfalls unter http://www.stack.nl/~dimitri/doxygen/manual/install.html#install_bin_unix)
Windows: Installieren und Wizard folgen.
3. Im Projekthauptverzeichnis `doxygen Doxygen.cfg` ausführen um eine (fantastische!) Doku in das "documentation"-Verzeichnis zu generieren (doucmentation ist im .ignore-File, also ggf. selbst erstellen)
4. Profit.
