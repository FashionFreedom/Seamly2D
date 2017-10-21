# Contributing code

We highly recommend that you read this page before you clone the repo and start making changes. 

Our workflow is based on "Git Flow".    
Read more about git flow branch development here: http://nvie.com/posts/a-successful-git-branching-model/  
Read the Github tutorials about making Pull Requests https://help.github.com/articles/about-pull-requests/ and https://help.github.com/articles/creating-a-pull-request/  

## Named branches

We have a naming scheme for branches, based on "Git Flow":

* **master** - Used only for releases.
* **develop** - Contains next major release. Used for testing and sharing among developers. 
* **release-x.x.x** - Create from **develop**. Contains feature freeze state before the next major release. Used for preparing each major & minor release with updated build information. Merge to **master** with new tag to create a new release.
* **feature-_issue#_** - Create from **develop**. Contains code for new or improved features. Merge to **develop**.
* **hotfix-x.x.x** - Create from **develop** and **master**. Contains a quick fix. Merge to **master** (or **release**) and **develop**.


### Create your feature branch

Get the latest code & create your **feature-_issue#_** branch.  Include the issue number that you're working on.

> git pull  
git checkout develop                 
git checkout -b feature-_issue#_   
git checkout feature-_issue#_      
#make your code changes then commit...
git commit -a
git commit -m "_your commit message_"
git commit push feature-_issue#_   

If you don't have rights to create a branch, contact the maintainer - they will give you rights.


### Follow the daily code routine - important!

Check daily for **develop** updates & merge into **feature-_issue#_**

>git checkout develop
git pull
git checkout feature-_issue#_
git merge develop
#make your code changes then commit...
git commit -a
git commit -m "_your commit message_"
git push origin  feature-_issue#_    #save your changes to repo!

Periodically test your code by building in QtCreator and running the app.
Checkout your feature branch before building in Qt! 
No need to build the develop branch :)


### When you're ready to submit your feature branch

Build your code in QtCreator one last time and test by running the app, trying to break your new or improved feature.
If all is good...
Merge your **feature-_issue#_** to your **develop** & resolve merge issues.
Go back to **feature-_issue#_** & push  to the github repo.

>git checkout develop
git merge feature-_issue_#
git checkout feature-_issue#_
git push origin feature-_issue#_

Login to the github repo and make a pull request with description "**Resolved (or Fixed) issue #XXX. \<Text of issue description\>**"
Add your changes to file **ChangeLog.txt**!!!

The maintainer will merge your branch into **develop**.
 

# Reviewing 
Most likely we will ask you to fix some issues in your code. In this case you will add your changes to your local **feature** branch, push your **feature** branch to the repo, and update your existing pull request. 
