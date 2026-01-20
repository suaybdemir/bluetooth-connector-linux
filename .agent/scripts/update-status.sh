#!/bin/bash
# ============================================================================
# update-status.sh - Quick Phase Status Updater v2.0.0
# ============================================================================
# Quickly update phase status without manually editing files.
# ============================================================================

set -e

VERSION="2.0.0"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
AGENT_DIR="$(dirname "$SCRIPT_DIR")"
PROJECT_ROOT="$(dirname "$AGENT_DIR")"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
WHITE='\033[1;37m'
NC='\033[0m'

print_header() {
    echo -e "${PURPLE}"
    echo "╔══════════════════════════════════════════════════════════════╗"
    echo "║            📝 Quick Status Updater v${VERSION}                   ║"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
}

show_help() {
    echo "Usage: update-status.sh [OPTIONS] PHASE_NUMBER STATUS"
    echo ""
    echo "Arguments:"
    echo "  PHASE_NUMBER    Phase to update (1, 2, 3, etc.)"
    echo "  STATUS          New status (complete, in_progress, not_started, blocked)"
    echo ""
    echo "Options:"
    echo "  -h, --help      Show this help message"
    echo "  -v, --version   Show version"
    echo "  -f, --file      Path to task_plan.md"
    echo "  -l, --list      List all phases and their current status"
    echo "  -n, --next      Mark current in_progress as complete, start next phase"
    echo "  -a, --all       Set all phases to a status"
    echo ""
    echo "Shortcuts:"
    echo "  c, done         → complete"
    echo "  p, wip          → in_progress"
    echo "  n, pending      → not_started"
    echo "  b, wait         → blocked"
    echo ""
    echo "Examples:"
    echo "  update-status.sh 1 complete        # Mark phase 1 complete"
    echo "  update-status.sh 2 p               # Mark phase 2 in progress"
    echo "  update-status.sh --next            # Auto-advance to next phase"
    echo "  update-status.sh -l                # List all phases"
    echo ""
}

get_timestamp() {
    date "+%Y-%m-%d %H:%M:%S"
}

normalize_status() {
    case "$1" in
        c|done|complete|completed)
            echo "complete"
            ;;
        p|wip|in_progress|active|current)
            echo "in_progress"
            ;;
        n|pending|not_started|todo)
            echo "not_started"
            ;;
        b|wait|waiting|blocked)
            echo "blocked"
            ;;
        *)
            echo "$1"
            ;;
    esac
}

list_phases() {
    local file="$1"
    local phase_num=0
    
    echo -e "${CYAN}Current Phase Status:${NC}"
    echo ""
    
    while IFS= read -r line; do
        if [[ "$line" =~ ^###[[:space:]]Phase[[:space:]]([0-9]+):[[:space:]]*(.+) ]]; then
            phase_num="${BASH_REMATCH[1]}"
            phase_name="${BASH_REMATCH[2]}"
        elif [[ "$line" =~ "Status:"[[:space:]]*(.+) ]] && [ -n "$phase_num" ]; then
            local status="${BASH_REMATCH[1]}"
            status="${status%%\*\*}"
            status=$(echo "$status" | tr -d ' ')
            
            local icon=""
            local color=""
            case "$status" in
                complete|completed) icon="✅"; color="${GREEN}" ;;
                in_progress|active) icon="🔄"; color="${YELLOW}" ;;
                blocked|waiting) icon="🚫"; color="${RED}" ;;
                *) icon="⏸️ "; color="${NC}" ;;
            esac
            
            printf "  ${color}$icon [%d] %-35s %s${NC}\n" "$phase_num" "$phase_name" "$status"
            phase_num=""
        fi
    done < "$file"
    echo ""
}

update_phase_status() {
    local file="$1"
    local target_phase="$2"
    local new_status="$3"
    
    local temp_file=$(mktemp)
    local in_target_phase=false
    local updated=false
    
    while IFS= read -r line; do
        if [[ "$line" =~ ^###[[:space:]]Phase[[:space:]]$target_phase: ]]; then
            in_target_phase=true
        elif [[ "$line" =~ ^###[[:space:]]Phase ]]; then
            in_target_phase=false
        fi
        
        if [ "$in_target_phase" = true ] && [[ "$line" =~ ^-[[:space:]]\*\*Status:\*\* ]]; then
            echo "- **Status:** $new_status" >> "$temp_file"
            updated=true
            
            # Also update Started/Completed timestamps
            if [ "$new_status" = "in_progress" ]; then
                # Read next line to potentially update Started timestamp
                :
            elif [ "$new_status" = "complete" ]; then
                # Will update Completed timestamp
                :
            fi
        else
            echo "$line" >> "$temp_file"
        fi
    done < "$file"
    
    if [ "$updated" = true ]; then
        mv "$temp_file" "$file"
        return 0
    else
        rm -f "$temp_file"
        return 1
    fi
}

advance_to_next() {
    local file="$1"
    local current_phase=""
    local next_phase=""
    
    # Find current in_progress phase
    local phase_num=0
    while IFS= read -r line; do
        if [[ "$line" =~ ^###[[:space:]]Phase[[:space:]]([0-9]+): ]]; then
            phase_num="${BASH_REMATCH[1]}"
        elif [[ "$line" =~ "Status:"[[:space:]]*in_progress ]]; then
            current_phase="$phase_num"
        fi
    done < "$file"
    
    if [ -z "$current_phase" ]; then
        echo -e "${YELLOW}No phase currently in progress. Starting phase 1...${NC}"
        update_phase_status "$file" 1 "in_progress" && \
            echo -e "${GREEN}✅ Phase 1 marked as in_progress${NC}"
        return 0
    fi
    
    next_phase=$((current_phase + 1))
    
    echo -e "${CYAN}Advancing: Phase $current_phase → Phase $next_phase${NC}"
    
    update_phase_status "$file" "$current_phase" "complete" && \
        echo -e "${GREEN}✅ Phase $current_phase marked as complete${NC}"
    
    update_phase_status "$file" "$next_phase" "in_progress" && \
        echo -e "${GREEN}✅ Phase $next_phase marked as in_progress${NC}" || \
        echo -e "${YELLOW}⚠️  Phase $next_phase not found (task may be complete)${NC}"
}

# Parse arguments
TASK_PLAN=""
PHASE_NUM=""
STATUS=""
LIST_MODE=false
NEXT_MODE=false
ALL_MODE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -v|--version)
            echo "Quick Status Updater v$VERSION"
            exit 0
            ;;
        -f|--file)
            TASK_PLAN="$2"
            shift 2
            ;;
        -l|--list)
            LIST_MODE=true
            shift
            ;;
        -n|--next)
            NEXT_MODE=true
            shift
            ;;
        -a|--all)
            ALL_MODE=true
            shift
            ;;
        -*)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
        *)
            if [ -z "$PHASE_NUM" ]; then
                PHASE_NUM="$1"
            elif [ -z "$STATUS" ]; then
                STATUS="$1"
            fi
            shift
            ;;
    esac
done

# Find task_plan.md
if [ -z "$TASK_PLAN" ]; then
    [ -f "$PROJECT_ROOT/task_plan.md" ] && TASK_PLAN="$PROJECT_ROOT/task_plan.md"
    [ -f "./task_plan.md" ] && TASK_PLAN="./task_plan.md"
fi

if [ ! -f "$TASK_PLAN" ]; then
    echo -e "${RED}❌ task_plan.md not found${NC}"
    exit 1
fi

# Execute
print_header

if [ "$LIST_MODE" = true ]; then
    list_phases "$TASK_PLAN"
    exit 0
fi

if [ "$NEXT_MODE" = true ]; then
    advance_to_next "$TASK_PLAN"
    echo ""
    list_phases "$TASK_PLAN"
    exit 0
fi

if [ -z "$PHASE_NUM" ] || [ -z "$STATUS" ]; then
    echo -e "${YELLOW}Usage: update-status.sh PHASE_NUMBER STATUS${NC}"
    echo "Use -h for help, -l to list phases"
    exit 1
fi

STATUS=$(normalize_status "$STATUS")

if update_phase_status "$TASK_PLAN" "$PHASE_NUM" "$STATUS"; then
    echo -e "${GREEN}✅ Phase $PHASE_NUM updated to: $STATUS${NC}"
    echo ""
    list_phases "$TASK_PLAN"
else
    echo -e "${RED}❌ Failed to update phase $PHASE_NUM${NC}"
    exit 1
fi
