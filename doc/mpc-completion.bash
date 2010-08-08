# Installation:
# - If you have system bash completion, place this in /etc/bash_completion.d or
#   source it from $HOME/.bash_completion
# - If you don't have system bash completion, source this from your .bashrc

# Escape special characters with backslashes
# Something like this should (but doesn't) also work:
# while read -r line; do printf "%q\n" "$line"; done
__escape_strings_stdin () {
	sed "s/\([><()\";\`' ]\)/\\\\\\1/g"
}

# Read everything past the command as a single word
# This is used for filenames (they may have spaces)
__get_long_cur () {
	cur="$(echo "${COMP_LINE#*$command}" | sed 's/^ *//')"
}

# Complete boolean choices
_mpc_boolean () {
	local IFS=$'\n'
	COMPREPLY=($(IFS=' '; compgen -W "true false yes no on off" -S ' ' "$cur"))
}

# Complete playlist names
_mpc_playlists () {
	local IFS=$'\n'
	__get_long_cur
	if [ -z "$cur" ]; then
		COMPREPLY=($(mpc lsplaylists | __escape_strings_stdin))
	else
		COMPREPLY=($(mpc loadtab $(eval echo "$cur") | __escape_strings_stdin))
	fi
}

# Complete long option names
_mpc_long_options () {
	local IFS=$'\n'
	COMPREPLY=($(mpc help | grep -o -- "$cur"'[a-z-]*=\?' | sed 's/[^=]$/& /'))
}

# Complete command names
_mpc_commands () {
	local IFS=$'\n'
	hold=$(mpc help 2>&1 | awk '/^ *mpc [a-z]+ /{print $2" "}');
	COMPREPLY=($(compgen -W "$hold"$'\n'"status " "$cur"))
}

# Complete the add command (files)
_mpc_add () {
	local IFS=$'\n'
	__get_long_cur
	COMPREPLY=($(mpc tab $(eval echo "$cur") | sed -re "s%^(${cur}[^/]*/?).*%\\1%" | sort -u | __escape_strings_stdin))
}

# Complete the ls command (directories)
_mpc_ls () {
	local success IFS=$'\n'
	__get_long_cur
	if [ -z "$cur" ]; then
		COMPREPLY=($(mpc ls | sed 's@$@/@' | __escape_strings_stdin))
	else
		COMPREPLY=($(mpc ls $(eval echo "$cur") 2> /dev/null | __escape_strings_stdin))
		if [ ${#COMPREPLY[*]} -eq 0 ]; then
			COMPREPLY=($(mpc lstab $(eval echo "$cur") | __escape_strings_stdin))
		fi
	fi
}

# Complete search command (query types)
_mpc_search () {
	local IFS=$'\n'
	COMPREPLY=($(IFS=' '; compgen -W "artist album title track name genre date composer performer comment disc filename any" -S ' ' "$cur"))
}

# Main completion function
_mpc ()
{
	local c=1 word command

	# Skip through long options, caching host/port
	while [ $c -lt $COMP_CWORD ]; do
		word="${COMP_WORDS[c]}"
		case "$word" in
			--host=*) MPD_HOST="${word#--host=}" ;;
			--port=*) MPD_PORT="${word#--host=}" ;;
			-f|--format|--wait|-q|--quiet|--no-status|-v|--verbose) ;;
			*) command="$word"; break ;;
		esac
		c=$((c+1))
	done

	cur="${COMP_WORDS[COMP_CWORD]}"

	# If there's no command, either complete options or commands
	if [ -z "$command" ]; then
		case "$cur" in
			--*) _mpc_long_options ;;
			-*) COMPREPLY=() ;;
			*) _mpc_commands ;;
		esac
		return
	fi

	# Complete command arguments
	case "$command" in
	add)         _mpc_add ;;
	clear)       ;; # no arguments
	consume)     _mpc_boolean ;;
	crop)        ;; # no arguments
	crossfade)   ;; # don't complete numbers
	current)     ;; # no arguments
	del)         ;; # don't complete numbers
	load)        _mpc_playlists ;;
	ls)          _mpc_ls ;;
	lsplaylists) ;; # no arguments
	move)        ;; # don't complete numbers
	next)        ;; # no arguments
	pause)       ;; # no arguments
	play)        ;; # don't complete numbers
	prev)        ;; # no arguments
	random)      _mpc_boolean ;;
	repeat)      _mpc_boolean ;;
	rm)          _mpc_playlists ;;
	save)        _mpc_playlists ;;
	search)      _mpc_search ;;
	seek)        ;; # don't complete numbers
	single)      _mpc_boolean ;;
	stats)       ;; # no arguments
	status)      ;; # no arguments
	stop)        ;; # no arguments
	toggle)      ;; # no arguments
	update)      _mpc_ls ;;
	version)     ;; # no arguments
	volume)      ;; # don't complete numbers
	*)           ;;
	esac

}
complete -o nospace -F _mpc mpc
