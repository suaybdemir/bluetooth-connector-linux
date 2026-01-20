#!/bin/bash
# ============================================================================
# archive-session.sh - Session Archiver v2.0.0
# ============================================================================
# Archive completed planning sessions with metadata and compression.
# ============================================================================

set -e

VERSION="2.0.0"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
AGENT_DIR="$(dirname "$SCRIPT_DIR")"
PROJECT_ROOT="$(dirname "$AGENT_DIR")"
ARCHIVE_DIR="$AGENT_DIR/archive"

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
    echo "║             📦 Session Archiver v${VERSION}                      ║"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
}

show_help() {
    echo "Usage: archive-session.sh [OPTIONS] [SOURCE_DIR]"
    echo ""
    echo "Options:"
    echo "  -h, --help       Show this help message"
    echo "  -v, --version    Show version"
    echo "  -n, --name       Custom archive name"
    echo "  -c, --compress   Create compressed tarball"
    echo "  -d, --delete     Delete source files after archiving"
    echo "  -l, --list       List existing archives"
    echo "  -r, --restore    Restore an archive (requires archive name)"
    echo "  --force          Archive even if not all phases complete"
    echo ""
    echo "Examples:"
    echo "  archive-session.sh                     # Archive current session"
    echo "  archive-session.sh -n 'api-feature'    # With custom name"
    echo "  archive-session.sh -c -d               # Compress and delete source"
    echo "  archive-session.sh -l                  # List archives"
    echo "  archive-session.sh -r session_20240115 # Restore archive"
    echo ""
}

get_timestamp() {
    date "+%Y%m%d_%H%M%S"
}

get_date() {
    date "+%Y-%m-%d"
}

list_archives() {
    echo -e "${CYAN}Existing Archives:${NC}"
    echo ""
    
    if [ ! -d "$ARCHIVE_DIR" ]; then
        echo "  No archives found"
        return
    fi
    
    local count=0
    for archive in "$ARCHIVE_DIR"/*; do
        if [ -d "$archive" ] || [ -f "$archive" ]; then
            local name=$(basename "$archive")
            local size=""
            
            if [ -d "$archive" ]; then
                size=$(du -sh "$archive" 2>/dev/null | cut -f1)
                echo -e "  📁 ${WHITE}$name${NC} [$size]"
            elif [[ "$archive" == *.tar.gz ]]; then
                size=$(du -h "$archive" 2>/dev/null | cut -f1)
                echo -e "  📦 ${WHITE}$name${NC} [$size]"
            fi
            ((count++))
        fi
    done
    
    [ "$count" -eq 0 ] && echo "  No archives found"
    echo ""
    echo "  Total: $count archive(s)"
    echo ""
}

restore_archive() {
    local archive_name="$1"
    local target_dir="${2:-$PROJECT_ROOT}"
    
    local archive_path="$ARCHIVE_DIR/$archive_name"
    local tarball_path="$ARCHIVE_DIR/${archive_name}.tar.gz"
    
    if [ -d "$archive_path" ]; then
        echo -e "${CYAN}Restoring from directory: $archive_name${NC}"
        cp -r "$archive_path"/* "$target_dir/"
        echo -e "${GREEN}✅ Restored to $target_dir${NC}"
    elif [ -f "$tarball_path" ]; then
        echo -e "${CYAN}Restoring from tarball: ${archive_name}.tar.gz${NC}"
        tar -xzf "$tarball_path" -C "$target_dir"
        echo -e "${GREEN}✅ Restored to $target_dir${NC}"
    else
        echo -e "${RED}❌ Archive not found: $archive_name${NC}"
        exit 1
    fi
}

check_completion() {
    local task_plan="$1"
    
    if [ ! -f "$task_plan" ]; then
        return 1
    fi
    
    local total=0
    local complete=0
    
    while IFS= read -r line; do
        if [[ "$line" =~ ^###[[:space:]]Phase ]]; then
            ((total++))
        elif [[ "$line" =~ "Status:"[[:space:]]*complete ]]; then
            ((complete++))
        fi
    done < "$task_plan"
    
    [ "$total" -gt 0 ] && [ "$complete" -eq "$total" ]
}

archive_session() {
    local source_dir="$1"
    local archive_name="$2"
    local compress="$3"
    local delete_source="$4"
    
    mkdir -p "$ARCHIVE_DIR"
    
    local archive_path="$ARCHIVE_DIR/$archive_name"
    
    echo -e "${CYAN}Creating archive: $archive_name${NC}"
    
    # Create archive directory
    mkdir -p "$archive_path"
    
    # Copy planning files
    local files_copied=0
    for file in task_plan.md findings.md progress.md .session_meta.json; do
        if [ -f "$source_dir/$file" ]; then
            cp "$source_dir/$file" "$archive_path/"
            echo -e "  ${GREEN}✓${NC} $file"
            ((files_copied++))
        fi
    done
    
    # Create archive metadata
    cat > "$archive_path/.archive_meta.json" << EOF
{
    "archive_name": "$archive_name",
    "archived_at": "$(date -Iseconds)",
    "source_dir": "$source_dir",
    "files_count": $files_copied,
    "compressed": $compress
}
EOF
    
    # Compress if requested
    if [ "$compress" = "true" ]; then
        echo -e "${CYAN}Compressing archive...${NC}"
        tar -czf "${archive_path}.tar.gz" -C "$ARCHIVE_DIR" "$archive_name"
        rm -rf "$archive_path"
        echo -e "  ${GREEN}✓${NC} Created ${archive_name}.tar.gz"
    fi
    
    # Delete source if requested
    if [ "$delete_source" = "true" ]; then
        echo -e "${CYAN}Removing source files...${NC}"
        for file in task_plan.md findings.md progress.md .session_meta.json; do
            [ -f "$source_dir/$file" ] && rm "$source_dir/$file"
        done
        echo -e "  ${GREEN}✓${NC} Source files removed"
    fi
    
    echo ""
    echo -e "${GREEN}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║               📦 Session Archived Successfully!              ║${NC}"
    echo -e "${GREEN}╚══════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    echo "  Location: $ARCHIVE_DIR/$archive_name$([ "$compress" = "true" ] && echo ".tar.gz")"
    echo "  Files: $files_copied"
    echo ""
}

# Parse arguments
SOURCE_DIR=""
ARCHIVE_NAME=""
COMPRESS=false
DELETE_SOURCE=false
LIST_MODE=false
RESTORE_MODE=false
FORCE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -v|--version)
            echo "Session Archiver v$VERSION"
            exit 0
            ;;
        -n|--name)
            ARCHIVE_NAME="$2"
            shift 2
            ;;
        -c|--compress)
            COMPRESS=true
            shift
            ;;
        -d|--delete)
            DELETE_SOURCE=true
            shift
            ;;
        -l|--list)
            LIST_MODE=true
            shift
            ;;
        -r|--restore)
            RESTORE_MODE=true
            shift
            ;;
        --force)
            FORCE=true
            shift
            ;;
        -*)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
        *)
            SOURCE_DIR="$1"
            shift
            ;;
    esac
done

# Execute
print_header

if [ "$LIST_MODE" = true ]; then
    list_archives
    exit 0
fi

if [ "$RESTORE_MODE" = true ]; then
    if [ -z "$SOURCE_DIR" ]; then
        echo -e "${RED}Please specify archive name to restore${NC}"
        echo "Use -l to list available archives"
        exit 1
    fi
    restore_archive "$SOURCE_DIR"
    exit 0
fi

# Default source directory
SOURCE_DIR="${SOURCE_DIR:-$PROJECT_ROOT}"

# Check if session exists
if [ ! -f "$SOURCE_DIR/task_plan.md" ]; then
    echo -e "${RED}❌ No planning session found in: $SOURCE_DIR${NC}"
    exit 1
fi

# Check completion
if [ "$FORCE" != "true" ]; then
    if ! check_completion "$SOURCE_DIR/task_plan.md"; then
        echo -e "${YELLOW}⚠️  Not all phases are complete!${NC}"
        echo ""
        echo "  Use --force to archive anyway"
        echo "  Or run update-status.sh to mark phases complete"
        exit 1
    fi
fi

# Generate archive name
ARCHIVE_NAME="${ARCHIVE_NAME:-session_$(get_timestamp)}"

archive_session "$SOURCE_DIR" "$ARCHIVE_NAME" "$COMPRESS" "$DELETE_SOURCE"
