# Installation Steps Linux
	
To install the sumo plugin, user requires the *traici library* which will be linked to the plugin.
Traici can be installed into the system using the Makefile availiable in /plugins/sumo folder.
The info on Makefile is explained in the below steps :

 * Makefile in sumo plugin consists of the following targets
	* **all**   : depends on lib
	* **lib**   : builds the traici library assuming all the dependences are installed
	* **deps**  : installs all the required dependencies to build traici
	* **clean** : cleans up all the transiional files
	

 * `make deps` and `make lib` are being called from the ./configure file of the cloe repository. `make deps` will build the dependencies and `make lib` will call the conan command to build the traici library and store it in the local cache

 * Finally the conan install command in the ./configure file will install the traici lib , which the sumo plugin can refer to link
 
 **NOTE** : 
 **The Sumo version should be built from the repository from where the traici was built**
 **Mismatch in the version of traici and sumo will result in incompatibility issues**
 **Copy the built sumo to "/usr/lib/"** 
 