# git-safe-pull-once.ps1
# Refuse to pull if you have unpushed commits or a dirty tree.
$ErrorActionPreference = 'Stop'

# 1) Ensure we're in a git repo
git rev-parse --git-dir 2>$null | Out-Null
if ($LASTEXITCODE -ne 0) { Write-Error "Not a Git repo."; exit 1 }

# 2) Ensure upstream exists
$branch   = (git rev-parse --abbrev-ref HEAD).Trim()
$upstream = git rev-parse --abbrev-ref --symbolic-full-name "@{u}" 2>$null
if ($LASTEXITCODE -ne 0) {
  Write-Error "No upstream set for '$branch'. Set it with:`n  git branch --set-upstream-to origin/$branch"
  exit 1
}

# 3) Refuse if ahead of upstream (unpushed commits)
$counts = (git rev-list --left-right --count HEAD..."@{u}").Trim()
$parts  = ($counts -split '\s+')
$ahead  = [int]$parts[0]
$behind = [int]$parts[1]

if ($ahead -gt 0) {
  Write-Error "Refusing to pull: you are ahead of $upstream by $ahead commit(s)."
  Write-Output "Push first:  git push --force-with-lease"
  Write-Output "Or rebase after fetch:  git fetch; git rebase --autostash @{u}"
  exit 1
}

# 4) Refuse if working tree is dirty (optional but safer)
if ((git status --porcelain)) {
  Write-Error "Working tree not clean. Commit or stash before pulling."
  exit 1
}

# 5) Safe pull (no merges)
git pull --ff-only --tags
exit $LASTEXITCODE
