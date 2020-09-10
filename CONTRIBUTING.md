# Contributing code

We highly recommend that you read this page before you clone the repo and start making changes. 

Our preferred workflow is based on **Git Flow**. If you use a different workflow approach keep in mind this might create extra work when creating pull requests.

Fix one issue at a time. This allows other team members to quickly test & merge your code.

More about **Git Flow**:  http://nvie.com/posts/a-successful-git-branching-model/   
More about **Pull Requests**: https://help.github.com/articles/about-pull-requests/ and https://help.github.com/articles/creating-a-pull-request/    

## Permanent Named branches 

The permanent named branches on this repo are: 
* **develop** - Contains next major release. Used for testing and sharing among developers. 
* **master** - Used only for releases.
* **release-x.x.x** - Created from **develop**. Contains feature freeze state before the next major release. Used for preparing each major & minor release with updated build information. Merge to **master** with new tag to create a new release.
* **hotfix-x.x.x** - Create from **develop** and **master**. Contains a quick emergency fix for big bugs. Merge to **master** (or **release**) and **develop**.

## Git Flow summary 
_These steps use Git command line_ 
_It's easier if you use Github Desktop https://desktop.github.com/ for Windows & Mac_

### Git Flow 1 - Clone the repo

* Clone our repo to your local PC.
>git clone https://github.com/fashionfreedom/seamly2d  
* Set the origin URL.
>git remote set-url origin https://github.com/fashionfreedom/seamly2d  

### Git Flow 2 - Create your issue branch from develop branch then switch to your issue branch 

* Create your issue branch, name it after the issue you will fix: **issue-#_n_** (eg - **issue-#155** to name your fix for issue #155.) 
>git checkout develop       
git checkout -b issue-#_n_   
git checkout issue-#_n_  

### GitFlow 3 - Daily workflow

* Get latest changes from the **develop** branch from our main repo before
>git pull develop issue-#_n_  
git checkout issue-#_n_   
* Make your code changes 
* Commit your changes to issue-#_n_    
>git commit -a  
git commit -m "_your commit message_"  
git commit push issue-#_n_    
_Note: At the next build after merging your code into the develop branch, your commit message will be automatically added to the Changelog.md file_

### GitFlow 4 - Test your issue branch (DON'T SKIP THIS STEP)
 
* Periodically build your issue branch. You'll need Qt 5.13.2 and Qt Creator 4.3.x with MSVC 2017 64bit or MinGW 7.3.0 64bit in your Qt project Kit.
* Run it on your local PC. Try to find how to make your changes crash before making your pull request.

### GitFLow 5 - Push your issue branch up to the main repo

* Push your branch up to our main repo 
>git push -u origin issue-#_n_ 
* Go to **Pull Requests** and click on the green **New Pull Request** button

### GitFLow 6 - Create a Pull Request

* On the Code page in the main repo, make sure **Branch:develop** is selected.   
* Click on **Pull Requests** in the main menu bar. Click on the green **New Pull Request** button.
* From the **Compare:develop** dropdown list, select your issue branch.  
* In the comments box add a short description, eg "Fix issue #155" or other short description of what was done.  
* On the next line optionally add additional relevant info. (eg "Tested on Windows 10 64-bit"). 
* Click the **Create Pull Request** button
* Link your Pull Request to the issue with **Linked Issues** in the right hand column.  
* Other team members will test it on their systems and approve the Pull Request.  
* The maintainer will merge your issue branch into **develop**.
* At the next build, the first line of your short description will be added to ChangeLog.md

### GitFlow 7 - Update your Pull Request 

Most likely we will ask you to fix some issues in your code. In this case you would: 
* Make changes locally to your **issue-#_n_** branch  
* Push your **issue-#_n_** branch up to the repo again  
* Update your existing pull request   
* Rejoice ! :D  

Thanks for being an open source contributor!  
