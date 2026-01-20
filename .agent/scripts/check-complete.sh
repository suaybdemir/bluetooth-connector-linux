#!/bin/bash
# ============================================================================
# check-complete.sh - Advanced Task Completion Analyzer v2.0.0
# ============================================================================
# Comprehensive completion analysis with progress visualization,
# time tracking, checklist validation, and detailed reporting.
# ============================================================================

# Note: set -e removed to prevent premature exit during loop operations

# ============================================================================
# CONFIGURATION
# ============================================================================
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
GRAY='\033[0;90m'
NC='\033[0m'

# ============================================================================
# HELPER FUNCTIONS
# ============================================================================

print_header() {
    echo -e "${PURPLE}"
    echo "╔══════════════════════════════════════════════════════════════╗"
    echo "║           🔍 Planning with Files - Completion Check          ║"
    echo "║                        v${VERSION}                               ║"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
}

print_success() { echo -e "${GREEN}✅ $1${NC}"; }
print_warning() { echo -e "${YELLOW}⚠️  $1${NC}"; }
print_error() { echo -e "${RED}❌ $1${NC}"; }
print_info() { echo -e "${BLUE}ℹ️  $1${NC}"; }

show_help() {
    echo "Usage: check-complete.sh [OPTIONS] [TASK_PLAN_PATH]"
    echo ""
    echo "Options:"
    echo "  -h, --help       Show this help message"
    echo "  -v, --version    Show version information"
    echo "  -d, --detailed   Show detailed phase breakdown"
    echo "  -c, --checklist  Analyze all checklist items"
    echo "  -t, --timeline   Show phase timeline"
    echo "  -s, --stats      Show session statistics"
    echo "  -j, --json       Output in JSON format"
    echo "  -q, --quiet      Only output final status (exit code)"
    echo "  -a, --all        Show all reports"
    echo ""
    echo "Exit Codes:"
    echo "  0 - All phases complete"
    echo "  1 - File not found or parse error"
    echo "  2 - Some phases in progress"
    echo "  3 - Phases not started"
    echo ""
}

draw_progress_bar() {
    local current=$1
    local total=$2
    local width=${3:-40}
    
    if [ "$total" -eq 0 ]; then
        echo "[$(printf '░%.0s' $(seq 1 $width))] 0%"
        return
    fi
    
    local percentage=$((current * 100 / total))
    local filled=$((current * width / total))
    local empty=$((width - filled))
    
    local bar=""
    [ "$filled" -gt 0 ] && bar+=$(printf '█%.0s' $(seq 1 $filled))
    [ "$empty" -gt 0 ] && bar+=$(printf '░%.0s' $(seq 1 $empty))
    
    local color="${GREEN}"
    [ "$percentage" -lt 50 ] && color="${YELLOW}"
    [ "$percentage" -lt 25 ] && color="${RED}"
    
    echo -e "${color}[$bar] ${percentage}%${NC}"
}

# ============================================================================
# ANALYSIS FUNCTIONS
# ============================================================================

analyze_phases() {
    local task_plan="$1"
    
    TOTAL_PHASES=0
    COMPLETE_PHASES=0
    IN_PROGRESS_PHASES=0
    NOT_STARTED_PHASES=0
    BLOCKED_PHASES=0
    
    declare -a PHASE_NAMES
    declare -a PHASE_STATUSES
    declare -a PHASE_LINES
    
    local current_phase=""
    local line_num=0
    
    while IFS= read -r line; do
        ((line_num++))
        
        if [[ "$line" =~ ^###[[:space:]]Phase[[:space:]]([0-9]+): ]]; then
            current_phase="${line#*: }"  # Extract everything after ": "
            PHASE_NAMES+=("$current_phase")
            PHASE_LINES+=("$line_num")
            ((TOTAL_PHASES++))
        elif [[ "$line" =~ \*\*Status:\*\*[[:space:]]*([a-z_]+) ]]; then
            local status="${BASH_REMATCH[1]}"
            
            PHASE_STATUSES+=("$status")
            
            case "$status" in
                complete|completed|done)
                    ((COMPLETE_PHASES++))
                    ;;
                in_progress|active|current)
                    ((IN_PROGRESS_PHASES++))
                    ;;
                blocked|waiting)
                    ((BLOCKED_PHASES++))
                    ;;
                not_started|pending|*)
                    ((NOT_STARTED_PHASES++))
                    ;;
            esac
        fi
    done < "$task_plan"
}

analyze_checklists() {
    local task_plan="$1"
    
    TOTAL_ITEMS=0
    CHECKED_ITEMS=0
    UNCHECKED_ITEMS=0
    IN_PROGRESS_ITEMS=0
    
    while IFS= read -r line; do
        if [[ "$line" =~ ^[[:space:]]*-[[:space:]]\[x\] ]]; then
            ((TOTAL_ITEMS++))
            ((CHECKED_ITEMS++))
        elif [[ "$line" =~ ^[[:space:]]*-[[:space:]]\[/\] ]]; then
            ((TOTAL_ITEMS++))
            ((IN_PROGRESS_ITEMS++))
        elif [[ "$line" =~ ^[[:space:]]*-[[:space:]]\[[[:space:]]\] ]]; then
            ((TOTAL_ITEMS++))
            ((UNCHECKED_ITEMS++))
        fi
    done < "$task_plan"
}

analyze_session_stats() {
    local dir="$1"
    local meta_file="$dir/.session_meta.json"
    
    if [ -f "$meta_file" ]; then
        SESSION_ID=$(grep -o '"session_id"[[:space:]]*:[[:space:]]*"[^"]*"' "$meta_file" | cut -d'"' -f4)
        CREATED_AT=$(grep -o '"created_at"[[:space:]]*:[[:space:]]*"[^"]*"' "$meta_file" | cut -d'"' -f4)
        SESSION_GOAL=$(grep -o '"goal"[[:space:]]*:[[:space:]]*"[^"]*"' "$meta_file" | cut -d'"' -f4)
    else
        SESSION_ID="unknown"
        CREATED_AT="unknown"
        SESSION_GOAL=""
    fi
    
    # Count findings entries
    FINDINGS_COUNT=0
    if [ -f "$dir/findings.md" ]; then
        FINDINGS_COUNT=$(grep -c "^### " "$dir/findings.md" 2>/dev/null || echo 0)
    fi
    
    # Count progress entries
    PROGRESS_ENTRIES=0
    if [ -f "$dir/progress.md" ]; then
        PROGRESS_ENTRIES=$(grep -c "Session Start\|Session End" "$dir/progress.md" 2>/dev/null || echo 0)
    fi
}

# ============================================================================
# REPORT FUNCTIONS
# ============================================================================

print_phase_summary() {
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━ Phase Summary ━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    
    echo -e "  Total Phases:    ${WHITE}$TOTAL_PHASES${NC}"
    echo -e "  ✅ Complete:     ${GREEN}$COMPLETE_PHASES${NC}"
    echo -e "  🔄 In Progress:  ${YELLOW}$IN_PROGRESS_PHASES${NC}"
    echo -e "  ⏸️  Not Started:  ${GRAY}$NOT_STARTED_PHASES${NC}"
    [ "$BLOCKED_PHASES" -gt 0 ] && echo -e "  🚫 Blocked:      ${RED}$BLOCKED_PHASES${NC}"
    echo ""
    
    echo -n "  Progress: "
    draw_progress_bar "$COMPLETE_PHASES" "$TOTAL_PHASES" 30
    echo ""
}

print_detailed_phases() {
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━ Phase Details ━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    
    for i in "${!PHASE_NAMES[@]}"; do
        local name="${PHASE_NAMES[$i]}"
        local status="${PHASE_STATUSES[$i]}"
        local line="${PHASE_LINES[$i]}"
        
        local icon=""
        local color=""
        
        case "$status" in
            complete|completed|done)
                icon="✅"
                color="${GREEN}"
                ;;
            in_progress|active|current)
                icon="🔄"
                color="${YELLOW}"
                ;;
            blocked|waiting)
                icon="🚫"
                color="${RED}"
                ;;
            *)
                icon="⏸️ "
                color="${GRAY}"
                ;;
        esac
        
        printf "  ${color}$icon Phase %d: %-30s [%s]${NC}\n" "$((i+1))" "$name" "$status"
    done
    echo ""
}

print_checklist_summary() {
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━ Checklist Summary ━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    
    echo -e "  Total Items:     ${WHITE}$TOTAL_ITEMS${NC}"
    echo -e "  ✅ Complete:     ${GREEN}$CHECKED_ITEMS${NC}"
    echo -e "  🔄 In Progress:  ${YELLOW}$IN_PROGRESS_ITEMS${NC}"
    echo -e "  ⬜ Pending:      ${GRAY}$UNCHECKED_ITEMS${NC}"
    echo ""
    
    echo -n "  Progress: "
    draw_progress_bar "$CHECKED_ITEMS" "$TOTAL_ITEMS" 30
    echo ""
}

print_session_stats() {
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━ Session Statistics ━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    
    echo -e "  Session ID:      ${WHITE}$SESSION_ID${NC}"
    echo -e "  Created:         ${WHITE}$CREATED_AT${NC}"
    [ -n "$SESSION_GOAL" ] && echo -e "  Goal:            ${PURPLE}$SESSION_GOAL${NC}"
    echo -e "  Findings:        ${BLUE}$FINDINGS_COUNT entries${NC}"
    echo -e "  Progress Logs:   ${BLUE}$PROGRESS_ENTRIES entries${NC}"
    echo ""
}

print_final_status() {
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━ Final Status ━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    
    if [ "$TOTAL_PHASES" -eq 0 ]; then
        print_warning "No phases found in task_plan.md"
        echo ""
        echo "  Make sure phases use format:"
        echo "    ### Phase N: Name"
        echo "    - **Status:** complete|in_progress|not_started"
        echo ""
        return 1
    elif [ "$COMPLETE_PHASES" -eq "$TOTAL_PHASES" ]; then
        echo -e "${GREEN}╔══════════════════════════════════════════════════════════════╗${NC}"
        echo -e "${GREEN}║                  🎉 ALL PHASES COMPLETE! 🎉                  ║${NC}"
        echo -e "${GREEN}╚══════════════════════════════════════════════════════════════╝${NC}"
        echo ""
        echo "  Task is ready for final review and handoff."
        echo "  Run ${CYAN}archive-session.sh${NC} to archive this session."
        echo ""
        return 0
    elif [ "$IN_PROGRESS_PHASES" -gt 0 ]; then
        echo -e "${YELLOW}╔══════════════════════════════════════════════════════════════╗${NC}"
        echo -e "${YELLOW}║                   🔄 TASK IN PROGRESS                        ║${NC}"
        echo -e "${YELLOW}╚══════════════════════════════════════════════════════════════╝${NC}"
        echo ""
        echo "  Active phase(s) need completion."
        
        # Show current phase
        for i in "${!PHASE_STATUSES[@]}"; do
            if [[ "${PHASE_STATUSES[$i]}" =~ in_progress|active|current ]]; then
                echo -e "  📍 Working on: ${CYAN}Phase $((i+1)): ${PHASE_NAMES[$i]}${NC}"
            fi
        done
        echo ""
        return 2
    else
        echo -e "${GRAY}╔══════════════════════════════════════════════════════════════╗${NC}"
        echo -e "${GRAY}║                   ⏸️  NOT YET STARTED                         ║${NC}"
        echo -e "${GRAY}╚══════════════════════════════════════════════════════════════╝${NC}"
        echo ""
        echo "  Update Phase 1 status to 'in_progress' to begin."
        echo ""
        return 3
    fi
}

output_json() {
    cat << EOF
{
    "version": "$VERSION",
    "file": "$TASK_PLAN",
    "phases": {
        "total": $TOTAL_PHASES,
        "complete": $COMPLETE_PHASES,
        "in_progress": $IN_PROGRESS_PHASES,
        "not_started": $NOT_STARTED_PHASES,
        "blocked": ${BLOCKED_PHASES:-0},
        "percentage": $([ "$TOTAL_PHASES" -gt 0 ] && echo "$((COMPLETE_PHASES * 100 / TOTAL_PHASES))" || echo 0)
    },
    "checklists": {
        "total": $TOTAL_ITEMS,
        "checked": $CHECKED_ITEMS,
        "in_progress": $IN_PROGRESS_ITEMS,
        "unchecked": $UNCHECKED_ITEMS
    },
    "session": {
        "id": "$SESSION_ID",
        "created": "$CREATED_AT",
        "goal": "$SESSION_GOAL"
    },
    "status": "$([ "$COMPLETE_PHASES" -eq "$TOTAL_PHASES" ] && [ "$TOTAL_PHASES" -gt 0 ] && echo "complete" || ([ "$IN_PROGRESS_PHASES" -gt 0 ] && echo "in_progress" || echo "not_started"))"
}
EOF
}

# ============================================================================
# PARSE ARGUMENTS
# ============================================================================

DETAILED=false
CHECKLIST=false
TIMELINE=false
STATS=false
JSON=false
QUIET=false
TASK_PLAN=""

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -v|--version)
            echo "Task Completion Analyzer v$VERSION"
            exit 0
            ;;
        -d|--detailed)
            DETAILED=true
            shift
            ;;
        -c|--checklist)
            CHECKLIST=true
            shift
            ;;
        -t|--timeline)
            TIMELINE=true
            shift
            ;;
        -s|--stats)
            STATS=true
            shift
            ;;
        -j|--json)
            JSON=true
            shift
            ;;
        -q|--quiet)
            QUIET=true
            shift
            ;;
        -a|--all)
            DETAILED=true
            CHECKLIST=true
            STATS=true
            shift
            ;;
        -*)
            print_error "Unknown option: $1"
            exit 1
            ;;
        *)
            TASK_PLAN="$1"
            shift
            ;;
    esac
done

# Find task_plan.md
if [ -z "$TASK_PLAN" ]; then
    if [ -f "$PROJECT_ROOT/task_plan.md" ]; then
        TASK_PLAN="$PROJECT_ROOT/task_plan.md"
    elif [ -f "./task_plan.md" ]; then
        TASK_PLAN="./task_plan.md"
    else
        [ "$QUIET" != "true" ] && [ "$JSON" != "true" ] && print_error "task_plan.md not found!"
        exit 1
    fi
fi

TASK_DIR="$(dirname "$TASK_PLAN")"

# ============================================================================
# MAIN EXECUTION
# ============================================================================

if [ ! -f "$TASK_PLAN" ]; then
    [ "$QUIET" != "true" ] && [ "$JSON" != "true" ] && print_error "File not found: $TASK_PLAN"
    exit 1
fi

# Run analysis
analyze_phases "$TASK_PLAN"
analyze_checklists "$TASK_PLAN"
analyze_session_stats "$TASK_DIR"

# Output
if [ "$JSON" = "true" ]; then
    output_json
    [ "$COMPLETE_PHASES" -eq "$TOTAL_PHASES" ] && [ "$TOTAL_PHASES" -gt 0 ] && exit 0
    [ "$IN_PROGRESS_PHASES" -gt 0 ] && exit 2
    exit 3
fi

if [ "$QUIET" = "true" ]; then
    [ "$COMPLETE_PHASES" -eq "$TOTAL_PHASES" ] && [ "$TOTAL_PHASES" -gt 0 ] && exit 0
    [ "$IN_PROGRESS_PHASES" -gt 0 ] && exit 2
    exit 3
fi

print_header
echo -e "  File: ${WHITE}$TASK_PLAN${NC}"
echo ""

print_phase_summary
[ "$DETAILED" = "true" ] && print_detailed_phases
[ "$CHECKLIST" = "true" ] && print_checklist_summary
[ "$STATS" = "true" ] && print_session_stats
print_final_status

exit $?
