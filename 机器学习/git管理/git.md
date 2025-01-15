#基于某次提交而生成的分支#
找到生成分支的提交记录,然后使用git checkout -b 分支名 commit_id

#回退上次提交的修改#
git reset --hard HEAD^  再次同步提交需要git push -f

##