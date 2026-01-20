#!/bin/bash
# ============================================================================
# init-session.sh - Advanced Planning Session Initializer v2.0.0
# ============================================================================
# Creates planning files from templates with interactive configuration,
# backup support, and session metadata tracking.
# ============================================================================

set -e

# ============================================================================
# CONFIGURATION
# ============================================================================
VERSION="2.0.0"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
AGENT_DIR="$(dirname "$SCRIPT_DIR")"
TEMPLATES_DIR="$AGENT_DIR/templates"
PROJECT_ROOT="$(dirname "$AGENT_DIR")"
SESSIONS_DIR="$AGENT_DIR/sessions"
ARCHIVE_DIR="$AGENT_DIR/archive"
CONFIG_FILE="$AGENT_DIR/config/settings.json"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
WHITE='\033[1;37m'
NC='\033[0m' # No Color

# ============================================================================
# HELPER FUNCTIONS
# ============================================================================

print_header() {
    echo -e "${PURPLE}"
    echo "╔══════════════════════════════════════════════════════════════╗"
    echo "║        🧠 Planning with Files - Session Initializer          ║"
    echo "║                        v${VERSION}                               ║"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
}

print_success() { echo -e "${GREEN}✅ $1${NC}"; }
print_warning() { echo -e "${YELLOW}⚠️  $1${NC}"; }
print_error() { echo -e "${RED}❌ $1${NC}"; }
print_info() { echo -e "${BLUE}ℹ️  $1${NC}"; }
print_step() { echo -e "${CYAN}➤ $1${NC}"; }

show_help() {
    echo "Usage: init-session.sh [OPTIONS] [OUTPUT_DIR]"
    echo ""
    echo "Options:"
    echo "  -h, --help          Show this help message"
    echo "  -v, --version       Show version information"
    echo "  -i, --interactive   Run in interactive mode (prompts for goal)"
    echo "  -f, --force         Overwrite existing files (creates backups)"
    echo "  -g, --goal TEXT     Set the goal directly"
    echo "  -n, --name TEXT     Set the session name"
    echo "  -p, --phases N      Number of phases to create (default: 5)"
    echo "  -q, --quiet         Suppress non-essential output"
    echo "  -b, --no-backup     Skip backup when overwriting"
    echo "  --archive           Archive existing session before creating new"
    echo ""
    echo "Examples:"
    echo "  init-session.sh                        # Initialize in project root"
    echo "  init-session.sh ./planning             # Initialize in ./planning"
    echo "  init-session.sh -i                     # Interactive mode"
    echo "  init-session.sh -g 'Build API' -n api  # With goal and name"
    echo "  init-session.sh -f --archive           # Force with archive"
    echo ""
}

get_timestamp() {
    date "+%Y-%m-%d %H:%M:%S"
}

get_date() {
    date "+%Y-%m-%d"
}

get_session_id() {
    date "+%Y%m%d_%H%M%S"
}

create_backup() {
    local file="$1"
    local backup_dir="$AGENT_DIR/backups/$(get_date)"
    
    if [ -f "$file" ]; then
        mkdir -p "$backup_dir"
        local basename=$(basename "$file")
        local backup_file="$backup_dir/${basename%.md}_$(get_session_id).md"
        cp "$file" "$backup_file"
        print_info "Backed up: $basename → backups/$(get_date)/"
        return 0
    fi
    return 1
}

archive_session() {
    local output_dir="$1"
    local archive_name="session_$(get_session_id)"
    
    mkdir -p "$ARCHIVE_DIR"
    
    if [ -f "$output_dir/task_plan.md" ] || [ -f "$output_dir/findings.md" ] || [ -f "$output_dir/progress.md" ]; then
        local archive_path="$ARCHIVE_DIR/$archive_name"
        mkdir -p "$archive_path"
        
        [ -f "$output_dir/task_plan.md" ] && cp "$output_dir/task_plan.md" "$archive_path/"
        [ -f "$output_dir/findings.md" ] && cp "$output_dir/findings.md" "$archive_path/"
        [ -f "$output_dir/progress.md" ] && cp "$output_dir/progress.md" "$archive_path/"
        
        print_success "Archived existing session to: archive/$archive_name/"
        return 0
    fi
    return 1
}

update_session_metadata() {
    local output_dir="$1"
    local goal="$2"
    local session_name="$3"
    
    local metadata_file="$output_dir/.session_meta.json"
    
    cat > "$metadata_file" << EOF
{
    "session_id": "$(get_session_id)",
    "created_at": "$(get_timestamp)",
    "goal": "$goal",
    "name": "$session_name",
    "version": "$VERSION",
    "status": "active",
    "phases": {
        "total": $NUM_PHASES,
        "completed": 0,
        "in_progress": 0
    },
    "files": {
        "task_plan": "task_plan.md",
        "findings": "findings.md",
        "progress": "progress.md"
    },
    "stats": {
        "research_actions": 0,
        "errors_encountered": 0,
        "last_updated": "$(get_timestamp)"
    }
}
EOF
}

inject_goal_into_template() {
    local file="$1"
    local goal="$2"
    local session_name="$3"
    
    if [ -n "$goal" ] && [ -f "$file" ]; then
        # Replace placeholder with actual goal
        sed -i "s/\[Describe the main objective here\]/$goal/g" "$file"
        print_info "Injected goal into task_plan.md"
    fi
    
    if [ -n "$session_name" ] && [ -f "$file" ]; then
        # Add session name as header
        sed -i "1s/^/<!-- Session: $session_name -->\n\n/" "$file"
    fi
    
    # Update date in all files
    sed -i "s/\[YYYY-MM-DD\]/$(get_date)/g" "$file"
    sed -i "s/\[YYYY-MM-DD HH:MM\]/$(get_timestamp)/g" "$file"
}

generate_custom_phases() {
    local num_phases="$1"
    local output_file="$2"
    
    # Default phase names based on count
    local phase_names=()
    
    case $num_phases in
        3)
            phase_names=("Research & Planning" "Implementation" "Verification & Handoff")
            ;;
        4)
            phase_names=("Discovery" "Design" "Implementation" "Verification")
            ;;
        5)
            phase_names=("Research & Discovery" "Planning & Design" "Implementation" "Verification" "Completion")
            ;;
        6)
            phase_names=("Research" "Design" "Setup" "Implementation" "Testing" "Deployment")
            ;;
        *)
            # Generate generic phase names
            for i in $(seq 1 $num_phases); do
                phase_names+=("Phase $i")
            done
            ;;
    esac
    
    # Generate phases section
    local phases_content=""
    for i in $(seq 1 $num_phases); do
        local idx=$((i-1))
        local phase_name="${phase_names[$idx]}"
        phases_content+="
### Phase $i: $phase_name
- **Status:** not_started
- **Started:** -
- **Completed:** -
- **Description:** [Describe what happens in this phase]
- **Deliverables:**
  - [ ] Deliverable 1
  - [ ] Deliverable 2

"
    done
    
    echo "$phases_content"
}

copy_template() {
    local template_name="$1"
    local output_dir="$2"
    local force="$3"
    local no_backup="$4"
    
    local src="$TEMPLATES_DIR/$template_name"
    local dest="$output_dir/$template_name"
    
    if [ ! -f "$src" ]; then
        print_error "Template not found: $template_name"
        return 1
    fi
    
    if [ -f "$dest" ]; then
        if [ "$force" = "true" ]; then
            [ "$no_backup" != "true" ] && create_backup "$dest"
            cp "$src" "$dest"
            print_success "Overwrote $template_name"
        else
            print_warning "$template_name already exists, skipping..."
            return 0
        fi
    else
        cp "$src" "$dest"
        print_success "Created $template_name"
    fi
    
    return 0
}

run_interactive_mode() {
    echo ""
    print_step "Interactive Mode"
    echo ""
    
    # Get session name
    if [ -z "$SESSION_NAME" ]; then
        echo -e "${WHITE}Session name (optional, press Enter to skip):${NC}"
        read -r SESSION_NAME
    fi
    
    # Get goal
    if [ -z "$GOAL" ]; then
        echo -e "${WHITE}What is your goal for this session?${NC}"
        read -r GOAL
    fi
    
    # Get number of phases
    if [ "$NUM_PHASES" -eq 5 ]; then
        echo -e "${WHITE}Number of phases (3-7, default: 5):${NC}"
        read -r input_phases
        if [[ "$input_phases" =~ ^[3-7]$ ]]; then
            NUM_PHASES="$input_phases"
        fi
    fi
    
    echo ""
}

# ============================================================================
# PARSE ARGUMENTS
# ============================================================================

INTERACTIVE=false
FORCE=false
GOAL=""
SESSION_NAME=""
NUM_PHASES=5
QUIET=false
NO_BACKUP=false
DO_ARCHIVE=false
OUTPUT_DIR=""

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -v|--version)
            echo "Planning Session Initializer v$VERSION"
            exit 0
            ;;
        -i|--interactive)
            INTERACTIVE=true
            shift
            ;;
        -f|--force)
            FORCE=true
            shift
            ;;
        -g|--goal)
            GOAL="$2"
            shift 2
            ;;
        -n|--name)
            SESSION_NAME="$2"
            shift 2
            ;;
        -p|--phases)
            NUM_PHASES="$2"
            shift 2
            ;;
        -q|--quiet)
            QUIET=true
            shift
            ;;
        -b|--no-backup)
            NO_BACKUP=true
            shift
            ;;
        --archive)
            DO_ARCHIVE=true
            shift
            ;;
        -*)
            print_error "Unknown option: $1"
            echo "Use -h for help"
            exit 1
            ;;
        *)
            OUTPUT_DIR="$1"
            shift
            ;;
    esac
done

# Default output directory
OUTPUT_DIR="${OUTPUT_DIR:-$PROJECT_ROOT}"

# ============================================================================
# MAIN EXECUTION
# ============================================================================

[ "$QUIET" != "true" ] && print_header

# Show configuration
if [ "$QUIET" != "true" ]; then
    echo -e "${CYAN}Configuration:${NC}"
    echo "  Templates:  $TEMPLATES_DIR"
    echo "  Output:     $OUTPUT_DIR"
    echo "  Phases:     $NUM_PHASES"
    [ -n "$SESSION_NAME" ] && echo "  Name:       $SESSION_NAME"
    [ -n "$GOAL" ] && echo "  Goal:       $GOAL"
    echo ""
fi

# Interactive mode
[ "$INTERACTIVE" = "true" ] && run_interactive_mode

# Create output directory
mkdir -p "$OUTPUT_DIR"

# Archive existing session if requested
[ "$DO_ARCHIVE" = "true" ] && archive_session "$OUTPUT_DIR"

# Copy templates
print_step "Creating planning files..."
echo ""

copy_template "task_plan.md" "$OUTPUT_DIR" "$FORCE" "$NO_BACKUP"
copy_template "findings.md" "$OUTPUT_DIR" "$FORCE" "$NO_BACKUP"
copy_template "progress.md" "$OUTPUT_DIR" "$FORCE" "$NO_BACKUP"

# Inject goal and update dates
inject_goal_into_template "$OUTPUT_DIR/task_plan.md" "$GOAL" "$SESSION_NAME"
inject_goal_into_template "$OUTPUT_DIR/findings.md" "" "$SESSION_NAME"
inject_goal_into_template "$OUTPUT_DIR/progress.md" "" "$SESSION_NAME"

# Create session metadata
update_session_metadata "$OUTPUT_DIR" "$GOAL" "$SESSION_NAME"
print_success "Created session metadata"

# Final summary
if [ "$QUIET" != "true" ]; then
    echo ""
    echo -e "${GREEN}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║              📋 Session Initialized Successfully!            ║${NC}"
    echo -e "${GREEN}╚══════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    echo -e "${WHITE}Next Steps:${NC}"
    echo "  1. Edit ${CYAN}task_plan.md${NC} with your goal and phase details"
    echo "  2. Track research in ${CYAN}findings.md${NC} (every 2 actions!)"
    echo "  3. Log progress in ${CYAN}progress.md${NC} as you work"
    echo ""
    echo -e "${WHITE}Available Commands:${NC}"
    echo "  ${BLUE}check-complete.sh${NC}   - Verify completion status"
    echo "  ${BLUE}update-status.sh${NC}    - Quick status updates"
    echo "  ${BLUE}sync-progress.sh${NC}    - Sync files and update stats"
    echo "  ${BLUE}archive-session.sh${NC}  - Archive completed session"
    echo ""
    echo -e "${PURPLE}Remember the core principles:${NC}"
    echo "  📝 2-Action Rule:    Save findings every 2 research actions"
    echo "  🎯 3-Strike Protocol: Structured error recovery"
    echo "  ❓ 5-Question Reboot: Context recovery after breaks"
    echo ""
fi

exit 0
