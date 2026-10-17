# CLAUDE.md

## Project
- Test: `make t`

## How we work
You are one of several agents working in this repo at the same time. Each agent
has its own git worktree and branch. The human reviews all work as pull requests
on GitHub, not in this terminal.

### Rules
- Stay on your current branch. Never check out, commit to, or push to `main`.
- Only edit files inside this worktree.
- Never merge PRs, approve PRs, or force-push to a branch that isn't yours.
- Don't change CI config, branch protection, or secrets unless the task asks for it.
- If the task is unclear or blocked, stop and ask instead of guessing.

### Finishing a task
1. Run tests. Fix anything that fails. If something can't be fixed,
   note it in the PR description.
2. Commit with clear, focused messages (imperative mood, e.g. "Add retry to upload client").
3. Pull in the latest main and resolve conflicts:
   `git fetch origin && git rebase origin/main`
4. Push: `git push -u origin HEAD`
5. Open a PR with `gh pr create`, using a title that says what changed and this body:

   ```
   ## What
   Short summary of the change.

   ## Why
   The problem or task this addresses.

   ## How it was tested
   Commands run and results. Anything not tested.

   ## Notes for reviewer
   Risky areas, open questions, follow-ups.
   ```
6. Reply with the PR URL and stop.

### Addressing review feedback
When asked to address comments on a PR:
1. Read them: `gh pr view <number> --comments` and
   `gh api repos/{owner}/{repo}/pulls/<number>/comments` for inline comments.
2. Make the changes on the same branch, run tests and lint, commit, and push.
   Don't open a new PR.
3. Reply with a short summary of what you changed for each comment, and flag
   any comment you disagreed with or couldn't address.