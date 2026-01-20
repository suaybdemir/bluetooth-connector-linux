# Hooks (Automatic Behaviors)

This directory contains hook definitions for automating key planning principles.

## Available Hooks

### PreToolUse Hook
- **Trigger:** Before Write/Edit/Bash operations
- **Behavior:** Reads `task_plan.md` to refresh goals
- **Benefit:** Automatically keeps context without manual re-reading

### Stop Hook  
- **Trigger:** Before stopping/completing
- **Behavior:** Verifies all phases are complete via `check-complete.sh`
- **Benefit:** Ensures no phases are left incomplete

## Hook Configuration

Hooks are defined in `.agent/hooks/hooks.json`:

```json
{
  "hooks": {
    "PreToolUse": {
      "enabled": true,
      "actions": ["read_task_plan"]
    },
    "Stop": {
      "enabled": true,
      "actions": ["verify_completion"]
    }
  }
}
```

## Usage Notes

1. Hooks are automatically triggered - no manual intervention needed
2. If `task_plan.md` doesn't exist, hooks gracefully skip
3. To disable hooks temporarily, set `enabled: false` in config
