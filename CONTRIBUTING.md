# Contributing code

Thanks for contributing to Seamly code, documentation, translations, website, and wiki! 

Our workflow is based on **Git Flow**, outlined in the steps below.    
As always, please fix only one issue at a time.   
Build your branch with Qt and test, then make a pull request.   

Read more about **Git Flow**:  http://nvie.com/posts/a-successful-git-branching-model/   
Read more about **Pull Requests**: https://help.github.com/articles/about-pull-requests/ and https://help.github.com/articles/creating-a-pull-request/    

## Naming conventions 

Name your branch after the issue you will fix. Example: Name your branch **issue-#155** for issue #155.

The permanent named branches on this repo are: 
* **develop** - Contains next major release. Used for testing and sharing among developers. 
* **master** - Used only for releases.
* **release-x.x.x** - Created from **develop**. Contains feature freeze state before the next major release. Used for preparing each major & minor release with updated build information. Merge to **master** with new tag to create a new release.
* **hotfix-x.x.x** - Create from **develop** and **master**. Contains a quick emergency fix for big bugs. Merge to **master** (or **release**) and **develop**.

## Clone the repo & create your issue branch

* Clone the repo & set the origin URL
>git clone https://github.com/fashionfreedom/seamly2d  
git remote set-url origin https://github.com/fashionfreedom/seamly2d 

* Create your issue branch then switch focus to your issue branch. Examples use branch for issue #155.
>git checkout develop       
git checkout -b issue-#155   
git checkout issue-#155    

## Daily workflow  

* Pull latest changes from the origin **develop** branch & merge them into your issue branch 
>git pull origin develop  
git checkout issue-#155  
git merge develop  

* Make your code changes & commit your changes.  
>git commit -a  
git commit -m "fix bug with arc and line tool issue-#155"  
git commit push issue-#155 

## Create a Pull Request

* Build & test your issue branch with Qt 5.13.2 & Qt Creator 4.3.x with compilers MSVC 2017, MinGW 7.3.0, gcc, or clang. 

* Push your issue branch up to the github repo.
>git push -u origin issue-#155 

* Select **Branch:develop** then **Pull Requests** on the repo main menu bar. 
* Select the green **New Pull Request** button then select your issue branch from the **Compare:develop** dropdown list.  
* Enter the Pull Request Header as "tag: description issue_" using _conventional-changelog/commit-zen-cli_ tags so your commits are auto-included in the Changelog.md file. Read more here: https://docs.changelogg.io/conventional-changelog-template. Example PR Header: "fix: bug with Arc and line tool #issue-#155".    
 + Valid Tags are: 
    - fix
    - feat
    - build
    - ci
    - chore
    - docs
    - perf
    - refactor
    - revert
    - style
    - test 
* Click the **Create Pull Request** button.
* Select **Linked Issues** in the right hand column and select the issue. Example: select issue #155.  
* The maintainer will check that all CI/CD tests passed, review the code, then merge your issue branch into **develop**.
* At the next build, the PR Header description will be auto-added to the ChangeLog.md file.


Thanks for being an open source contributor!  
