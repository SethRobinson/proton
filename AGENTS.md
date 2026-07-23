# AGENTS.md

Project operating instructions for AI assistants working in this repository.

## Shared Project Memory

- At the start of each new task or thread involving this repository, read this file before inspecting files, running commands, making a plan, or taking any other project action.
- Treat follow-up replies in the same continuous task as part of that task. Do not reread this file unless the repository or working directory changes, this file is modified, or its instructions are no longer available in context.
- Treat this file as the shared project memory for AI assistants.
- Do not rely on vendor-specific, proprietary, or hidden memory systems for project facts, preferences, or operating instructions. (except to remember to ALWAYS read this file first before doing anything.  Remember that.)
- Update this file with important repo-specific information learned during work, including build commands, test commands, conventions, decisions, pitfalls, and current project preferences.
- Keep this file accurate and current. Remove or correct stale, misleading, or incorrect information when discovered.
- If information is temporary or uncertain, label it clearly rather than presenting it as permanent fact.

Scope policy: this file holds cross-cutting rules, workflows, and gotchas that most sessions need, plus a feature index. Keep it around 30 KB. Feature deep-dives live in `docs/<topic>.md`: before working on a feature listed in the index, read its doc; when finishing feature work, update that doc and keep the index entry here to one or two lines (where it lives + the non-obvious constraint). Cross-cutting rules and new gotchas still land here directly. When a change makes anything stale, here or in a linked doc, update it in the same change.

## Testing

- When possible, design automated tests for new features and bug fixes.
- Run relevant automated tests after finishing changes to guard against regressions.
- If tests cannot be run or do not exist, state that clearly in the handoff and describe any manual verification performed.


## Security

- Never commit sensitive data, including credentials, tokens, passwords, private keys, cookies, customer data, personal data, or machine-specific authentication material.
- If an AI assistant needs authentication data or other secrets for local work, use `agents_secret.md` for those notes.
- `agents_secret.md` must stay ignored by git and must not be committed.
- Do not put secrets in commit messages, logs, issue text, pull request descriptions, generated docs, or other tracked files.
- Before committing, review staged changes for accidental secrets.

## Mac build/test machine

- A Mac for building and testing is reachable at `ssh seth@studiomac.local` (key auth already set up from Seth's PC). Proton checkout lives at `~/projects/proton` there.
- Build OSX demo apps with e.g. `xcodebuild -project RTLooneyLadders/OSX/RTLooneyLadders.xcodeproj -target RTLooneyLadders -configuration Debug build` (run from the repo root). SDL2/SDL2_mixer frameworks are installed in `~/Library/Frameworks` on that machine.
- GUI apps launched over ssh do run (a console session is active), so smoke tests via running the built binary and reading stdout work.

## Git

- `.gitignore` uses a whitelist: `/*` ignores everything at the repo root, and
  tracked projects/files are re-included with `!/Name/` lines at the top of the
  file. New project folders in the root are ignored by default; to start
  tracking one, add a `!/FolderName/` line (do not add per-file ignore lists).
- Never add OpenAI/Codex/Claude etc as a co-author on git commits.
- NEVER `git commit` unless explicitly told to commit.
- NEVER `git push` unless explicitly told to push. "Commit" means commit
  locally only; committing is not permission to push.

