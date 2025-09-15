# 0) optional: if you have unstaged changes
# git add -A; git commit -m "wip"  # or: git stash -u

# 1) get the remote commits locally (no integration yet)
git fetch origin

# 2) replay your local commits on top of origin/main
git rebase --autostash origin/main
# If there are conflicts:
#   - edit files to resolve
#   - git add <resolved files>
#   - git rebase --continue
# If you need to bail out:
#   - git rebase --abort

# 3) push the rebased commits (safe force)
git push --force-with-lease
