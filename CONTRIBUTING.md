# Before pull request

This page describes how to create a pull request. We highly recommend you to read this page before you will decide to send us your changes.

## Named branches

First you should know little bit about our workflow. Right now we work with several named branches.
Read more about git flow branch development here: http://nvie.com/posts/a-successful-git-branching-model/

* *master* - Used only for releases.
* *develop* - Contains the next major release. Used for testing and sharing among developers. 
* *release-x.x.x* - Created from *develop*. Feature freeze state before the next major release. Used for preparing each major & minor release. Merged into *master* with new tag info to create a release.
* *feature-xxx* - Created from *develop*. Contains code for unfinished features. Multiple programmers can work on a *feature*. Merged to *develop*.
* *hotfix-x.x.x* - Created from *develop* and *master*. Contains a quick fix for current release (or proposed release). Merged to *master* (or *release*) and *develop*.


## Submitting your code

Before creating a pull request:

- Read the Github tutorials about making Pull Requests (https://help.github.com/articles/about-pull-requests/) and (https://help.github.com/articles/creating-a-pull-request/)
- Pull latest code and checkout the develop branch. Create your feature branch. You should use **feature-\<issue#\>**

> git pull  
git checkout develop    
git checkout -b feature-\<issue#\>    #creates feature branch  
git checkout feature-\<issue#\>n      #switches to feature branch  

- now make your changes to your **feature** branch
- Be sure you use correct branch for your changes. **This is the most popular mistake.**

- Add correct description for your pull request. It should be "**Resolved (or Fixed) issue #XXX. <Text of an issue description>**". Where XXX is a number of issue in the Issue Tracker.
- Don't forget!!!Add your changes to file ChangeLog.txt!!!
- Merge your local **feature** branch with your local **develop** branch. Fix any issues which arise.
- Then push your local **feature** branch to project repo:
-- git push origin feature-\<issue#\>
- Then make a pull request on the repo so that the maintainer can merge your changes into the develop branch.

# Reviewing 
Most likely we will ask you to fix some issues in your code. In this case you will add your changes to your local **feature** branch, push your **feature** branch to the repo, and update your existing pull request. 
