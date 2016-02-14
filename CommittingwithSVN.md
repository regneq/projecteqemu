There are many ways to submit code, but I am going to use TortoiseSVN (Windows) to guide you through.

1. Download and install [TortoiseSVN](http://tortoisesvn.net/downloads) ([Mirror](http://www.visualsvn.com/visualsvn/download/tortoisesvn/)). Reboot if necessary.

2. Open up a command prompt (Start - Run - Type "cmd")

3. At the command prompt, CD to the directory you want as your working directory.

4. At the prompt type: "svn checkout https://projecteqemu.googlecode.com/svn/trunk/ projecteqemu --username yourname"

5. Hit "p" to accept Google's cert permanently. It will ask you for your password. Go to this page: http://code.google.com/p/projecteqemu/source/checkout and click "googlecode.com password." (Make sure you are logged in first) This will give you your password, just copy and paste it at the prompt.

6. You'll see the files come down, and if all goes right, it will end with Revision <#> You are now setup with a directory on your hard drive that has write access to our repo.

7. Here's why we chose TortoiseSVN. Make a change to a file in the repo. Let's say you made a change to _zone/attack.cpp_. Once you save the file, the file's icon will change from a green check mark, to a red exclamation point. This indicates that the file on your drive is different than the one in our repo.



### Additional Useful Tips: ###
  * To commit that change in our repo, go back to the root directory of the source (currently, EQEmuServer) and right click anywhere in the explorer window. Select SVN Commit. Make sure the files you want committed are selected, and please fill in a message, even if it's just See Changelog (Make sure you update that as well!) Click OK, and you're set!

  * Before committing, it's a good idea to update your source, in case another developer made a change before you. To do this, simply right click in the root directory and click SVN Update. That'll pull down any files changed by another author (it will NOT revert files you have changed but not yet submitted - if you changed a file that was also updated, SVN will attempt to merge them for you.)

  * To add a new file to the repo, create the file in your working folder. Right click it, and click TortoiseSVN - Add. You can select multiple files and do this at once. Make sure all the added files are selected, and click OK. Then, do a SVN Commit to make the change on our repo.

  * Removing a file is the same deal, right click the file and select TortoiseSVN - Delete. Again, you must do a SVN Commit for it to take effect in our repo.

  * To create a diff, change whatever files you are working with, but do not commit yet. Right click somewhere in the root directory of your source, and click TortoiseSVN - Create Patch... Make sure the files you changed are selected, and then click OK. Name and save the .patch file somewhere and SVN will create the patch for you. You can post that on the EQEmu forums, give it to other developers, etc.