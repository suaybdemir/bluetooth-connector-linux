---
description: Planning with Files - Structured task management with hooks, templates and scripts
---

# Planning with Files Workflow

## Core Principles

### The 2-Action Rule
After every 2 view/browser/search operations, save findings to files:
- WebSearch → WebSearch → MUST Write findings.md

### The 3-Strike Error Protocol
Structured error recovery:
1. Diagnose & Fix
2. Alternative Approach  
3. Broader Rethink
4. Escalate to User

### The 5-Question Reboot Test
Your planning files should answer:
1. **Where am I?** → Current phase in task_plan.md
2. **Where am I going?** → Remaining phases
3. **What's the goal?** → Goal statement
4. **What have I learned?** → findings.md
5. **What have I done?** → progress.md

## Usage

// turbo-all

### Initialize Session
```bash
./scripts/init-session.sh
```

### Check Task Completion
```bash
./scripts/check-complete.sh
```

### Update Task Status
Edit `task_plan.md` and change phase status:
- `not_started` → `in_progress` → `complete`

## File Locations
- `templates/task_plan.md` - Phase tracking template
- `templates/findings.md` - Research storage template  
- `templates/progress.md` - Session log template
- `scripts/init-session.sh` - Session initializer
- `scripts/check-complete.sh` - Completion verifier
