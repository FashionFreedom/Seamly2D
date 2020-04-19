# Contributing code

We highly recommend that you read this page before you clone the repo and start making changes. 

Our preferred workflow is based on **Git Flow**. If you use a different workflow approach keep in mind this might create extra work when creating pull requests.

More about **Git Flow**:  http://nvie.com/posts/a-successful-git-branching-model/   
More about **Pull Requests**: https://help.github.com/articles/about-pull-requests/ and https://help.github.com/articles/creating-a-pull-request/    

## Permanent Named branches 

The permanent named branches on this repo are: 
* **develop** - Contains next major release. Used for testing and sharing among developers. 
* **master** - Used only for releases.
* **release-x.x.x** - Created from **develop**. Contains feature freeze state before the next major release. Used for preparing each major & minor release with updated build information. Merge to **master** with new tag to create a new release.
* **hotfix-x.x.x** - Create from **develop** and **master**. Contains a quick emergency fix for big bugs. Merge to **master** (or **release**) and **develop**.

## Git Flow summary 
### Git Flow 1 - Clone the repo

* Clone our repo to your local PC.
>git clone https://github.com/fashionfreedom/seamly2d
* Set the origin URL.
>git remote set-url origin https://github.com/fashionfreedom/seamly2d
* Create your issue branch, name it after the issue you will fix: **issue-#_n_** (eg - **issue-#155** to name your fix for issue #155.) 

### Git Flow 2 - Create your issue branch from develop branch then switch to your issue branch
>git checkout develop       
git checkout -b issue-#_n_
git checkout issue-#_n_

### GitFlow 3 - Daily workflow

* Get latest changes from the **develop** branch from our main repo before
>git pull develop issue-#_n_
git checkout issue-#_n_ 
* Make your code changes 
* Commit your changes to issue-#_n_
git commit -a
git commit -m "_your commit message_"
git commit push issue-#_n_   

### GitFlow 4 - Test your issue branch (DON'T SKIP THIS STEP)

* Periodically build your issue branch. Use QT or VisualStudio for Windows and Mac, or create an AppImage for Linux.
* Run it on your local PC. Try to find how to make your changes crash.

### GitFLow 5 -Push your issue branch up to the main repo

* When you confirm that the issue was fixed (at least for you! :D), ask our repo admins to give you write permissions.
* Push your branch up to our main repo. 
>git push -u origin issue-#_n_

### GitFLow 6 - Create a Pull Request

* On the Code page in the main repo, next to **Branch:develop** in upper left above file listing, select the **New Pull Request** button. If **Branch:master** is displayed use the dropdown list to select the **develop** branch.
* The Pull Request page will appear. From the **Compare:** dropdown list, select your issue branch.
* In the comments box, add the text "Fix issue #_n_" (eg "Fix issue #155") 
* On the next line add the text "Tested on _operatingsystem_ _version_ _buswidth_". (eg "Tested on Windows 10 64-bit")
* Click the **Create Pull Request** button.
* Link your Pull Request to the fixed issue with the **Linked Issues** option in the right hand column.
* Other team members will test it on their systems and approve the Pull Request.
* The maintainer will merge your issue branch into **develop**.
* After merge, add your changes to file **ChangeLog.txt**

### GitFlow 7 - Update your Pull Request 
Most likely we will ask you to fix some issues in your code. In this case you would: 
* Make changes to your local **issue-#_n_** branch
* Push your **issue-#_n_** branch up to the repo
* Update your existing pull request 
* Rejoice again! :D

Thanks for being an open source contributor!
