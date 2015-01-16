/*
 * music player command (mpc)
 * Copyright (C) 2003-2015 The Music Player Daemon Project
 * http://www.musicpd.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "list.h"

#include <stdlib.h>
#include <assert.h>

List *makeList(void) {
	List * list = malloc(sizeof(List));

	assert(list!=NULL);

	list->firstNode = NULL;
	list->lastNode = NULL;
	list->numberOfNodes = 0;

	return list;
}

void insertInListWithoutKey(List * list, void * data) {
	ListNode * node;

	assert(list!=NULL);
	assert(data!=NULL);

	node = malloc(sizeof(ListNode));
	assert(node!=NULL);

	if(list->firstNode==NULL) {
		assert(list->lastNode==NULL);
		list->firstNode = node;
	}
	else {
		assert(list->lastNode!=NULL);
		assert(list->lastNode->nextNode==NULL);
		list->lastNode->nextNode = node;
	}

	node->data = data;
	node->nextNode = NULL;

	list->lastNode = node;

	list->numberOfNodes++;
}

void freeList(void * list) {
	ListNode * tmpNode;
	ListNode * tmpNode2;

	assert(list!=NULL);

	tmpNode = ((List *)list)->firstNode;

	while(tmpNode!=NULL) {
		tmpNode2 = tmpNode->nextNode;
		free(tmpNode);
		tmpNode = tmpNode2;
	}

	free(list);
}
