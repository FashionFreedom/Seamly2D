# Before pull request

This page describes how to create a pull request. We highly recommend you to read this page before you will decide to send us your changes.

## Named branches

First you should know little bit about our workflow. Right now we work with several named branches.

* *master* - used only for releases.
* *develop* - branch that contains a code for the next major release. Code in this branch good enough for testing and sharing among developers.
* *feature* - branch that contains a code for an unfinished features.
* *release* - used for feature freeze state before the next major release. And for preparing the next minor release.

These are the main branches you should know about. There are several long term task specific branches in repository too. But we will not discuss them in this document.

## Check yourself

There are several things you should know for successful creating a pull request.

- Please, read the Github tutorials about making Pull Requests (https://help.github.com/articles/about-pull-requests/) and (https://help.github.com/articles/creating-a-pull-request/).
- You should use **feature**.
- Create your local **feature** branch:
-- git checkout develop
-- git checkout -b feature
-- git checkout feature
-- #now make your changes to your **feature** branch
- Be sure you use correct branch for your changes. **This is the most popular mistake.**
- - For regular patches (contains several commits) you should use the branch **feature**. 
- Add correct description for your pull request. It should contain string "**Resolved (or Fixed) issue #XXX. <Text of an issue description>**". Where XXX is a number of issue in the Issue Tracker.
- Don't forget!!!Add your changes to file ChangeLog.txt!!!
- Merge your local **feature** branch with your local **develop** branch. Fix any issues which arist.
- Then push your local **feature** branch to project repo:
-- git push origin feature
- Then make a pull request.

# Reviewing 
Most likely we will ask you to fix some issues in a code. In this case you will add your changes to the **feature** branch and update your pull request. No need to create new one each time.
