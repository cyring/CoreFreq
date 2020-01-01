#!/bin/sh
#
# CoreFreq
# Copyright (C) 2015-2020 CYRIL INGENIERIE
# Licenses: GPL2
#
if (( $# > 2 )); then
	COMMAND=$1
	shift
	OPTION=$1
	while [[ -n ${OPTION} ]];
	do
		if [[ "${OPTION}" == "--" ]]; then
			break;
		else
			ARGS="${ARGS} ${OPTION}"
		fi
		shift
		OPTION=$1
	done
#
	shift
	ITEM=$1
	while [[ -n ${ITEM} ]];
	do
		echo "${COMMAND}${ARGS} ${ITEM}"
		${COMMAND}${ARGS} ${ITEM}
		shift
		ITEM=$1
	done
else
	echo "$0: missing arguments"
fi
#
