# Contributing code

Thanks for contributing to Seamly code, documentation, translations, website, and wiki!

## How to Contact Us
If you need any help or have any questions, need a lengthy discussion or share documentation, ask on the forum at https://forum.seamly.net. Our users and coders hang out there frequently. You can always find someone there.
Alternatively, if you leave a comment on a repo issue we'll quickly get back to you.

## How we work
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

## Clone the repo & create your issue branch
* Clone the repo, set the origin URL, switch focus to develop branch, and create your feature branch. Examples use branch for issue #155.
```
git clone https://github.com/fashionfreedom/seamly2d  
git remote set-url origin https://github.com/fashionfreedom/seamly2d
git checkout develop       
git checkout -b issue-#155   
git checkout issue-#155    
```

## Daily workflow  
* Pull latest changes from the origin **develop** branch & merge them into your issue branch
```
git pull origin develop  
git checkout issue-#155  
git merge develop
```  
* Make your code changes & commit your changes.  
```
git commit -a  
git commit -m "fix bug with arc and line tool issue-#155"  
git commit push issue-#155
```

## Create a Pull Request -- git CLI
* Build & test your issue branch with Qt 5.15.2 or Qt 5.15.12, and Qt Creator 
* Push your issue branch up to the github repo
```
git push -u origin issue-#155
```
* In the main repo on github, on the main menu bar, select **Branch: develop** then **Pull Requests** 
* From the **Compare:develop** dropdown list, select the green **New Pull Request** button then select your issue branch 
* Enter the Pull Request Header as **"<tag>: <description issue_#nnn>"** using **_conventional-changelog/commit-zen-cli_** tags so your commits are auto-included in the Changelog.md file. Read more here: https://docs.changelogg.io/conventional-changelog-template. Example PR Header: "fix: Arc & line tool #issue-#155".    
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
* When prompted, sign the **Code Licensing Agreement (CLA)**. You sign this only once.
* Select **Linked Issues** in the right hand column and select the issue. Example: select issue #155.  
* The maintainer will check that all CI/CD tests passed, review the code, then merge your issue branch into **develop**.
* Releases are weekly on Monday morning 01:30am UTC.

Thanks for being an open source contributor.
Let us know how to send you a cup of coffee!

