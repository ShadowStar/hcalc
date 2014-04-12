#!/bin/bash -
#===============================================================================
#
#          FILE: verinfo.sh
#
#         USAGE: ./verinfo.sh <GIT_DIR>
#
#   DESCRIPTION: Get Git Version Info
#
#       OPTIONS: ---
#  REQUIREMENTS: ---
#          BUGS: ---
#         NOTES: ---
#        AUTHOR: Lei Liu (ShadowStar), orphen.leiliu@me.com
#  ORGANIZATION: 
#       CREATED: 2012/08/30 11:56
#      REVISION:  ---
#===============================================================================

#set -o nounset                              # Treat unset variables as an error

if [ -z "$1" ]; then
	git_dir=$PWD
else
	git_dir="$1"
fi

commit_id="HEAD"

[ -d "${git_dir}/.git" ] && git_dir=${git_dir}/.git

export LC_ALL=en_US.UTF-8
export GIT_DIR=${git_dir}

get_ver () {
	local major minor patch
	if [ -z "$1" ]; then
		major="0"
		minor="0"
		patch="0"
	else
		major=$(echo $1 | awk -F'.' '{ print $1 }' | sed -e 's|^[vV]||')
		minor=$(echo $1 | awk -F'.' '{ print $2 }')
		if [ -z "$2" ]; then
			patch="0"
		else
			patch=$(echo $2 | sed -e "s|^$1-||" | cut -d'-' -f 1 2>/dev/null)
		fi
	fi
	echo ${major}.${minor}.${patch}
}

get_commit () {
	local commit
	if [ -z "$1" ]; then
		commit=$(git describe --always ${commit_id} 2>/dev/null)
	else
		commit=$(echo $1 | awk -F'-' '{ print $NF}' | sed -e 's|^g||')
	fi
	echo ${commit}
}

git_info () {
	local branch full_info tag_info
	git branch >/dev/null 2>&1 || return
	branch=$(git branch --contain ${commit_id} | head -n1 | cut -d' ' -f 2- 2>/dev/null)
	full_info=$(git describe --long ${commit_id} 2>/dev/null)
	tag_info=$(git describe --abbrev=0 ${commit_id} 2>/dev/null)

	echo "$(get_ver ${tag_info} ${full_info})-$(get_commit ${full_info})"
}

echo "$(git_info)"

