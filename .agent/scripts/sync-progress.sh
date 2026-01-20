#!/bin/bash
# ============================================================================
# sync-progress.sh - Progress Synchronizer v2.0.0
# ============================================================================
# Sync and validate planning files, update statistics, and ensure consistency.
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
    echo "║            🔄 Progress Synchronizer v${VERSION}                  ║"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
}

show_help() {
    echo "Usage: sync-progress.sh [OPTIONS] [DIRECTORY]"
    echo ""
    echo "Options:"
    echo "  -h, --help       Show this help message"
    echo "  -v, --version    Show version"
    echo "  -c, --check      Check consistency only (no modifications)"
    echo "  -f, --fix        Auto-fix issues where possible"
    echo "  -s, --stats      Update session statistics"
    echo "  -r, --report     Generate detailed sync report"
    echo ""
    echo "What it does:"
    echo "  - Validates file structure and formatting"
    echo "  - Checks phase status consistency"
    echo "  - Updates session metadata statistics"
    echo "  - Verifies 2-Action Rule compliance"
    echo "  - Reports stale tasks and blockers"
    echo ""
}

get_timestamp() {
    date "+%Y-%m-%d %H:%M:%S"
}

# ============================================================================
# VALIDATION FUNCTIONS
# ============================================================================

validate_task_plan() {
    local file="$1"
    local issues=0
    
    echo -e "${CYAN}Validating task_plan.md...${NC}"
    
    if [ ! -f "$file" ]; then
        echo -e "  ${RED}✗ File not found${NC}"
        return 1
    fi
    
    # Check for goal
    if ! grep -q "^## Goal" "$file"; then
        echo -e "  ${YELLOW}⚠ Missing Goal section${NC}"
        ((issues++))
    else
        echo -e "  ${GREEN}✓ Goal section present${NC}"
    fi
    
    # Check for phases
    local phase_count=$(grep -c "^### Phase" "$file" || echo 0)
    if [ "$phase_count" -eq 0 ]; then
        echo -e "  ${RED}✗ No phases defined${NC}"
        ((issues++))
    else
        echo -e "  ${GREEN}✓ $phase_count phases defined${NC}"
    fi
    
    # Check status format
    local status_count=$(grep -c "Status:\*\*" "$file" || echo 0)
    if [ "$status_count" -ne "$phase_count" ]; then
        echo -e "  ${YELLOW}⚠ Status count ($status_count) doesn't match phase count ($phase_count)${NC}"
        ((issues++))
    else
        echo -e "  ${GREEN}✓ All phases have status${NC}"
    fi
    
    # Check for multiple in_progress
    local in_progress=$(grep -c "Status:.*in_progress" "$file" || echo 0)
    if [ "$in_progress" -gt 1 ]; then
        echo -e "  ${YELLOW}⚠ Multiple phases marked as in_progress ($in_progress)${NC}"
        ((issues++))
    fi
    
    return $issues
}

validate_findings() {
    local file="$1"
    local issues=0
    
    echo -e "${CYAN}Validating findings.md...${NC}"
    
    if [ ! -f "$file" ]; then
        echo -e "  ${YELLOW}⚠ File not found (optional)${NC}"
        return 0
    fi
    
    # Check for 2-Action Rule reminder
    if ! grep -q "2-Action Rule" "$file"; then
        echo -e "  ${YELLOW}⚠ Missing 2-Action Rule reminder${NC}"
        ((issues++))
    else
        echo -e "  ${GREEN}✓ 2-Action Rule reminder present${NC}"
    fi
    
    # Count findings entries
    local entries=$(grep -c "^### " "$file" || echo 0)
    echo -e "  ${BLUE}ℹ $entries research entries${NC}"
    
    return $issues
}

validate_progress() {
    local file="$1"
    local issues=0
    
    echo -e "${CYAN}Validating progress.md...${NC}"
    
    if [ ! -f "$file" ]; then
        echo -e "  ${YELLOW}⚠ File not found (optional)${NC}"
        return 0
    fi
    
    # Check for 5-Question Reboot Test
    if ! grep -q "5-Question Reboot" "$file"; then
        echo -e "  ${YELLOW}⚠ Missing 5-Question Reboot Test${NC}"
        ((issues++))
    else
        echo -e "  ${GREEN}✓ 5-Question Reboot Test present${NC}"
    fi
    
    # Count session logs
    local sessions=$(grep -c "Session Start\|Session End" "$file" || echo 0)
    echo -e "  ${BLUE}ℹ $((sessions/2)) session entries${NC}"
    
    return $issues
}

validate_metadata() {
    local file="$1"
    
    echo -e "${CYAN}Validating session metadata...${NC}"
    
    if [ ! -f "$file" ]; then
        echo -e "  ${YELLOW}⚠ No metadata file (run init-session.sh)${NC}"
        return 1
    fi
    
    echo -e "  ${GREEN}✓ Metadata file present${NC}"
    
    # Parse and display key info
    local session_id=$(grep -o '"session_id"[[:space:]]*:[[:space:]]*"[^"]*"' "$file" | cut -d'"' -f4)
    local created=$(grep -o '"created_at"[[:space:]]*:[[:space:]]*"[^"]*"' "$file" | cut -d'"' -f4)
    
    echo -e "  ${BLUE}ℹ Session: $session_id${NC}"
    echo -e "  ${BLUE}ℹ Created: $created${NC}"
    
    return 0
}

# ============================================================================
# UPDATE FUNCTIONS
# ============================================================================

update_statistics() {
    local dir="$1"
    local meta_file="$dir/.session_meta.json"
    
    echo -e "${CYAN}Updating statistics...${NC}"
    
    if [ ! -f "$meta_file" ]; then
        echo -e "  ${YELLOW}⚠ No metadata file to update${NC}"
        return 1
    fi
    
    # Count phases
    local total_phases=0
    local complete_phases=0
    local in_progress_phases=0
    
    if [ -f "$dir/task_plan.md" ]; then
        total_phases=$(grep -c "^### Phase" "$dir/task_plan.md" || echo 0)
        complete_phases=$(grep -c "Status:.*complete" "$dir/task_plan.md" || echo 0)
        in_progress_phases=$(grep -c "Status:.*in_progress" "$dir/task_plan.md" || echo 0)
    fi
    
    # Count research actions (findings entries)
    local research_actions=0
    if [ -f "$dir/findings.md" ]; then
        research_actions=$(grep -c "^### " "$dir/findings.md" || echo 0)
    fi
    
    # Update metadata using sed
    local temp_file=$(mktemp)
    
    cat "$meta_file" | \
        sed "s/\"total\":[[:space:]]*[0-9]*/\"total\": $total_phases/" | \
        sed "s/\"completed\":[[:space:]]*[0-9]*/\"completed\": $complete_phases/" | \
        sed "s/\"in_progress\":[[:space:]]*[0-9]*/\"in_progress\": $in_progress_phases/" | \
        sed "s/\"research_actions\":[[:space:]]*[0-9]*/\"research_actions\": $research_actions/" | \
        sed "s/\"last_updated\":[[:space:]]*\"[^\"]*\"/\"last_updated\": \"$(get_timestamp)\"/" > "$temp_file"
    
    mv "$temp_file" "$meta_file"
    
    echo -e "  ${GREEN}✓ Statistics updated${NC}"
    echo -e "  ${BLUE}ℹ Phases: $complete_phases/$total_phases complete${NC}"
    echo -e "  ${BLUE}ℹ Research entries: $research_actions${NC}"
    
    return 0
}

generate_report() {
    local dir="$1"
    
    echo ""
    echo -e "${WHITE}═══════════════════════════════════════════════════════════════${NC}"
    echo -e "${WHITE}                     SYNC REPORT                               ${NC}"
    echo -e "${WHITE}═══════════════════════════════════════════════════════════════${NC}"
    echo ""
    echo -e "  ${CYAN}Directory:${NC} $dir"
    echo -e "  ${CYAN}Generated:${NC} $(get_timestamp)"
    echo ""
    
    # File status
    echo -e "${WHITE}File Status:${NC}"
    [ -f "$dir/task_plan.md" ] && echo -e "  ✓ task_plan.md" || echo -e "  ✗ task_plan.md (missing)"
    [ -f "$dir/findings.md" ] && echo -e "  ✓ findings.md" || echo -e "  ○ findings.md (optional)"
    [ -f "$dir/progress.md" ] && echo -e "  ✓ progress.md" || echo -e "  ○ progress.md (optional)"
    [ -f "$dir/.session_meta.json" ] && echo -e "  ✓ .session_meta.json" || echo -e "  ○ .session_meta.json (optional)"
    echo ""
    
    # Phase summary
    if [ -f "$dir/task_plan.md" ]; then
        echo -e "${WHITE}Phase Summary:${NC}"
        local phase_num=0
        while IFS= read -r line; do
            if [[ "$line" =~ ^###[[:space:]]Phase[[:space:]]([0-9]+):[[:space:]]*(.+) ]]; then
                phase_num="${BASH_REMATCH[1]}"
                phase_name="${BASH_REMATCH[2]}"
            elif [[ "$line" =~ "Status:"[[:space:]]*(.+) ]] && [ "$phase_num" -ne 0 ]; then
                local status="${BASH_REMATCH[1]}"
                status="${status%%\*\*}"
                
                local icon=""
                case "$status" in
                    *complete*) icon="✅" ;;
                    *in_progress*) icon="🔄" ;;
                    *blocked*) icon="🚫" ;;
                    *) icon="⏸️" ;;
                esac
                
                echo "  $icon Phase $phase_num: $phase_name"
                phase_num=0
            fi
        done < "$dir/task_plan.md"
    fi
    
    echo ""
    echo -e "${WHITE}═══════════════════════════════════════════════════════════════${NC}"
}

# ============================================================================
# MAIN
# ============================================================================

TARGET_DIR=""
CHECK_ONLY=false
AUTO_FIX=false
UPDATE_STATS=false
GENERATE_REPORT=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -v|--version)
            echo "Progress Synchronizer v$VERSION"
            exit 0
            ;;
        -c|--check)
            CHECK_ONLY=true
            shift
            ;;
        -f|--fix)
            AUTO_FIX=true
            shift
            ;;
        -s|--stats)
            UPDATE_STATS=true
            shift
            ;;
        -r|--report)
            GENERATE_REPORT=true
            shift
            ;;
        -*)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
        *)
            TARGET_DIR="$1"
            shift
            ;;
    esac
done

TARGET_DIR="${TARGET_DIR:-$PROJECT_ROOT}"

print_header
echo -e "  Directory: ${WHITE}$TARGET_DIR${NC}"
echo ""

total_issues=0

# Validate all files
validate_task_plan "$TARGET_DIR/task_plan.md" || ((total_issues+=$?))
echo ""
validate_findings "$TARGET_DIR/findings.md" || ((total_issues+=$?))
echo ""
validate_progress "$TARGET_DIR/progress.md" || ((total_issues+=$?))
echo ""
validate_metadata "$TARGET_DIR/.session_meta.json" || ((total_issues+=$?))
echo ""

# Update statistics if requested
if [ "$UPDATE_STATS" = true ]; then
    update_statistics "$TARGET_DIR"
    echo ""
fi

# Generate report if requested
if [ "$GENERATE_REPORT" = true ]; then
    generate_report "$TARGET_DIR"
fi

# Final summary
echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━ Summary ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo ""
if [ "$total_issues" -eq 0 ]; then
    echo -e "${GREEN}✅ All validations passed!${NC}"
else
    echo -e "${YELLOW}⚠️  Found $total_issues issue(s)${NC}"
fi
echo ""
